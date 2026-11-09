#include <cstddef>
#include <exception>
#include <iostream>
#include <string>

#include "terminal.hpp"

bool is_backspace(const char key) {
  return ((key == '\x7f'/* DEL */) || (key == '\x08'/* BS */));
}

void remove_character(std::string &text) {
  if (!text.empty()) {
    text.pop_back();
  }
}

void delete_character(std::string &text, std::size_t &cursor_position) {
  if (cursor_position > 0) {
    size_t n_character = 1;
    text.erase(cursor_position - 1, n_character);
    --cursor_position;
  }
}

void clear_screen() {
  std::cout << "\033[2J\033[H";
  std::cout.flush();
}

void draw(const std::string &text) {
  clear_screen();

  std::cout << "MyEdi\n";
  std::cout << "Press 'q' to quit\n";
  std::cout << text;
  std::cout.flush();
}

int main() {
  try {
    Terminal terminal;

    terminal.enable_raw_mode();

    std::string text{};
    draw(text);

    while (true) {
      const char key = terminal.read_key();
      if (key == 'q') {
        break;
      }

      if (is_backspace(key)) {
        remove_character(text);
      } else {
        text.push_back(key);
      }

      draw(text);
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
