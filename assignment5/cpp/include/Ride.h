// Ride.h - base class for every ride type in the system.
//
// OOP notes
//   Encapsulation : all state is private; subclasses and callers go through
//                   accessors or the protected helper.
//   Inheritance   : StandardRide, PremiumRide and PoolRide derive from Ride.
//   Polymorphism  : fare(), rideType() and rideDetails() are virtual, so a
//                   std::vector<std::shared_ptr<Ride>> dispatches to the
//                   right override at run time.
#pragma once

#include <string>

enum class RideStatus { Requested, Assigned, Completed };

std::string toString(RideStatus status);
std::string formatMoney(double amount);   // 11.6 -> "$11.60"
std::string formatMiles(double miles);    // 5.2  -> "5.2 mi"

class Ride {
public:
    Ride(std::string rideID, std::string pickupLocation,
         std::string dropoffLocation, double distance);

    // A class meant to be used polymorphically needs a virtual destructor so
    // deleting through a Ride pointer runs the subclass destructor too.
    virtual ~Ride() = default;

    // Polymorphic interface ------------------------------------------------
    virtual double fare() const;              // base pricing: $1.50 per mile
    virtual std::string rideType() const;     // "Ride", "Standard", ...
    virtual std::string rideDetails() const;  // one-line summary

    // Upfront pricing (like Lyft): the fare is quoted and locked in when the
    // rider requests the ride. This is a non-virtual method that calls the
    // virtual fare(), so it always uses the subclass pricing.
    double lockInFare();
    bool isFareLocked() const { return fareLocked_; }
    double lockedFare() const;  // falls back to fare() if not locked yet

    // Lifecycle -------------------------------------------------------------
    void assignDriver(const std::string& driverID, const std::string& driverName);
    void complete();

    // Read-only accessors ----------------------------------------------------
    const std::string& rideID() const { return rideID_; }
    const std::string& pickupLocation() const { return pickupLocation_; }
    const std::string& dropoffLocation() const { return dropoffLocation_; }
    double distance() const { return distance_; }
    RideStatus status() const { return status_; }
    const std::string& driverID() const { return driverID_; }
    const std::string& driverName() const { return driverName_; }

protected:
    // Shared helper for subclasses: enforce a minimum fare.
    static double atLeast(double amount, double minimum);

private:
    std::string rideID_;
    std::string pickupLocation_;
    std::string dropoffLocation_;
    double distance_;
    double fare_ = 0.0;
    bool fareLocked_ = false;
    RideStatus status_ = RideStatus::Requested;
    std::string driverID_;
    std::string driverName_;
};
