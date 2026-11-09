"RideTypes.st - concrete ride types. Each one overrides #fare.

   Ride
    |-- StandardRide        $2.50 + $1.75/mi, minimum $7.00
    |    `-- PoolRide       Standard price minus a shared-ride discount
    `-- PremiumRide         $5.00 + $3.25/mi + $2.50 booking fee, minimum $15.00"

Ride subclass: StandardRide [
    <comment: 'An everyday ride.'>

    fare [ ^(2.50 + (1.75 * self distance)) max: 7.00 ]

    rideType [ ^'Standard' ]
]

Ride subclass: PremiumRide [
    | vehicleClass |

    <comment: 'A higher-end vehicle with a booking fee.'>

    PremiumRide class >> id: anID from: pickup to: dropoff miles: miles vehicle: aClass [
        ^(self id: anID from: pickup to: dropoff miles: miles) setVehicle: aClass
    ]

    setVehicle: aClass [
        <category: 'private'>
        vehicleClass := aClass
    ]

    vehicleClass [ ^vehicleClass ifNil: ['Lux Black'] ]

    fare [
        | bookingFee |
        bookingFee := 2.50.
        ^(5.00 + (3.25 * self distance) + bookingFee) max: 15.00
    ]

    rideType [ ^'Premium' ]

    rideDetails [
        "Extend the inherited description instead of rewriting it."
        ^super rideDetails, ' | vehicle: ', self vehicleClass
    ]
]

StandardRide subclass: PoolRide [
    | coRiders |

    <comment: 'A shared ride. Inherits from StandardRide and discounts its price.'>

    PoolRide class >> id: anID from: pickup to: dropoff miles: miles coRiders: count [
        ^(self id: anID from: pickup to: dropoff miles: miles) setCoRiders: count
    ]

    setCoRiders: count [
        <category: 'private'>
        count < 0 ifTrue: [
            ^SystemExceptions.InvalidArgument signalOn: count
                reason: 'coRiders cannot be negative'].
        coRiders := count
    ]

    coRiders [ ^coRiders ifNil: [0] ]

    discountRate [
        "20% plus 5% per co-rider, capped at 40%."
        ^(0.20 + (0.05 * self coRiders)) min: 0.40
    ]

    fare [
        "Reuse the parent's pricing through super, then discount it."
        ^(super fare * (1 - self discountRate)) max: 5.00
    ]

    rideType [ ^'Pool' ]

    rideDetails [
        ^super rideDetails, ' | ', self coRiders printString, ' co-rider',
            (self coRiders = 1 ifTrue: [''] ifFalse: ['s']), ', ',
            (self discountRate * 100) rounded printString, '% off'
    ]
]
