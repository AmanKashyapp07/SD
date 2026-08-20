#include "SpotFactory.hpp"

using std::string;
using std::shared_ptr;
using std::make_shared;

shared_ptr<ParkingSpot> SpotFactory::createSpot(const string& spotId, int floor, SpotType type) {
    return make_shared<ParkingSpot>(spotId, floor, type);
}
