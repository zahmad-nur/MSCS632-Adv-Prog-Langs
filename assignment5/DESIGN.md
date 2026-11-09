# Design and Testing Notes

This document explains how the Ride Sharing System works and how it is tested. The accompanying report focuses on how C++ and Smalltalk express encapsulation, inheritance, and polymorphism. Both implementations follow the same design, so everything below applies to both.

## Inspiration

The system is loosely modeled on Lyft. Fares are quoted upfront when a ride is requested, riders can choose a cheaper shared Pool ride, and riders can mark favorite drivers who are offered their rides first.

## Classes

```
Ride                    base fare $1.50/mi
 ├── StandardRide       $2.50 + $1.75/mi, minimum $7.00
 │    └── PoolRide      Standard price minus 20% + 5% per co-rider (max 40%), minimum $5.00
 └── PremiumRide        $5.00 + $3.25/mi + $2.50 booking fee, minimum $15.00

Driver                  driverID, name, rating, assignedRides (private)
Rider                   riderID, name, requestedRides, favoriteDrivers
RideSharingSystem       books rides and matches drivers
```

**Ride** holds the ride ID, pickup and dropoff locations, distance, the locked-in fare, and a status that moves from *requested* to *assigned* to *completed*. Each subclass overrides `fare()` with its own pricing. `rideDetails()` returns a one-line summary, and Premium and Pool rides extend the parent's summary with their vehicle class or co-rider discount. `lockInFare()` is called when a ride is requested. It calls the overridden `fare()` and stores the result, so the rider pays the price they were quoted.

**Driver** keeps its list of assigned rides private. Rides are added with `addRide()`, and `rides()` returns a copy of the list. The star rating is a running average that changes only through `addRating()`, which rejects anything outside 1 to 5. A driver is available when none of their rides is in progress. Their earnings count completed rides only.

**Rider** keeps the rides they have requested and an ordered list of favorite driver IDs, with the most preferred driver first. `viewRides()` prints the rider's history and `totalSpent()` sums their fares.

**RideSharingSystem** books rides. When a rider books, the system first offers the ride to the rider's favorite drivers in order of preference. If none of them is available, it assigns the highest-rated available driver. If every driver is busy, the ride waits as *requested*. Completing a ride records the rider's star rating for the driver.

## Demo

The demo runs the same scenario in both languages, and the two produce identical output.

1. Quotes the same 6-mile trip as every ride type, using a single polymorphic loop.
2. Creates four drivers and two riders. Sofia's favorite drivers are Jamal, then Maya. Omar has no favorites.
3. Books five rides. This shows a favorite being matched, a fallback to the second favorite, the top-rated driver being matched, and a ride left waiting when every driver is busy.
4. Completes four rides with star ratings, then books another ride once Sofia's first-choice favorite is free again.
5. Prints every ride polymorphically through `rideDetails()`.
6. Prints each rider's history (`viewRides()`).
7. Prints each driver's details (`getDriverInfo()`).
8. Checks encapsulation. Clearing the copy returned by `rides()` leaves the driver's own list intact, and an invalid rating is rejected. In Smalltalk, asking for `assignedRides` directly fails with `doesNotUnderstand`.

## Tests

Both versions run the same 11 tests, using doctest in C++ (`cpp/tests/test_rideshare.cpp`) and SUnit in Smalltalk (`smalltalk/tests/RideSharingTest.st`).

| Test | What it checks | Principle |
|---|---|---|
| Each ride type overrides `fare()` | A 6-mile trip costs $9.00 (Ride), $13.00 (Standard), $27.00 (Premium), and $9.75 or $8.45 (Pool) | Inheritance |
| Minimum fares | Short trips are raised to each type's minimum ($7, $15, $5) | Inheritance |
| Pool discount cap | The discount is 20% plus 5% per co-rider, never above 40% | Inheritance |
| Invalid ride input | Zero distance and negative co-riders are rejected | Encapsulation |
| Polymorphic collection | One list of mixed rides returns each subclass's fare and details | Polymorphism |
| Fare locked at request | Requesting a ride locks in the subclass's price | Polymorphism |
| Driver rides stay private | `assignedRides` can't be reached from outside the class. Clearing the returned copy leaves the driver's list intact. In C++, a C++20 concept checks this at compile time. | Encapsulation |
| Ratings validated | Ratings outside 1 to 5 are rejected and the average updates correctly | Encapsulation |
| Ride lifecycle | A ride can't be completed before it is assigned, or assigned twice | Encapsulation |
| Favorites, then top-rated | A favorite gets the ride first. If they're busy, the highest-rated driver gets it. If nobody is free, the ride waits. | System |
| Favorites list | Favorites can't be duplicated, and removing one works | System |
