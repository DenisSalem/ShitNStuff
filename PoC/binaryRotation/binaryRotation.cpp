#include <iostream>

#define BITS_ROTATION 24

int main(int argc, char **argv) {
  float         original = 0.12345678;
  unsigned int  processed = (unsigned int) (original * (float) (1 << BITS_ROTATION));
  float         restored = (float) processed / (float) (1 << BITS_ROTATION);

  std::cout.precision(8);
  std::cout << "Original:\t\t" << original << "\n";
  std::cout << "As unsigned integer:\t" << processed << "\n";
  std::cout << "Restored:\t\t" << restored << "\n";
  return 0;
}
