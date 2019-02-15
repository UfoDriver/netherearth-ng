#include "building.h"
#include "utils.h"


Building::Building(std::istream& in)
{
  in >> type >> owner >> status >> pos;
}


std::ostream& operator<<(std::ostream& out, const Building& building)
{
  return out << building.type << ' ' << building.owner << ' ' << building.status << '\n'
             << building.pos;
}
