#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <string>

using std::string;

// Enum defining supported vehicle types in the system
enum class VehicleType {
    MOTORCYCLE,
    CAR,
    TRUCK
};

// Enum defining parking spot types corresponding to vehicle sizes
enum class SpotType {
    SMALL,   // For Motorcycles
    MEDIUM,  // For Cars
    LARGE    // For Trucks
};

// Helper utilities to convert Enums to readable string representation for console output
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
