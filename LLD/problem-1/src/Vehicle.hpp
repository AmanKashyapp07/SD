#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <string>
#include "Enums.hpp"

using std::string;

/**
 * @brief Represents a physical vehicle entering the parking lot.
 * 
 * WHY THIS CLASS EXISTS:
 * Immutable domain entity encapsulating the license plate and vehicle type.
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
