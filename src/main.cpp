#include <exception>
#include <iostream>

#include "terminal.hpp"

int main() {
  try {
    Terminal terminal;

    std::cout << "MyEdi\n";
    std::cout << "---------------------------\n";
    std::cout << "Press Enter to exit.\n";
    std::cout.flush();

    std::cin.get();

    if (!terminal.restore()) {
      std::cerr << "Failed to restore terminal settings.\n";
      return -1;
    }
  } catch (const std::exception &error) {
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
