#ifndef SPOT_FACTORY_HPP
#define SPOT_FACTORY_HPP

#include <memory>
#include <string>
#include "ParkingSpot.hpp"
#include "Enums.hpp"

using std::string;
using std::shared_ptr;

/**
 * @brief Factory class to create ParkingSpot instances.
 * 
 * WHY THIS PATTERN EXISTS (FACTORY PATTERN):
 * Encapsulates object instantiation logic. If spot initialization rules change
 * (e.g. adding charging capabilities or extra attributes for specific spots),
 * only this factory class needs to be modified, keeping caller classes decoupled.
 */
class SpotFactory {
public:
    static shared_ptr<ParkingSpot> createSpot(const string& spotId, int floor, SpotType type);
};

#endif // SPOT_FACTORY_HPP
