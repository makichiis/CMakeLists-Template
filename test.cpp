/**
 * @file test.cpp 
 * @brief A bad driver source that should aggravate AddressSanitizer.
 *
 * @author makichiis (Sarah-Laurie Evans)
 */

#include <iostream>

int main() {
	std::cout << "Hello, world!\n";

	int a[5];
	a[5] = 0;
}
