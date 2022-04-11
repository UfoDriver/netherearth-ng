cmake_minimum_required(VERSION 3.10)

project(netherearth VERSION 0.51)
add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(submodules/sexp-cpp)
include_directories(submodules/sexp-cpp/include)

find_library (GLLIB NAMES OpenGL)
find_library (GLULIB NAMES GLU)
find_library (GLUTLIB NAMES glut)
find_library (SDLLIB NAMES SDL)
find_library (SDLMIXERLIB NAMES SDL_mixer)
link_libraries(stdc++fs)


target_link_libraries (
  ${PROJECT_NAME}
  ${GLLIB}
  ${GLULIB}
  ${GLUTLIB}
  ${SDLLIB}
  ${SDLMIXERLIB}
  sexp)

target_sources (${PROJECT_NAME}
  PUBLIC
      3dobject.cpp
      ai.cpp
      bitmap.cpp
      building.cpp
      buildingblock.cpp
      buildingfactory.cpp
      buildingsimple.cpp
      buildingwarbase.cpp
      bullet.cpp
      bulletcannon.cpp
      bulletmissile.cpp
      bulletphaser.cpp
      camera.cpp
      cmc.cpp
      constructionscreen.cpp
      explosion.cpp
      glprintf.cpp
      light.cpp
      main.cpp
      mainmenu.cpp
      map.cpp
      menu.cpp
      myglutaux.cpp
      nether.cpp
      optionsscreen.cpp
      particle.cpp
      piece3dobject.cpp
      quaternion.cpp
      radar.cpp
      resources.cpp
      robot.cpp
      robots.cpp
      shadow3dobject.cpp
      ship.cpp
      soundmanager.cpp
      stats.cpp
      statusbutton.cpp
      vector.cpp
)
