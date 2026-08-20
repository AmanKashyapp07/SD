#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <string>

using std::string;

// Enum defining supported vehicle types
enum class VehicleType {
    MOTORCYCLE,
    CAR,
    TRUCK
};

// Enum defining spot sizes corresponding to vehicle types
enum class SpotType {
    SMALL,   // For Motorcycles
    MEDIUM,  // For Cars
    LARGE    // For Trucks
};

inline string vehicleTypeToString(VehicleType type) {
    switch (type) {
        case VehicleType::MOTORCYCLE: return "Motorcycle";
        case VehicleType::CAR:        return "Car";
        case VehicleType::TRUCK:      return "Truck";
        default:                      return "Unknown";
    }
}

inline string spotTypeToString(SpotType type) {
    switch (type) {
        case SpotType::SMALL:  return "Small (Motorcycle)";
        case SpotType::MEDIUM: return "Medium (Car)";
        case SpotType::LARGE:  return "Large (Truck)";
        default:               return "Unknown";
    }
}

#endif // ENUMS_HPP
