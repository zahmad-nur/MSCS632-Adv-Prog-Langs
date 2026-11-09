#!/usr/bin/env bash
# Run the SUnit tests. Requires GNU Smalltalk: brew install gnu-smalltalk
cd "$(dirname "$0")"
gst-sunit -v \
    -f src/Extensions.st -f src/Ride.st -f src/RideTypes.st -f src/Driver.st \
    -f src/Rider.st -f src/RideSharingSystem.st -f tests/RideSharingTest.st \
    RideSharingTest
