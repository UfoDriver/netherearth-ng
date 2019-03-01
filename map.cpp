#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <algorithm>
#include <cmath>

#include "map.h"
#include "nether.h"
#include "resources.h"
#include "utils.h"

extern int detaillevel;
extern int up_key, down_key, left_key, right_key, fire_key, pause_key;


void Map::resize(const int width, const int height)
{
  Width = width;
  Height = height;
  map.clear();
  map.resize(width * height, 0);
  explosions.clear();
  buildings.clear();
  bullets.clear();
  for (Robot* r: robots[0]) delete r;
  for (Robot* r: robots[1]) delete r;
  robots[0].clear();
  robots[1].clear();
}


void Map::draw(const Camera& camera, const Vector& light, const bool shadows)
{
  if (explosions.size()) {
    int minstep = 128;
    for (const Explosion& explosion: explosions) {
      if (explosion.size == 2 && explosion.step < minstep) minstep = explosion.step;
    }
    float r = (128 - minstep) / 256.0;
    float offs = sin(minstep) * r;
    gluLookAt(camera.viewport.x + camera.x * camera.zoom + offs,
              camera.viewport.y + camera.y * camera.zoom + offs,
              camera.viewport.z + camera.z * camera.zoom,
              camera.viewport.x + offs,
              camera.viewport.y + offs,
              camera.viewport.z,
              0, 0, 1);
  } else {
    gluLookAt(camera.viewport.x + camera.x * camera.zoom,
              camera.viewport.y + camera.y * camera.zoom,
              camera.viewport.z + camera.z * camera.zoom,
              camera.viewport.x,
              camera.viewport.y,
              camera.viewport.z,
              0, 0, 1);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (!shadows) {
    glPushMatrix();
    for (int j = 0; j < Height; j++) {
      glPushMatrix();
      for (int i = 0; i < Width; i++) {
        if (camera.canSee(Vector(i, j, 0.0))) {
          int o = map[i + j * Width];
          if (o == 0) {
            int m[8] = {13, 15, 17, 19, 7, 23, 21, 25};
            if (((i * 3 + j * 7) % m[(i + j) % 8]) == 0) o = 10;
            if (((i * 3 + j * 7) % m[(i + j + 1) % 8]) == 0 ) o = 11;
          } else {
          }
          if (detaillevel >= 1)
            Resources::tiles[o].draw(Resources::tiles[0].color);
          else
            Resources::tiles[o].draw_notexture(Resources::tiles[0].color);
        }
        glTranslatef(1.0, 0.0, 0.0);
      }
      glPopMatrix();
      glTranslatef(0.0, 1.0, 0.0);
    }
    glPopMatrix();
  }

  for (const Building& building: buildings) {
    if (camera.canSee(building.pos)) {
      glPushMatrix();
      glTranslatef(float(building.pos.x), float(building.pos.y), float(building.pos.z));
      building.draw(shadows, detaillevel, light);
      glPopMatrix();
    }
  }

  for (int i = 0; i < 2; i++) {
    for (Robot* r: robots[i]) {
      if (camera.canSee(r->pos)) {
        glPushMatrix();
        glTranslatef(r->pos.x, r->pos.y, r->pos.z);
        r->draw(i, shadows, Resources::pieceTiles, light);
        glPopMatrix();
      }
    }
  }

  for (const Bullet& bullet: bullets) {
    if (camera.canSee(bullet.pos)) {
      glPushMatrix();
      glTranslatef(bullet.pos.x, bullet.pos.y, bullet.pos.z);
      bullet.draw(shadows, particles);
      glPopMatrix();
    }
  }

  nether->getShip()->draw(shadows, light, *this, nether->getControlled());

  if (!shadows) {
    for (const Explosion& exp: explosions) {
      float a = (128.0f - exp.step) / 80.0f;
      float r = 1.0;
      if (exp.size == 0) {
        r = (float(exp.step) / 512.0f) + 0.1;
      }
      if (exp.size == 1) {
        r = (float(exp.step) / 96.0f) + 0.5;
      }
      if (exp.size == 2) {
        r = (float(exp.step) / 48.0f) + 1.0;
      }
      if (a < 0) a = 0;
      if (a > 1) a = 1;

      glPushMatrix();
      glTranslatef(exp.pos.x, exp.pos.y, exp.pos.z);
      glColor4f(1.0f, 0.5f, 0.0,a);
      glDepthMask(GL_FALSE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      // Somehow solid sphere dumps core
      // glutSolidSphere(r, 8, 8);
      glutWireSphere(r, 8, 8);
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
      glPopMatrix();
    }
  }

  if (!shadows) {
    for (const Particle& particle: particles) {
      if (camera.canSee(particle.pos))
        particle.draw();
    }
  }
}


float Map::maxZ(float x[2], float y[2]) const
{
  float z = 0;
  int o;

  for (int i = int(x[0]); float(i) < x[1]; i++) {
    for (int j = int(y[0]); float(j) < y[1]; j++) {
      if (i >= 0 && i < Width &&
          j >= 0 && j < Height) {
        o = map[i + j * Width];
        z = std::max(Resources::tiles[o].cmc.z[0], z);
        z = std::max(Resources::tiles[o].cmc.z[1], z);
      }
    }
  }
  return z;
}


int Map::terrain(float x, float y)
{
  switch (map[int(x) + int(y) * Width]) {
  case 0:
    return T_GRASS;
    break;
  case 1:
  case 2:
    return T_SAND;
    break;
  case 3:
    return T_MOUNTAINS;
    break;
  default:
    return T_HOLE;
    break;
  }
}


int Map::worseTerrain(float x[2], float y[2])
{
  int t = terrain(x[0] + 0.001f, y[0] + 0.001f);
  int t2 = terrain(x[1] - 0.001f, y[0] + 0.001f);
  if (t2 == T_HOLE || t == T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) || (t2 == T_SAND && t == T_GRASS))
    t = t2;
  t2 = terrain(x[0] + 0.001f, y[1] - 0.001f);
  if (t2==T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) || (t2 == T_SAND && t == T_GRASS))
    t = t2;
  t2 = terrain(x[1] - 0.001f, y[1] - 0.001f);
  if (t2 == T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) ||
      (t2 == T_SAND && t == T_GRASS)) t = t2;
  return t;
}


bool Map::loadMap(const std::string& filename)
{
  std::ifstream iFile(filename);
  iFile >> Width >> Height;
  resize(Width, Height);

  const std::vector<std::string> tiles = {"G", "S", "S2", "M", "H1",
                                          "H2", "H3", "H4", "H5", "H6",
                                          "GG", "SS", "MM", "?"};
  std::string tilestr;
  for (int i = 0; i < Width * Height; i++) {
    iFile >> tilestr;
    int tile = find_index(tiles, tilestr);
    if (tile >= 10) tile -= 10;
    map[i] = tile;
  }

  do {
    const std::vector<Building>& newBuildings {Building::readMapFile(iFile)};
    std::copy(newBuildings.cbegin(), newBuildings.cend(), std::back_inserter(buildings));
  } while (iFile.good());

  return true;
}


bool Map::cycle(unsigned char *keyboard)
{
  // Keyboard/zoom
  // ship movement
  // robot menu process/redrawing
  // viewport calculation
  // Status redraw
  // Ship on the factory/robot handling
  // Robots cycling
  // Building cycling (capturing)
  // Bullets cycling
  // Nuclear explosion stepping/removal
  // Menu cycling
  // Explosions cycling
  // Particles cycling
  // Game goals checking

  cycleRobots(keyboard);
  cycleBuildings();
  cycleBullets();

  explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                  [](auto& exp) { return !exp.cycle(); }),
                   explosions.end());

  particles.erase(std::remove_if(particles.begin(), particles.end(),
                                 [](auto& particle) { return !particle.cycle(); }),
                  particles.end());

  return true;
}


void Map::cycleBuildings()
{
  for (Building& b: buildings) {
    if (b.type == Building::TYPE::FACTORY_ELECTRONICS ||
        b.type == Building::TYPE::FACTORY_NUCLEAR ||
        b.type == Building::TYPE::FACTORY_PHASERS ||
        b.type == Building::TYPE::FACTORY_MISSILES ||
        b.type == Building::TYPE::FACTORY_CANNONS	||
        b.type == Building::TYPE::FACTORY_CHASSIS) {
      int robot = nether->ai.robotHere(b.pos + Vector(1, 0, 0));
      if (robot == 0) {
        b.status = 0;
      } else {
        if (robot == T_ROBOT) b.status++;
        if (robot == T_EROBOT) b.status--;

        if (b.status >= 12 * 12 * 12) {
          b.owner = 1;
          b.status = 0;
          nether->requestStatsRecomputing();
        }
        if (b.status <= -12 * 12 * 12) {
          b.owner = 2;
          b.status = 0;
          nether->requestStatsRecomputing();
        }
      }
    }

    if (b.type == Building::TYPE::WARBASE) {
      int robot = nether->ai.robotHere(b.pos + Vector(2, 0, 0));
      if (robot == 0) {
        b.status = 0;
      } else {
        if (robot == T_ROBOT) b.status++;
        if (robot == T_EROBOT) b.status--;
        if (b.status >= 12 * 12 * 12) {
          b.owner = 1;
          b.status = 0;
          nether->requestStatsRecomputing();
        }

        if (b.status <= -12 * 12 * 12) {
          b.owner = 2;
          b.status = 0;
          nether->requestStatsRecomputing();
        }
      }
    }
  }
}


void Map::cycleBullets()
{
  bullets.erase(remove_if(bullets.begin(), bullets.end(),
                          [this](auto& bullet) {
                            bool ret = false;

                            int persistence = CANNON_PERSISTENCE;
                            if (bullet.angle == 0) bullet.pos.x += BULLET_SPEED;
                            if (bullet.angle == 90) bullet.pos.y += BULLET_SPEED;
                            if (bullet.angle == 180) bullet.pos.x -= BULLET_SPEED;
                            if (bullet.angle == 270) bullet.pos.y -= BULLET_SPEED;
                            bullet.step++;

                            if (bullet.type == Bullet::TYPE::MISSILES) persistence = MISSILE_PERSISTENCE;
                            if (bullet.type == Bullet::TYPE::PHASERS) persistence = PHASER_PERSISTENCE;
                            Robot* r = 0;
                            if (bullet.step >= persistence || bullet.checkCollision(buildings, robots, &r)) {
                              ret = true;
                              if (bullet.step < persistence) {
                                explosions.emplace_back(bullet.pos, 0);
                              }
                            }
                            if (r != 0) {
                              /* The bullet has collided with a robot: */
                              if (!r->bulletHit(bullet.type)) {
                                /* Robot destroyed: */
                                explosions.emplace_back(r->pos,1);
                                nether->sManager.playExplosion(nether->getShip()->pos, r->pos);
                                nether->detachShip(r);
                                nether->ai.killRobot(r->pos);
                                find_and_destroy_robot(r);
                              }
                            }
                            return ret;
                          }),
                bullets.end());
}


void Map::cycleRobots(unsigned char* keyboard)
{
  float x[2], y[2], minz;
  Vector old_pos;
  int old_chassis_state;
  int terrain;

  for (int i = 0; i < 2; i++) {
    for (Robot* r: robots[i]) {
      /* Robot cycle: */
      /* Animations: */
      if (r->electronics_state != 0) {
        r->electronics_state += 6;
        if (r->electronics_state >= 360)
          r->electronics_state = 0;
      }
      /* Apply ROBOT operator: */
      old_pos = r->pos;
      old_chassis_state = r->chassis_state;
      if (r->traction == 2) {
        r->chassis_state++;
      }
      x[0] = r->pos.x - 0.5;
      x[1] = r->pos.x + 0.5;
      y[0] = r->pos.y - 0.5;
      y[1] = r->pos.y + 0.5;
      minz = maxZ(x, y);
      terrain = worseTerrain(x, y);

      /* Avoid that a Robot can walk agains another and they both get stuck: */
      if (r->op == Robot::OPERATOR::FORWARD &&
          (int(r->pos.x * 256) % 128) == 0 &&
          (int(r->pos.y * 256) % 128) == 0) {
        switch(r->angle) {
        case 0:
          if (nether->ai.worseMapTerrain(int((r->pos.x + 0.5) / 0.5), int((r->pos.y - 0.5) / 0.5), 1, 2) > T_HOLE)
            r->op = Robot::OPERATOR::NONE;
          break;
        case 90:
          if (nether->ai.worseMapTerrain(int((r->pos.x - 0.5) / 0.5), int((r->pos.y + 0.5) / 0.5), 2, 1) > T_HOLE)
            r->op = Robot::OPERATOR::NONE;
          break;
        case 180:
          if (nether->ai.worseMapTerrain(int((r->pos.x - 1.0) / 0.5), int((r->pos.y - 0.5) / 0.5), 1, 2) > T_HOLE)
            r->op = Robot::OPERATOR::NONE;
          break;
        case 270:
          if (nether->ai.worseMapTerrain(int((r->pos.x - 0.5) / 0.5), int((r->pos.y - 1.0) / 0.5), 2, 1) > T_HOLE)
            r->op = Robot::OPERATOR::NONE;
          break;
        }
      }

      if (r->op == Robot::OPERATOR::FORWARD) {
        float speed = r->robotSpeed(terrain);
        /* BIPOD ANIMATION: */
        if (r->traction == 0) {
          r->chassis_state += int(speed / 0.00390625);
          if (r->chassis_state > 64)
            r->chassis_state =- 63;
        }

        /* TRACKS PARTICLES: */
        if (r->traction == 1) {
          if (detaillevel >= 4) {
            Vector pos, sp1;

            for (int i= 0; i < 2; i++) {
              pos.x = r->pos.x + float(rand() % 10) / 100.0;
              pos.y = r->pos.y + float(rand() % 10) / 100.0;
              pos.z = 0;
              Color color(0.9F + float(rand() % 21 - 10) / 100.0,
                          0.7F + float(rand() % 21 - 10) / 100.0,
                          0.5F + float(rand() % 21 - 10) / 100.0);
              switch (r->angle) {
              case 0:
                sp1 = Vector(-0.05, float(rand() % 9 - 4) / 200.0, 0);
                pos.x -= 0.25;
                pos.y += ((rand() % 2) == 0 ? -0.5 : 0.5);
                break;
              case 90:
                sp1 = Vector(float(rand() % 9 - 4) / 200.0, -0.05, 0);
                pos.y -= 0.25;
                pos.x += ((rand() % 2) == 0 ? -0.5 : 0.5);
                break;
              case 180:
                sp1 = Vector(0.05, float(rand() % 9 - 4) / 200.0, 0);
                pos.x += 0.25;
                pos.y += ((rand() % 2) == 0 ? -0.5 : 0.5);
                break;
              case 270:
                sp1 = Vector(float(rand() % 9 - 4) / 200.0, 0.05, 0);
                pos.y += 0.25;
                pos.x += ((rand() % 2) == 0 ? -0.5 : 0.5);
                break;
              }
              particles.emplace_back(pos, sp1, Vector(0, 0, 0.05), 0, 0.3, color, 1.0, 0.0, 20+ (rand() % 10));
            }
          }
        }

        switch (r->angle) {
        case 0:
          if (r->pos.x < Width - 0.5)
            r->pos.x += speed;
          break;
        case 90:
          if (r->pos.y > 0.5)
            r->pos.y += speed;
          break;
        case 180:
          if (r->pos.x > 0.5)
            r->pos.x -= speed;
          break;
        case 270:
          if (r->pos.y < Height - 0.5)
            r->pos.y -= speed;
          break;
        }
      } else {
        if (r->traction == 0)
          r->chassis_state = 0;
      }

      if (r->op == Robot::OPERATOR::LEFT)
        r->angle -= r->robotRotationSpeed(terrain);
      if (r->op == Robot::OPERATOR::RIGHT)
        r->angle += r->robotRotationSpeed(terrain);
      if (r->angle >= 360)
        r->angle -= 360;
      if (r->angle < 0)
        r->angle += 360;

      if (r->op == Robot::OPERATOR::CANNONS && r->firetimer == 0) {
        Vector pos(r->pos);
        pos.z = r->piecez(0) + 0.3f;
        bullets.emplace_back(Bullet::TYPE::CANNONS, pos, r);
        nether->sManager.playShot(nether->getShip()->pos, r->pos);
      }

      if (r->op == Robot::OPERATOR::MISSILES && r->firetimer == 0) {
        Vector pos (r->pos);
        pos.z = r->piecez(1) + 0.2f;
        bullets.emplace_back(Bullet::TYPE::MISSILES, pos, r);
        nether->sManager.playShot(nether->getShip()->pos, r->pos);
      }

      if (r->op == Robot::OPERATOR::PHASERS && r->firetimer == 0) {
        Vector pos(r->pos);
        pos.z = r->piecez(2) + 0.3f;
        bullets.emplace_back(Bullet::TYPE::PHASERS, pos, r);
        nether->sManager.playShot(nether->getShip()->pos, r->pos);
      }

      if (r->op == Robot::OPERATOR::CANNONS ||
          r->op == Robot::OPERATOR::MISSILES ||
          r->op == Robot::OPERATOR::PHASERS)
        r->firetimer++;

      if (r->op == Robot::OPERATOR::NUCLEAR) {
        Explosion exp(r->pos, 2);

        explosions.push_back(exp);

        /* Robot destroyed: */
        nether->detachShip(r);

        /* Find Robots to destroy: */
        for (int i = 0; i < 2; i++) {
          robots[i].erase(std::remove_if(robots[i].begin(), robots[i].end(),
                                         [exp, this] (auto& r) {
                                           float distance=(r->pos - exp.pos).norma();
                                           if (distance <= NUCLEAR_RADIUS) {
                                             nether->ai.killRobot(r->pos);
                                             return true;
                                           } else {
                                             return false;
                                           }
                                         }),
                          robots[i].end());
        }

        /* Find buildings to destroy: */
        buildings.erase(std::remove_if(buildings.begin(), buildings.end(),
                                       [exp, this](auto& b) {
                                         float distance = (b.pos - (exp.pos - Vector(0.5, 0.5, 0.5))).norma();
                                         if (distance <= NUCLEAR_RADIUS) {
                                           nether->ai.removeBuilding(b.pos);
                                           return true;
                                         } else {
                                           return false;
                                         }
                                       }),
                        buildings.end());
        nether->sManager.playExplosion(nether->getShip()->pos, r->pos);
        nether->stats.requestRecomputing();
      }

      {
        x[0] = r->pos.x - 0.5;
        x[1] = r->pos.x + 0.5;
        y[0] = r->pos.y - 0.5;
        y[1] = r->pos.y + 0.5;
        minz = maxZ(x, y);
        terrain = worseTerrain(x, y);
        r->pos.z = minz;

        if (r->shipover) {
          nether->getShip()->pos.x = r->pos.x - 0.5;
          nether->getShip()->pos.y = r->pos.y - 0.5;
          nether->getShip()->pos.z = r->pos.z + r->cmc.z[1];
        }

        /* Collision: */
        if (r->checkCollision(buildings, robots, false, nether->getShip()) || !r->walkable(terrain)) {
          r->pos = old_pos;
            if (r->traction == 0)
              r->chassis_state = old_chassis_state;
            if (r->shipover) {
              nether->getShip()->pos.x = r->pos.x - 0.5;
              nether->getShip()->pos.y = r->pos.y - 0.5;
              nether->getShip()->pos.z = r->pos.z + r->cmc.z[1];
            }
        } else {
          nether->ai.moveRobot(old_pos,r->pos,i);
        }

        if (r->op == Robot::OPERATOR::FORWARD && (r->angle == 0 || r->angle == 180) &&
            (int(r->pos.x * 256) % 128) == 0)
          r->op = Robot::OPERATOR::NONE;
        if (r->op == Robot::OPERATOR::FORWARD && (r->angle == 90 || r->angle == 270) &&
            (int(r->pos.y * 256) % 128) == 0)
          r->op = Robot::OPERATOR::NONE;
        if (r->op == Robot::OPERATOR::LEFT && (r->angle%90) == 0)
          r->op = Robot::OPERATOR::NONE;
        if (r->op == Robot::OPERATOR::RIGHT && (r->angle%90) == 0)
          r->op = Robot::OPERATOR::NONE;
        if (r->op == Robot::OPERATOR::CANNONS && r->firetimer >= 64) {
          r->op = Robot::OPERATOR::NONE;
          r->firetimer = 0;
        }
        if (r->op == Robot::OPERATOR::MISSILES && r->firetimer >= 64) {
          r->op = Robot::OPERATOR::NONE;
          r->firetimer = 0;
        }
        if (r->op == Robot::OPERATOR::PHASERS && r->firetimer >= 64) {
          r->op = Robot::OPERATOR::NONE;
          r->firetimer = 0;
        }

        /* Follow ROBOT program: */
        if (r->op == Robot::OPERATOR::NONE && !r->shipover) {
          switch (r->program) {
          case Robot::PROGRAM_NONE:
            break;
          case Robot::PROGRAM_FORWARD:
            r->op = Robot::OPERATOR::FORWARD;
            break;
          case Robot::PROGRAM_STOPDEFEND:
            r->op = nether->ai.programStopDefend(*r, &(r->program_goal), i + 1);
            break;
          case Robot::PROGRAM_ADVANCE:
            r->op = nether->ai.programAdvance(*r, i + 1);
            if (r->op == Robot::OPERATOR::FORWARD && r->angle == 90)
              r->program_parameter.as_int--;
            if (r->op == Robot::OPERATOR::FORWARD && r->angle == 270)
              r->program_parameter.as_int++;
            if (r->program_parameter.as_int == 0)
              r->program = Robot::PROGRAM_STOPDEFEND;
            break;
          case Robot::PROGRAM_RETREAT:
            r->op = nether->ai.programRetreat(*r, i + 1);
            if (r->op == Robot::OPERATOR::FORWARD && r->angle == 270)
              r->program_parameter.as_int--;
            if (r->op == Robot::OPERATOR::FORWARD && r->angle == 90)
              r->program_parameter.as_int++;
            if (r->program_parameter.as_int == 0)
              r->program = Robot::PROGRAM_STOPDEFEND;
            break;
          case Robot::PROGRAM_DESTROY:
            r->op = nether->ai.programDestroy(*r, &(r->program_goal), i + 1);
            // if (r->program_goal.x == -1) r->program = PROGRAM_STOPDEFEND;
            break;
          case Robot::PROGRAM_CAPTURE:
            r->op = nether->ai.programCapture(*r, &(r->program_goal), i + 1);
            // if (r.program_goal.x==-1) r.program=PROGRAM_STOPDEFEND;
            break;
          }
        }

        /* Follow USER's command: */
        if (r->op == Robot::OPERATOR::NONE && r->shipover &&
            (nether->getActiveMenu() == Menu::TYPE::DIRECTCONTROL ||
             nether->getActiveMenu() == Menu::TYPE::DIRECTCONTROL2)) {
          if (keyboard[right_key]) {
            if (r->angle == 0) {
              r->op = Robot::OPERATOR::FORWARD;
            } else {
              if (r->angle == 270)
                r->op = Robot::OPERATOR::RIGHT;
              else
                r->op = Robot::OPERATOR::LEFT;
            }
          }
          if (keyboard[left_key]) {
            if (r->angle == 180) {
              r->op = Robot::OPERATOR::FORWARD;
            } else {
              if (r->angle == 90)
                r->op = Robot::OPERATOR::RIGHT;
              else
                r->op = Robot::OPERATOR::LEFT;
            }
          }
          if (keyboard[up_key]) {
            if (r->angle == 90) {
              r->op = Robot::OPERATOR::FORWARD;
            } else {
              if (r->angle == 0)
                r->op = Robot::OPERATOR::RIGHT;
              else
                r->op = Robot::OPERATOR::LEFT;
            }
          }
          if (keyboard[down_key]) {
            if (r->angle == 270) {
              r->op = Robot::OPERATOR::FORWARD;
            } else {
              if (r->angle == 180)
                r->op = Robot::OPERATOR::RIGHT;
              else
                r->op = Robot::OPERATOR::LEFT;
            }
          }
        }
      }
    }
  }
}


void Map::find_and_destroy_robot(Robot* robot)
{
  for (int i = 0; i < 2; i++) {
    robots[i].erase(std::remove_if(robots[i].begin(), robots[i].end(),
                                   [robot](const Robot* r) {
                                     return r == robot;
                                   }),
                    robots[i].end());
  }
  delete robot;
}


std::ostream& operator<<(std::ostream& out, const Map& map)
{
  out << map.width() << ' ' << map.height() << '\n';

  for (int i = 0; i < map.height(); i++) {
    for (int j = 0; j < map.width(); j++) {
      out << map.map[j + i * map.width()] << ' ';
    }
    out << '\n';
  }
  return out;
}


std::istream& operator>>(std::istream& in, Map& map)
{
  int width, height;
  in >> width >> height;
  map.resize(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      int tile;
      in >> tile;
      map.map.push_back(tile);
    }
  }
  return in;
}
