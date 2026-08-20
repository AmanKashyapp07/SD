#include "ParkingLevel.hpp"

using namespace std;

ParkingLevel::ParkingLevel(int floor)
    : floorNumber(floor) {}

int ParkingLevel::getFloorNumber() const {
    return floorNumber;
}

const vector<shared_ptr<ParkingSpot>>& ParkingLevel::getSpots() const {
    return spots;
}

void ParkingLevel::addSpot(shared_ptr<ParkingSpot> spot) {
    spots.push_back(spot);
}

shared_ptr<ParkingSpot> ParkingLevel::findAvailableSpot(VehicleType vehicleType) {
    for (auto& spot : spots) {
        if (!spot->getIsOccupied() && spot->canFitVehicle(vehicleType)) {
            return spot;
        }
    }
    return nullptr;
}
