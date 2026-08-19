#include "ParkingSpot.hpp"

using std::string;
using std::shared_ptr;

ParkingSpot::ParkingSpot(const string& id, int floor, SpotType type)
    : spotId(id), floorNumber(floor), spotType(type), occupied(false), parkedVehicle(nullptr) {}

string ParkingSpot::getSpotId() const {
    return spotId;
}

int ParkingSpot::getFloorNumber() const {
    return floorNumber;
}

SpotType ParkingSpot::getSpotType() const {
    return spotType;
}

bool ParkingSpot::isOccupied() const {
    return occupied;
}

shared_ptr<Vehicle> ParkingSpot::getParkedVehicle() const {
    return parkedVehicle;
}

bool ParkingSpot::isCompatibleWith(VehicleType vehicleType) const {
    switch (vehicleType) {
        case VehicleType::MOTORCYCLE: return spotType == SpotType::SMALL;
        case VehicleType::CAR:        return spotType == SpotType::MEDIUM;
        case VehicleType::TRUCK:      return spotType == SpotType::LARGE;
        default:                      return false;
    }
}

bool ParkingSpot::park(shared_ptr<Vehicle> vehicle) {
    if (occupied || !isCompatibleWith(vehicle->getType())) {
        return false;
    }
    parkedVehicle = vehicle;
    occupied = true;
    return true;
}

void ParkingSpot::unpark() {
    parkedVehicle = nullptr;
    occupied = false;
}
