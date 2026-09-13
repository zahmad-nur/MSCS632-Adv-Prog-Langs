//Section 2 Cpp Combine Program
#include <iostream>
using namespace std;

int combine(int a, int b) {
    return a + b;
}

int main() {
    cout << combine(5, 3) << endl;   // 8
    cout << combine("5", "3") << endl
    //would not compile — type mismatch caught before runtime
    return 0;
}