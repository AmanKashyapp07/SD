#ifndef PARKING_LOT_HPP
#define PARKING_LOT_HPP

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "ParkingSpot.hpp"
#include "Ticket.hpp"
#include "Vehicle.hpp"
#include "PricingStrategy.hpp"

using std::string;
using std::vector;
using std::unordered_map;
using std::shared_ptr;
using TimePoint = std::chrono::system_clock::time_point;

/**
 * @brief Core orchestrator managing the Parking Lot system.
 * 
 * WHY THIS CLASS EXISTS:
 * Serves as a Facade providing high-level operations (parkVehicle, unparkVehicle, addSpot)
 * while managing internal state (spots across floors, active tickets) and delegating
 * pricing calculations to the configured PricingStrategy.
 */
class ParkingLot {
private:
    vector<shared_ptr<ParkingSpot>> spots;
    unordered_map<string, shared_ptr<Ticket>> activeTickets; // ticketId -> Ticket
    shared_ptr<PricingStrategy> pricingStrategy;
    int nextTicketNumber;

public:
    explicit ParkingLot(shared_ptr<PricingStrategy> strategy);

    void setPricingStrategy(shared_ptr<PricingStrategy> strategy);
    void addSpot(shared_ptr<ParkingSpot> spot);

    shared_ptr<Ticket> parkVehicle(shared_ptr<Vehicle> vehicle);
    double unparkVehicle(const string& ticketId, TimePoint exitTime);

    void displayStatus() const;
};

#endif // PARKING_LOT_HPP
