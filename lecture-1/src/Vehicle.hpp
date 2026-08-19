#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <string>
#include "Enums.hpp"

using std::string;

/**
 * @brief Represents a Vehicle entering the parking lot.
 * 
 * WHY THIS CLASS EXISTS:
 * Encapsulates domain properties of a vehicle (license plate & vehicle type).
 * By keeping vehicle data immutable after creation, we prevent accidental state mutations.
 */
class Vehicle {
private:
    string licensePlate;
    VehicleType type;

public:
    Vehicle(const string& plate, VehicleType vehicleType);
    
    string getLicensePlate() const;
    VehicleType getType() const;
};

#endif // VEHICLE_HPP
