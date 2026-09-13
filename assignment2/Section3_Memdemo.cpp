//Section 3 Memory Demo for Cpp

#include <iostream>

// Deliberately leaks: allocates but never frees
void leaky() {
    int* arr = new int[1000];
    for (int i = 0; i < 1000; i++) arr[i] = i;
    std::cout << "leaky(): allocated 1000 ints, never freed\n";
    // no delete[] arr; -- intentional leak
}

// Deliberately creates a dangling pointer, then reads through it
void dangling() {
    int* p = new int[10];
    for (int i = 0; i < 10; i++) p[i] = i * 2;
    delete[] p;
    std::cout << "dangling(): p[0] after free = " << p[0] << "\n"; // use-after-free
}

int main() {
    leaky();
    dangling();
    std::cout << "done\n";
    return 0;
}