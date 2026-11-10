#pragma once

#include <cerrno>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/types.h>
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

  KeyEvent read_key() {
    const char first = read_byte();

    if (first != '\x1b') {
      return {KeyType::Character, first};
    }

    if (!wait_for_input(50)) {
      return {KeyType::Escape};
    }

    const char second = read_byte();

    if (second != '[') {
      return {KeyType::UnKnown};
    }

    if (!wait_for_input(50)) {
      return {KeyType::UnKnown};
    }

    const char third = read_byte();

    switch (third) {
    case 'A':
      return {KeyType::ArrowUp};
    case 'B':
      return {KeyType::ArrowDown};
    case 'C':
      return {KeyType::ArrowRight};
    case 'D':
      return {KeyType::ArrowLeft};
    default:
      return {KeyType::UnKnown};
    }
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

  char read_byte() {
    char byte{};
    ssize_t result{};

    do {
      result = ::read(STDIN_FILENO, &byte, 1);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
      const int error = errno;
      throw std::system_error(error, std::generic_category(), "read failed");
    }

    if (result == 0) {
      throw std::runtime_error("stdin reached EOF");
    }

    return byte;
  }

  bool wait_for_input(int timeout_millisec) {
    pollfd desc{};
    desc.fd = STDIN_FILENO;
    desc.events = POLLIN;

    int result{};

    do {
      result = ::poll(&desc, 1, timeout_millisec);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
      const int error = errno;
      throw std::system_error(error, std::generic_category(), "poll failed");
    }

    return result > 0 && (desc.revents & POLLIN) != 0;
  }
};
