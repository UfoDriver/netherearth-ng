#include "building.h"
#include "utils.h"


Building::Building(std::istream& in)
{
  in >> type >> owner >> status >> pos;
}
