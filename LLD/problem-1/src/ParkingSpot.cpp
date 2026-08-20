#include "ParkingSpot.hpp"

using namespace std;

ParkingSpot::ParkingSpot(const string& id, int floor, SpotType type)
    : spotId(id), floorNumber(floor), spotType(type), isOccupied(false), parkedVehicle(nullptr) {}

string ParkingSpot::getSpotId() const {
    return spotId;
}

int ParkingSpot::getFloorNumber() const {
    return floorNumber;
}

SpotType ParkingSpot::getSpotType() const {
    return spotType;
}

bool ParkingSpot::getIsOccupied() const {
    return isOccupied;
}

shared_ptr<Vehicle> ParkingSpot::getParkedVehicle() const {
    return parkedVehicle;
}

bool ParkingSpot::canFitVehicle(VehicleType vehicleType) const {
    switch (vehicleType) {
        case VehicleType::MOTORCYCLE: return spotType == SpotType::SMALL;
        case VehicleType::CAR:        return spotType == SpotType::MEDIUM;
        case VehicleType::TRUCK:      return spotType == SpotType::LARGE;
        default:                      return false;
    }
}

bool ParkingSpot::park(shared_ptr<Vehicle> vehicle) {
    if (isOccupied || !canFitVehicle(vehicle->getType())) {
        return false;
    }
    parkedVehicle = vehicle;
    isOccupied = true;
    return true;
}

void ParkingSpot::unpark() {
    parkedVehicle = nullptr;
    isOccupied = false;
}
