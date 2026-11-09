// main.cpp - demo of the Ride Sharing System (C++).
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Driver.h"
#include "RideSharingSystem.h"
#include "RideTypes.h"
#include "Rider.h"

namespace {

void banner(const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
}

void reportMatch(const Rider& rider, const Ride& ride, const MatchResult& match) {
    std::cout << "  " << rider.name() << " booked " << ride.rideID() << " (" << ride.rideType()
              << ", " << formatMoney(ride.lockedFare()) << ") -> ";
    if (!match.driver) {
        std::cout << "no drivers available, ride is waiting\n";
    } else if (match.viaFavorite) {
        std::cout << "matched with favorite driver " << match.driver->name() << '\n';
    } else {
        std::cout << "matched with top-rated available driver " << match.driver->name()
                  << " (" << std::fixed << std::setprecision(2)
                  << match.driver->rating() << ")\n";
    }
}

}  // namespace

int main() {
    std::cout << "Ride Sharing System - C++ demo\n";

    // ------------------------------------------------------------------
    banner("1. Polymorphism: same 6.0-mile trip, different ride types");
    std::vector<std::shared_ptr<Ride>> quotes = {
        std::make_shared<Ride>("Q1", "Midtown", "Riverside", 6.0),
        std::make_shared<StandardRide>("Q2", "Midtown", "Riverside", 6.0),
        std::make_shared<PremiumRide>("Q3", "Midtown", "Riverside", 6.0),
        std::make_shared<PoolRide>("Q4", "Midtown", "Riverside", 6.0, 1),
        std::make_shared<PoolRide>("Q5", "Midtown", "Riverside", 6.0, 3),
    };
    for (const auto& ride : quotes) {
        // One call site, five different fare() implementations.
        std::cout << "  " << std::left << std::setw(9) << ride->rideType()
                  << std::right << std::setw(8) << formatMoney(ride->fare()) << '\n';
    }

    // ------------------------------------------------------------------
    banner("2. Drivers and riders");
    RideSharingSystem app;
    auto maya  = app.addDriver("D01", "Maya Chen",    4.92, 250);
    auto jamal = app.addDriver("D02", "Jamal Brooks", 4.85, 180);
    auto priya = app.addDriver("D03", "Priya Nair",   4.97, 310);
    auto luis  = app.addDriver("D04", "Luis Ortega",  4.70, 95);
    for (const auto& d : app.drivers()) std::cout << "  " << d->getDriverInfo() << '\n';

    auto sofia = app.addRider("U01", "Sofia");
    auto omar  = app.addRider("U02", "Omar");
    sofia->addFavoriteDriver("D02");  // Jamal first
    sofia->addFavoriteDriver("D01");  // then Maya
    std::cout << "  Sofia's favorite drivers: Jamal Brooks, Maya Chen\n";
    std::cout << "  Omar has no favorites\n";

    // ------------------------------------------------------------------
    banner("3. Booking rides (favorites are checked first)");
    auto r1 = std::make_shared<StandardRide>("R001", "Home", "Campus", 5.2);
    auto r2 = std::make_shared<PremiumRide>("R002", "Downtown", "Airport", 12.4, "Lux Black SUV");
    auto r3 = std::make_shared<PoolRide>("R003", "Campus", "Library", 2.1, 2);
    auto r4 = std::make_shared<PoolRide>("R004", "Gym", "Downtown", 7.8, 1);
    auto r5 = std::make_shared<StandardRide>("R005", "Mall", "Home", 3.0);

    reportMatch(*sofia, *r1, app.bookRide(*sofia, r1));  // Jamal (favorite #1)
    reportMatch(*omar,  *r2, app.bookRide(*omar,  r2));  // Priya (top rated)
    reportMatch(*sofia, *r3, app.bookRide(*sofia, r3));  // Jamal busy -> Maya
    reportMatch(*omar,  *r4, app.bookRide(*omar,  r4));  // only Luis left
    reportMatch(*omar,  *r5, app.bookRide(*omar,  r5));  // everyone busy

    // ------------------------------------------------------------------
    banner("4. Completing rides and rating drivers");
    app.completeRide("R001", 5);
    app.completeRide("R002", 4);
    app.completeRide("R003", 5);
    app.completeRide("R004", 3);
    std::cout << "  Completed R001-R004 with ratings 5, 4, 5, 3\n";
    auto r6 = std::make_shared<PremiumRide>("R006", "Campus", "Concert Hall", 4.0);
    reportMatch(*sofia, *r6, app.bookRide(*sofia, r6));  // Jamal free again
    app.completeRide("R006", 5);

    // ------------------------------------------------------------------
    banner("5. All rides, processed polymorphically");
    double gross = 0.0;
    for (const auto& ride : app.allRides()) {
        std::cout << "  " << ride->rideDetails() << '\n';  // virtual dispatch
        gross += ride->lockedFare();
    }
    std::cout << "  Gross bookings: " << formatMoney(gross) << '\n';

    // ------------------------------------------------------------------
    banner("6. Rider history (viewRides)");
    std::cout << sofia->viewRides() << "\n\n" << omar->viewRides() << '\n';

    // ------------------------------------------------------------------
    banner("7. Driver details (getDriverInfo)");
    for (const auto& d : app.drivers()) std::cout << "  " << d->getDriverInfo() << '\n';

    // ------------------------------------------------------------------
    banner("8. Encapsulation checks");
    auto copy = jamal->rides();
    copy.clear();
    std::cout << "  Cleared the list returned by jamal->rides(); Jamal still has "
              << jamal->rideCount() << " rides\n";
    // jamal->assignedRides_.clear();   // compile error: 'assignedRides_' is private
    // copy[0]->complete();             // compile error: rides() hands out const Ride
    try {
        maya->addRating(7);
    } catch (const std::out_of_range& e) {
        std::cout << "  addRating(7) rejected: " << e.what() << '\n';
    }
    std::cout << "  Maya's rating is still " << std::fixed << std::setprecision(2)
              << maya->rating() << '\n';
    (void)luis;
    return 0;
}
