#include "PricingStrategy.hpp"
#include <cmath>

using namespace std;
using namespace std::chrono;

FlatHourlyPricingStrategy::FlatHourlyPricingStrategy(double rate)
    : hourlyRate(rate) {}

double FlatHourlyPricingStrategy::calculateFee(const Ticket& ticket, TimePoint exitTime) const {
    auto duration = duration_cast<hours>(exitTime - ticket.getEntryTime());
    long hrs = duration.count();
    if (hrs <= 0) hrs = 1; // Minimum 1-hour charge
    return hrs * hourlyRate;
}

VehicleBasedPricingStrategy::VehicleBasedPricingStrategy(double bike, double car, double truck)
    : bikeRate(bike), carRate(car), truckRate(truck) {}

double VehicleBasedPricingStrategy::calculateFee(const Ticket& ticket, TimePoint exitTime) const {
    auto duration = duration_cast<hours>(exitTime - ticket.getEntryTime());
    long hrs = duration.count();
    if (hrs <= 0) hrs = 1; // Minimum 1-hour charge

    double rate = carRate;
    switch (ticket.getVehicleType()) {
        case VehicleType::MOTORCYCLE: rate = bikeRate; break;
        case VehicleType::CAR:        rate = carRate; break;
        case VehicleType::TRUCK:      rate = truckRate; break;
    }

    return hrs * rate;
}
