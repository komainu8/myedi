#pragma once

#include <cerrno>
#include <system_error>

#include <termios.h>
#include <unistd.h>

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

  Terminal(const Terminal &) = delete;
  Terminal &operator=(const Terminal &) = delete;

  Terminal(Terminal &&) = delete;
  Terminal &operator=(Terminal &&) = delete;

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
