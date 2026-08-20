#ifndef PARKING_SPOT_HPP
#define PARKING_SPOT_HPP

#include <string>
#include <memory>
#include "Enums.hpp"
#include "Vehicle.hpp"

using std::string;
using std::shared_ptr;

/**
 * @brief Represents an individual parking slot on a level.
 * 
 * WHY THIS CLASS EXISTS:
 * Encapsulates the occupancy state, spot size, and parking/unparking behavior.
 */
class ParkingSpot {
private:
    string spotId;
    int floorNumber;
    SpotType spotType;
    bool isOccupied;
    shared_ptr<Vehicle> parkedVehicle;

public:
    ParkingSpot(const string& id, int floor, SpotType type);

    string getSpotId() const;
    int getFloorNumber() const;
    SpotType getSpotType() const;
    bool getIsOccupied() const;
    shared_ptr<Vehicle> getParkedVehicle() const;

    bool canFitVehicle(VehicleType vehicleType) const;
    bool park(shared_ptr<Vehicle> vehicle);
    void unpark();
};

#endif // PARKING_SPOT_HPP
