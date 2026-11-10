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

void insert_character(std::string &text, std::size_t &cursor_position,
                      char insert_char) {
  size_t n_character = 1;
  text.insert(cursor_position, n_character, insert_char);
  ++cursor_position;
}

void draw(const std::string &text, std::size_t cursor_position) {
  clear_screen();

  std::cout << "MyEdi\n";
  std::cout << "Press 'Ctrl-G' to quit\n";
  std::cout << text;

  std::cout << "\033[3;" << cursor_position + 1 << "H";
  std::cout.flush();
}

void move_cursor_head(std::size_t &cursor_position) {
  cursor_position = 0;
}

void move_cursor_tail(std::size_t &cursor_position, const std::string &text) {
  cursor_position = text.size();
}

void move_cursor_left(std::size_t &cursor_position) {
  if (cursor_position > 0) {
    --cursor_position;
  }
}

void move_cursor_right(std::size_t &cursor_position, const std::string &text) {
  if (cursor_position < text.max_size()) {
    cursor_position++;
  }
}

int main() {
  try {
    Terminal terminal;

    terminal.enable_raw_mode();

    std::string text{};
    std::size_t cursor_position{0};

    draw(text, cursor_position);

    while (true) {
      const KeyEvent key = terminal.read_key();

      if (key.type == KeyType::Character) {
        const char character = key.character;

        if (character == '\x07') {
          break;
        } else if (character == '\x01') {
          move_cursor_head(cursor_position);
        } else if (character == '\x05') {
          move_cursor_tail(cursor_position, text);
        } else if (character == '\x02') {
          move_cursor_left(cursor_position);
        } else if (character == '\x06') {
          move_cursor_right(cursor_position, text);
        } else if (is_backspace(character)) {
          delete_character(text, cursor_position);
        } else if (character >= ' ' && character <= '~') {
          insert_character(text, cursor_position, character);
        }
      } else if (key.type == KeyType::ArrowLeft) {
        move_cursor_left(cursor_position);
      } else if (key.type == KeyType::ArrowRight) {
        move_cursor_right(cursor_position, text);
      } else if (key.type == KeyType::ArrowUp) {
        ;
      } else if (key.type == KeyType::ArrowDown) {
        ;
      }

      draw(text, cursor_position);
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
