#include <iostream>

int main() {
  std::cout << "\033[2J\033[H";

  std::cout << "MyEdi\n";
  std::cout << "---------------------------\n";
  std::cout << "Press Enter to exit.\n";
  std::cout.flush();

  std::cin.get();

  std::cout << "\033[2J\033[H";
  std::cout.flush();

  return 0;
}
