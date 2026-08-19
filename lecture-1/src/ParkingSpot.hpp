#ifndef PARKING_SPOT_HPP
#define PARKING_SPOT_HPP

#include <string>
#include <memory>
#include "Enums.hpp"
#include "Vehicle.hpp"

using std::string;
using std::shared_ptr;

/**
 * @brief Represents an individual parking slot within the lot.
 * 
 * WHY THIS CLASS EXISTS:
 * Manages the state and availability of a specific parking slot.
 * Encapsulates parking and unparking operations to maintain consistency.
 */
class ParkingSpot {
private:
    string spotId;
    int floorNumber;
    SpotType spotType;
    bool occupied;
    shared_ptr<Vehicle> parkedVehicle;

public:
    ParkingSpot(const string& id, int floor, SpotType type);

    string getSpotId() const;
    int getFloorNumber() const;
    SpotType getSpotType() const;
    bool isOccupied() const;
    shared_ptr<Vehicle> getParkedVehicle() const;

    bool isCompatibleWith(VehicleType vehicleType) const;
    bool park(shared_ptr<Vehicle> vehicle);
    void unpark();
};

#endif // PARKING_SPOT_HPP
