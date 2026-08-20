#ifndef SPOT_FACTORY_HPP
#define SPOT_FACTORY_HPP

#include <memory>
#include <string>
#include "ParkingSpot.hpp"
#include "Enums.hpp"

using std::string;
using std::shared_ptr;

/**
 * @brief Factory class to encapsulate ParkingSpot instantiation.
 * 
 * WHY THIS PATTERN EXISTS (FACTORY PATTERN):
 * Decouples spot creation from the parking levels. If new spot types (e.g. Electric charging spots)
 * or special initialization parameters are introduced, only this factory changes.
 */
class SpotFactory {
public:
    static shared_ptr<ParkingSpot> createSpot(const string& spotId, int floor, SpotType type);
};

#endif // SPOT_FACTORY_HPP
