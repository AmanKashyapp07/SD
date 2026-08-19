#ifndef TICKET_HPP
#define TICKET_HPP

#include <string>
#include <chrono>
#include "Enums.hpp"

using std::string;
using TimePoint = std::chrono::system_clock::time_point;

/**
 * @brief Represents a Parking Ticket issued upon entry.
 * 
 * WHY THIS CLASS EXISTS:
 * Serves as an immutable record of a vehicle's parking session.
 * Tracks entry timestamp and assigned spot so the system can verify tickets and calculate fees accurately.
 */
class Ticket {
private:
    string ticketId;
    string licensePlate;
    VehicleType vehicleType;
    string spotId;
    TimePoint entryTime;

public:
    Ticket(const string& id, const string& plate, VehicleType vType, const string& sId, TimePoint entry);

    string getTicketId() const;
    string getLicensePlate() const;
    VehicleType getVehicleType() const;
    string getSpotId() const;
    TimePoint getEntryTime() const;
};

#endif // TICKET_HPP
