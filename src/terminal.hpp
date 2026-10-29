#pragma once

#include <cerrno>
#include <system_error>

#include <termios.h>
#include <unistd.h>

class Terminal {
public:
  Terminal() {
    if (::isatty(STDIN_FILENO, &original_) == 0) {
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

private:
  termios original_{};
  termios current_{};

  bool modified_ = false;
};
