#ifndef PARKING_LEVEL_HPP
#define PARKING_LEVEL_HPP

#include <vector>
#include <memory>
#include "ParkingSpot.hpp"
#include "Enums.hpp"

using std::vector;
using std::shared_ptr;

/**
 * @brief Represents a single level/floor within the parking lot.
 * 
 * WHY THIS CLASS EXISTS:
 * Encapsulates the collection of spots on a specific floor and isolates
 * floor-level spot searching logic from the top-level ParkingLot orchestrator.
 */
class ParkingLevel {
private:
    int floorNumber;
    vector<shared_ptr<ParkingSpot>> spots;

public:
    explicit ParkingLevel(int floor);

    int getFloorNumber() const;
    const vector<shared_ptr<ParkingSpot>>& getSpots() const;

    void addSpot(shared_ptr<ParkingSpot> spot);
    shared_ptr<ParkingSpot> findAvailableSpot(VehicleType vehicleType);
};

#endif // PARKING_LEVEL_HPP
