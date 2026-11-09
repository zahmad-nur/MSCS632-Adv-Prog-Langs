# Ride Sharing System: C++ and Smalltalk

This is my MSCS 632 Advanced Programming Languages assignment. It implements a Lyft-inspired ride sharing system twice, in C++ and in GNU Smalltalk, to compare how the two languages express encapsulation, inheritance, and polymorphism.

See [DESIGN.md](DESIGN.md) for how the system works, what the demo covers, and what each test checks.

## C++ (`cpp/`)

Requires a C++20 compiler (Apple clang from the Xcode Command Line Tools works).

```bash
cd cpp
make demo    # build and run the demo
make test    # build and run the unit tests (doctest, bundled in tests/)
```

## Smalltalk (`smalltalk/`)

Requires GNU Smalltalk (`brew install gnu-smalltalk`).

```bash
cd smalltalk
./run_demo.sh    # run the demo
./run_tests.sh   # run the SUnit tests
```

## Layout

```
DESIGN.md    design, demo walkthrough, test descriptions
cpp/
  include/   Ride.h, RideTypes.h, Driver.h, Rider.h, RideSharingSystem.h
  src/       matching .cpp files + main.cpp (demo)
  tests/     test_rideshare.cpp, doctest.h
smalltalk/
  src/       Extensions.st, Ride.st, RideTypes.st, Driver.st, Rider.st, RideSharingSystem.st
  tests/     RideSharingTest.st
  demo.st
```
