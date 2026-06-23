#include "serial_port.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace {

static speed_t to_baud(int baud) {
    switch (baud) {
    case 9600:   return B9600;
    case 19200:  return B19200;
    case 38400:  return B38400;
    case 57600:  return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    default:     return B115200;
    }
}

} // namespace

SerialPort::SerialPort() = default;

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open(const SerialConfig& cfg) {
    close();

    read_timeout_ms_ = cfg.read_timeout_ms;

    fd_ = ::open(cfg.port.c_str(), O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (fd_ < 0) return false;

    termios tty{};
    if (tcgetattr(fd_, &tty) != 0) {
        close();
        return false;
    }

    cfmakeraw(&tty);

    const auto baud = to_baud(cfg.baudrate);
    cfsetispeed(&tty, baud);
    cfsetospeed(&tty, baud);

    tty.c_cflag |= (CLOCAL | CREAD);

    tty.c_cflag &= ~CSIZE;
    switch (cfg.databits) {
    case 5: tty.c_cflag |= CS5; break;
    case 6: tty.c_cflag |= CS6; break;
    case 7: tty.c_cflag |= CS7; break;
    case 8: default: tty.c_cflag |= CS8; break;
    }

    if (cfg.stopbits == 2) tty.c_cflag |= CSTOPB;
    else tty.c_cflag &= ~CSTOPB;

    tty.c_cflag &= ~(PARENB | PARODD);
    if (cfg.parity == "even") tty.c_cflag |= PARENB;
    else if (cfg.parity == "odd") tty.c_cflag |= (PARENB | PARODD);

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        close();
        return false;
    }

    return true;
}

void SerialPort::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool SerialPort::is_open() const {
    return fd_ >= 0;
}

std::optional<std::string> SerialPort::read_some(std::size_t maxBytes) {
    if (fd_ < 0) return std::string{};

    pollfd pfd{};
    pfd.fd     = fd_;
    pfd.events = POLLIN;

    const int pr = ::poll(&pfd, 1, read_timeout_ms_);
    if (pr == 0) return std::nullopt;
    if (pr < 0) {
        if (errno == EINTR) return std::nullopt;
        return std::string{};
    }

    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
        return std::string{};

    std::string buf;
    buf.resize(maxBytes);
    const auto n = ::read(fd_, buf.data(), buf.size());
    if (n == 0) return std::string{};
    if (n < 0) {
        if (errno == EAGAIN || errno == EINTR) return std::nullopt;
        return std::string{};
    }

    buf.resize(static_cast<std::size_t>(n));
    return buf;
}

bool SerialPort::write_some(const std::string& data) {
    if (fd_ < 0 || data.empty()) return false;

    std::size_t written = 0;
    while (written < data.size()) {
        const ssize_t n = ::write(fd_, data.data() + written, data.size() - written);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        written += static_cast<std::size_t>(n);
    }
    return true;
}
