"Ride.st - base class for every ride type.

 OOP notes
   Encapsulation : instance variables are always private in Smalltalk; other
                   objects can only reach them through the messages below.
   Inheritance   : StandardRide, PremiumRide and PoolRide are subclasses.
   Polymorphism  : every message send is dynamically dispatched, so sending
                   #fare or #rideDetails to any ride runs that ride's own
                   method. No 'virtual' keyword is needed."

Object subclass: Ride [
    | rideID pickupLocation dropoffLocation distance fare fareLocked status driverID driverName |

    <comment: 'A trip from a pickup location to a dropoff location.'>

    Ride class >> id: anID from: pickup to: dropoff miles: miles [
        ^self new setId: anID from: pickup to: dropoff miles: miles
    ]

    setId: anID from: pickup to: dropoff miles: miles [
        <category: 'private'>
        miles > 0 ifFalse: [
            ^SystemExceptions.InvalidArgument signalOn: miles
                reason: 'distance must be positive'].
        rideID := anID.
        pickupLocation := pickup.
        dropoffLocation := dropoff.
        distance := miles.
        fare := 0.
        fareLocked := false.
        status := #requested
    ]

    "---------------------------------------------- polymorphic interface"

    fare [
        "Base pricing: $1.50 per mile. Subclasses override this."
        ^distance * 1.50
    ]

    rideType [ ^'Ride' ]

    rideDetails [
        | s |
        s := WriteStream on: String new.
        s nextPutAll: '['; nextPutAll: rideID; nextPutAll: '] ';
            nextPutAll: (self rideType paddedTo: 8);
            nextPutAll: ' | '; nextPutAll: pickupLocation;
            nextPutAll: ' -> '; nextPutAll: dropoffLocation;
            nextPutAll: ' | '; nextPutAll: distance asMiles;
            nextPutAll: ' | '; nextPutAll: self lockedFare asMoney;
            nextPutAll: ' | '; nextPutAll: status asString.
        driverName isNil ifFalse: [
            s nextPutAll: ' (driver: '; nextPutAll: driverName; nextPutAll: ')'].
        ^s contents
    ]

    "------------------------------------------------- upfront pricing"

    lockInFare [
        "Quote and lock the price when the ride is requested (like Lyft).
         'self fare' dispatches to the subclass's pricing."
        fareLocked ifFalse: [fare := self fare. fareLocked := true].
        ^fare
    ]

    isFareLocked [ ^fareLocked ]

    lockedFare [ ^fareLocked ifTrue: [fare] ifFalse: [self fare] ]

    "------------------------------------------------------- lifecycle"

    assignDriverId: anID name: aName [
        status = #requested ifFalse: [
            ^self error: 'ride ', rideID, ' already has a driver'].
        driverID := anID.
        driverName := aName.
        status := #assigned
    ]

    complete [
        status = #assigned ifFalse: [
            ^self error: 'ride ', rideID, ' must be assigned before completing'].
        status := #completed
    ]

    "------------------------------------------------ read-only access"

    rideID [ ^rideID ]
    pickupLocation [ ^pickupLocation ]
    dropoffLocation [ ^dropoffLocation ]
    distance [ ^distance ]
    status [ ^status ]
    driverID [ ^driverID ]
    driverName [ ^driverName ]
]
