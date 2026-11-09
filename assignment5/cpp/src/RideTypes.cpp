#include "RideTypes.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

// ---------------------------------------------------------------- Standard
double StandardRide::fare() const {
    return atLeast(2.50 + 1.75 * distance(), 7.00);
}

std::string StandardRide::rideType() const { return "Standard"; }

// ----------------------------------------------------------------- Premium
PremiumRide::PremiumRide(std::string rideID, std::string pickupLocation,
                         std::string dropoffLocation, double distance,
                         std::string vehicleClass)
    : Ride(std::move(rideID), std::move(pickupLocation),
           std::move(dropoffLocation), distance),
      vehicleClass_(std::move(vehicleClass)) {}

double PremiumRide::fare() const {
    const double bookingFee = 2.50;
    return atLeast(5.00 + 3.25 * distance() + bookingFee, 15.00);
}

std::string PremiumRide::rideType() const { return "Premium"; }

std::string PremiumRide::rideDetails() const {
    return Ride::rideDetails() + " | vehicle: " + vehicleClass_;
}

// -------------------------------------------------------------------- Pool
PoolRide::PoolRide(std::string rideID, std::string pickupLocation,
                   std::string dropoffLocation, double distance, int coRiders)
    : StandardRide(std::move(rideID), std::move(pickupLocation),
                   std::move(dropoffLocation), distance),
      coRiders_(coRiders) {
    if (coRiders_ < 0) throw std::invalid_argument("coRiders cannot be negative");
}

double PoolRide::discountRate() const {
    return std::min(0.20 + 0.05 * coRiders_, 0.40);
}

double PoolRide::fare() const {
    // Reuse the parent's pricing, then discount it.
    const double standardPrice = StandardRide::fare();
    return atLeast(standardPrice * (1.0 - discountRate()), 5.00);
}

std::string PoolRide::rideType() const { return "Pool"; }

std::string PoolRide::rideDetails() const {
    std::ostringstream out;
    out << StandardRide::rideDetails() << " | " << coRiders_ << " co-rider"
        << (coRiders_ == 1 ? "" : "s") << ", "
        << std::lround(discountRate() * 100) << "% off";
    return out.str();
}
