// Driver.h - a driver and the rides they have been assigned.
//
// Encapsulation: assignedRides_ is private. Outside code can add a ride
// through addRide() and read rides through rides(), which returns a copy of
// read-only (const) pointers, so callers can neither change the driver's list
// nor modify the rides in it. Rating is also private; it only changes
// through addRating(), which validates the 1-5 star range.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Ride.h"

class Driver {
public:
    Driver(std::string driverID, std::string name,
           double startingRating = 5.0, int startingRatingCount = 0);

    void addRide(const std::shared_ptr<Ride>& ride);
    void completeRide(const std::string& rideID);
    void addRating(int stars);

    std::vector<std::shared_ptr<const Ride>> rides() const;
    std::size_t rideCount() const { return assignedRides_.size(); }
    bool isAvailable() const;      // no ride currently in progress
    double totalEarnings() const;  // sum of completed fares
    double rating() const;
    int ratingCount() const { return ratingCount_; }

    std::string getDriverInfo() const;

    const std::string& driverID() const { return driverID_; }
    const std::string& name() const { return name_; }

private:
    std::string driverID_;
    std::string name_;
    double ratingTotal_;
    int ratingCount_;
    std::vector<std::shared_ptr<Ride>> assignedRides_;
};
