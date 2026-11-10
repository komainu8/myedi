#pragma once

#include <cerrno>
#include <stdexcept>
#include <system_error>

#include <termios.h>
#include <unistd.h>

enum class KeyType {
  Character,
  ArrowUp,
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  Escape,
  UnKnown
};

struct KeyEvent {
  KeyType type{KeyType::UnKnown};
  char character{};
};

class Terminal {
public:
  Terminal() {
    if (::isatty(STDIN_FILENO) == 0) {
      throw std::system_error(errno != 0 ? errno : ENOTTY,
                              std::generic_category(),
                              "stdin is not a terminal"
                              );
    }
    if (::tcgetattr(STDIN_FILENO, &original_) == -1) {
      throw std::system_error(errno, std::generic_category(),
                              "tcgetattr failed");
    }
    current_ = original_;
  }
  ~Terminal() noexcept {
    if (modified_) {
      (void)restore();
    }
  }

  /*
   * Terminal manages the terminal settings.
   * Copying or moving could result in multiple objects managing the same
   * terminal settings and attempting to restore them.
   * Therefore, copying and moving are disabled.
   */
  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;
  Terminal(Terminal &&) = delete;
  Terminal &operator=(Terminal &&) = delete;

  void enable_raw_mode() {
    termios raw_mode_settings = original_;
    raw_mode_settings.c_lflag &= ~(ICANON | ECHO);

    if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode_settings) == -1) {
      throw std::system_error(errno, std::generic_category(),
                              "tcsetattr failed");
    }

    modified_ = true;
  }

  char read_key() {
    char key{};

    const size_t READ_SIZE = 1;
    const ssize_t read_result = ::read(STDIN_FILENO, &key, READ_SIZE);

    if (read_result == -1) {
      throw std::system_error(errno, std::generic_category(),
                              "read from stdin failed");
    }
    if (read_result == 0) {
      throw std::runtime_error("stdin reached EOF");
    }

    return key;
  }


  [[nodiscard]]
  bool restore() noexcept {
    if (!modified_) {
      return true;
    }

    if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_) == -1) {
      return false;
    }

    current_ = original_;
    modified_ = false;

    return true;
  }

private:
  termios original_{};
  termios current_{};

  bool modified_ = false;
};
