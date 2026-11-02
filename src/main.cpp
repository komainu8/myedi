#include <exception>
#include <iostream>

#include "terminal.hpp"

void clear_screen() {
  std::cout << "\033[2J\033[H";
  std::cout.flush();
}

void draw() {
  clear_screen();

  std::cout << "MyEdi\n";
  std::cout << "Press 'q' to quit\n";
  std::cout.flush();
}

int main() {
  try {
    Terminal terminal;

    terminal.enable_raw_mode();

    draw();

    while (true) {
      const char key = terminal.read_key();
      if (key == 'q') {
        break;
      }

      std::cout << key;
      std::cout.flush();
    }

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
