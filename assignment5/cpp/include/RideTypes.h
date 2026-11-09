// RideTypes.h - concrete ride types. Each one overrides fare().
//
//   Ride
//    |-- StandardRide            $2.50 + $1.75/mi, minimum $7.00
//    |    `-- PoolRide           Standard price minus a shared-ride discount
//    `-- PremiumRide             $5.00 + $3.25/mi + $2.50 booking fee, minimum $15.00
#pragma once

#include "Ride.h"

class StandardRide : public Ride {
public:
    using Ride::Ride;  // inherit the base constructor

    double fare() const override;
    std::string rideType() const override;
};

class PremiumRide : public Ride {
public:
    PremiumRide(std::string rideID, std::string pickupLocation,
                std::string dropoffLocation, double distance,
                std::string vehicleClass = "Lux Black");

    double fare() const override;
    std::string rideType() const override;
    std::string rideDetails() const override;  // adds the vehicle class

    const std::string& vehicleClass() const { return vehicleClass_; }

private:
    std::string vehicleClass_;
};

// PoolRide inherits from StandardRide (two levels deep) and reuses its
// pricing through StandardRide::fare() before applying the discount.
class PoolRide : public StandardRide {
public:
    PoolRide(std::string rideID, std::string pickupLocation,
             std::string dropoffLocation, double distance, int coRiders);

    double fare() const override;
    std::string rideType() const override;
    std::string rideDetails() const override;  // adds co-riders and discount

    int coRiders() const { return coRiders_; }
    double discountRate() const;  // 20% + 5% per co-rider, capped at 40%

private:
    int coRiders_;
};
