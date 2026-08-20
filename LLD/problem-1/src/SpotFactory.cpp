#include "SpotFactory.hpp"

using namespace std;

shared_ptr<ParkingSpot> SpotFactory::createSpot(const string& spotId, int floor, SpotType type) {
    return make_shared<ParkingSpot>(spotId, floor, type);
}
