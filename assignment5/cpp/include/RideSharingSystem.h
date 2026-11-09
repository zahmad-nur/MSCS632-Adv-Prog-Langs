// RideSharingSystem.h - ties riders, drivers and rides together.
//
// Matching rule: check the rider's favorite drivers first (in the rider's
// order of preference); if none of them is available, assign the
// highest-rated available driver.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Driver.h"
#include "Ride.h"
#include "Rider.h"

struct MatchResult {
    std::shared_ptr<Driver> driver;  // nullptr if nobody is available
    bool viaFavorite = false;
};

class RideSharingSystem {
public:
    std::shared_ptr<Driver> addDriver(const std::string& id, const std::string& name,
                                      double rating = 5.0, int ratingCount = 0);
    std::shared_ptr<Rider> addRider(const std::string& id, const std::string& name);

    // Rider requests the ride (fare locked in), then a driver is matched.
    MatchResult bookRide(Rider& rider, const std::shared_ptr<Ride>& ride);

    // Driver finishes the trip and the rider leaves a star rating.
    void completeRide(const std::string& rideID, int stars);

    const std::vector<std::shared_ptr<Ride>>& allRides() const { return rides_; }
    const std::vector<std::shared_ptr<Driver>>& drivers() const { return drivers_; }
    std::shared_ptr<Driver> findDriver(const std::string& driverID) const;

private:
    MatchResult findDriverFor(const Rider& rider) const;

    std::vector<std::shared_ptr<Driver>> drivers_;
    std::vector<std::shared_ptr<Rider>> riders_;
    std::vector<std::shared_ptr<Ride>> rides_;
};
