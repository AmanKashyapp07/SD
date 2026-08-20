#ifndef PRICING_STRATEGY_HPP
#define PRICING_STRATEGY_HPP

#include <chrono>
#include "Ticket.hpp"

using TimePoint = std::chrono::system_clock::time_point;

/**
 * @brief Strategy interface for calculating parking fees.
 * 
 * WHY THIS PATTERN EXISTS (STRATEGY PATTERN):
 * Decouples fee calculation algorithms from the parking lot orchestrator.
 * Allows switching between flat hourly rates, vehicle-dependent rates, or peak surge rates
 * without modifying ParkingLot code (Open/Closed Principle).
 */
class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual double calculateFee(const Ticket& ticket, TimePoint exitTime) const = 0;
};

class FlatHourlyPricingStrategy : public PricingStrategy {
private:
    double hourlyRate;

public:
    explicit FlatHourlyPricingStrategy(double rate = 20.0);
    double calculateFee(const Ticket& ticket, TimePoint exitTime) const override;
};

class VehicleBasedPricingStrategy : public PricingStrategy {
private:
    double bikeRate;
    double carRate;
    double truckRate;

public:
    VehicleBasedPricingStrategy(double bike = 10.0, double car = 20.0, double truck = 50.0);
    double calculateFee(const Ticket& ticket, TimePoint exitTime) const override;
};

#endif // PRICING_STRATEGY_HPP
