"demo.st - demo of the Ride Sharing System (GNU Smalltalk).
 Run with ./run_demo.sh (loads src/*.st first)."

Eval [
    | banner report quotes app maya jamal priya luis sofia omar
      r1 r2 r3 r4 r5 r6 gross copy |

    banner := [:title | Transcript cr; showCr: '=== ', title, ' ==='].
    report := [:rider :ride :match |
        Transcript show: '  ', rider name, ' booked ', ride rideID,
            ' (', ride rideType, ', ', ride lockedFare asMoney, ') -> '.
        match found
            ifFalse: [Transcript showCr: 'no drivers available, ride is waiting']
            ifTrue: [match viaFavorite
                ifTrue: [Transcript showCr: 'matched with favorite driver ', match driver name]
                ifFalse: [Transcript showCr: 'matched with top-rated available driver ',
                    match driver name, ' (', (match driver rating asFixed: 2), ')']]].

    Transcript showCr: 'Ride Sharing System - Smalltalk demo'.

    "--------------------------------------------------------------"
    banner value: '1. Polymorphism: same 6.0-mile trip, different ride types'.
    quotes := OrderedCollection new.
    quotes
        add: (Ride id: 'Q1' from: 'Midtown' to: 'Riverside' miles: 6.0);
        add: (StandardRide id: 'Q2' from: 'Midtown' to: 'Riverside' miles: 6.0);
        add: (PremiumRide id: 'Q3' from: 'Midtown' to: 'Riverside' miles: 6.0);
        add: (PoolRide id: 'Q4' from: 'Midtown' to: 'Riverside' miles: 6.0 coRiders: 1);
        add: (PoolRide id: 'Q5' from: 'Midtown' to: 'Riverside' miles: 6.0 coRiders: 3).
    quotes do: [:ride |
        "One message send, five different #fare methods."
        Transcript showCr: '  ', (ride rideType paddedTo: 9),
            (ride fare asMoney rightAlignedTo: 8)].

    "--------------------------------------------------------------"
    banner value: '2. Drivers and riders'.
    app := RideSharingSystem new.
    maya  := app addDriverId: 'D01' name: 'Maya Chen'    rating: 4.92 count: 250.
    jamal := app addDriverId: 'D02' name: 'Jamal Brooks' rating: 4.85 count: 180.
    priya := app addDriverId: 'D03' name: 'Priya Nair'   rating: 4.97 count: 310.
    luis  := app addDriverId: 'D04' name: 'Luis Ortega'  rating: 4.70 count: 95.
    app drivers do: [:d | Transcript showCr: '  ', d getDriverInfo].

    sofia := app addRiderId: 'U01' name: 'Sofia'.
    omar  := app addRiderId: 'U02' name: 'Omar'.
    sofia addFavoriteDriver: 'D02'.   "Jamal first"
    sofia addFavoriteDriver: 'D01'.   "then Maya"
    Transcript showCr: '  Sofia''s favorite drivers: Jamal Brooks, Maya Chen'.
    Transcript showCr: '  Omar has no favorites'.

    "--------------------------------------------------------------"
    banner value: '3. Booking rides (favorites are checked first)'.
    r1 := StandardRide id: 'R001' from: 'Home' to: 'Campus' miles: 5.2.
    r2 := PremiumRide id: 'R002' from: 'Downtown' to: 'Airport' miles: 12.4 vehicle: 'Lux Black SUV'.
    r3 := PoolRide id: 'R003' from: 'Campus' to: 'Library' miles: 2.1 coRiders: 2.
    r4 := PoolRide id: 'R004' from: 'Gym' to: 'Downtown' miles: 7.8 coRiders: 1.
    r5 := StandardRide id: 'R005' from: 'Mall' to: 'Home' miles: 3.0.

    report value: sofia value: r1 value: (app bookRide: r1 for: sofia).  "Jamal (favorite #1)"
    report value: omar  value: r2 value: (app bookRide: r2 for: omar).   "Priya (top rated)"
    report value: sofia value: r3 value: (app bookRide: r3 for: sofia).  "Jamal busy -> Maya"
    report value: omar  value: r4 value: (app bookRide: r4 for: omar).   "only Luis left"
    report value: omar  value: r5 value: (app bookRide: r5 for: omar).   "everyone busy"

    "--------------------------------------------------------------"
    banner value: '4. Completing rides and rating drivers'.
    app completeRide: 'R001' stars: 5.
    app completeRide: 'R002' stars: 4.
    app completeRide: 'R003' stars: 5.
    app completeRide: 'R004' stars: 3.
    Transcript showCr: '  Completed R001-R004 with ratings 5, 4, 5, 3'.
    r6 := PremiumRide id: 'R006' from: 'Campus' to: 'Concert Hall' miles: 4.0.
    report value: sofia value: r6 value: (app bookRide: r6 for: sofia).  "Jamal free again"
    app completeRide: 'R006' stars: 5.

    "--------------------------------------------------------------"
    banner value: '5. All rides, processed polymorphically'.
    gross := 0.
    app allRides do: [:ride |
        Transcript showCr: '  ', ride rideDetails.   "dynamic dispatch"
        gross := gross + ride lockedFare].
    Transcript showCr: '  Gross bookings: ', gross asMoney.

    "--------------------------------------------------------------"
    banner value: '6. Rider history (viewRides)'.
    Transcript showCr: sofia viewRides; cr; showCr: omar viewRides.

    "--------------------------------------------------------------"
    banner value: '7. Driver details (getDriverInfo)'.
    app drivers do: [:d | Transcript showCr: '  ', d getDriverInfo].

    "--------------------------------------------------------------"
    banner value: '8. Encapsulation checks'.
    copy := jamal rides.
    copy removeAll: copy copy.
    Transcript showCr: '  Emptied the collection returned by jamal rides; Jamal still has ',
        jamal rideCount printString, ' rides'.
    [jamal assignedRides]
        on: MessageNotUnderstood
        do: [:e | Transcript showCr: '  jamal assignedRides -> doesNotUnderstand: #',
                e message selector, ' (no accessor, so the list stays private)'.
              e return: nil].
    [maya addRating: 7]
        on: SystemExceptions.ArgumentOutOfRange
        do: [:e | Transcript showCr: '  addRating: 7 rejected: ', e messageText.
              e return: nil].
    Transcript showCr: '  Maya''s rating is still ', (maya rating asFixed: 2)
]
