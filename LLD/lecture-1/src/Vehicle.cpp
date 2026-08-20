#include "Vehicle.hpp"

using std::string;

Vehicle::Vehicle(const string& plate, VehicleType vehicleType)
    : licensePlate(plate), type(vehicleType) {}

string Vehicle::getLicensePlate() const {
    return licensePlate;
}

VehicleType Vehicle::getType() const {
    return type;
}
