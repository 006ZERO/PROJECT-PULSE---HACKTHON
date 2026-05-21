#include "sensor_packet.hpp"
#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr const char *SERVER_IP = "192.168.1.184";
constexpr int PORT = 8080;
constexpr int LOOP_US = 10000;

// ADXL345 (inside GY-85)
constexpr uint8_t ADXL345_ADDR = 0x53;
constexpr uint8_t ADXL_POWER_CTL = 0x2D;
constexpr uint8_t ADXL_DATA_FORMAT = 0x31;
constexpr uint8_t ADXL_DATAX0 = 0x32;

// MAX30100
constexpr uint8_t MAX30100_ADDR = 0x57;
constexpr uint8_t MAX_MODE_CONFIG = 0x06;
constexpr uint8_t MAX_SPO2_CONFIG = 0x07;
constexpr uint8_t MAX_LED_CONFIG = 0x09;
constexpr uint8_t MAX_FIFO_DATA = 0x05;
constexpr uint8_t MAX_FIFO_WR_PTR = 0x02;
constexpr uint8_t MAX_FIFO_RD_PTR = 0x04;
constexpr uint8_t MAX_INT_STATUS = 0x00;

static int open_i2c(const char *bus, uint8_t addr) {
  int fd = open(bus, O_RDWR);
  if (fd < 0)
    return -1;
  if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    close(fd);
    return -1;
  }
  return fd;
}

static bool write_reg(int fd, uint8_t reg, uint8_t val) {
  uint8_t buf[2] = {reg, val};
  return write(fd, buf, 2) == 2;
}

static bool read_regs(int fd, uint8_t reg, uint8_t *out, int len) {
  if (write(fd, &reg, 1) != 1)
    return false;
  return read(fd, out, len) == len;
}

static void init_adxl345(int fd) {
  write_reg(fd, ADXL_POWER_CTL, 0x08);   // measure mode
  write_reg(fd, ADXL_DATA_FORMAT, 0x08); // full resolution, +/-2g
  usleep(10000);
}

static void init_max30100(int fd) {
  write_reg(fd, MAX_MODE_CONFIG, 0x03); // HR + SpO2 mode
  write_reg(fd, MAX_SPO2_CONFIG, 0x47); // 100sps, 1600us pulse
  write_reg(fd, MAX_LED_CONFIG, 0x24);  // IR=2.4mA, Red=2.4mA
  usleep(100000);
}

static void read_accel(int fd, float &ax, float &ay, float &az) {
  uint8_t buf[6] = {};
  if (!read_regs(fd, ADXL_DATAX0, buf, 6))
    return;

  auto to_int16 = [](uint8_t l, uint8_t h) -> int16_t {
    return static_cast<int16_t>((h << 8) | l);
  };

  constexpr float SCALE = 1.0f / 256.0f; // ADXL345 full res scale
  ax = to_int16(buf[0], buf[1]) * SCALE;
  ay = to_int16(buf[2], buf[3]) * SCALE;
  az = to_int16(buf[4], buf[5]) * SCALE;
}

static uint32_t read_heart_rate(int fd) {
  uint8_t status = 0;
  read_regs(fd, MAX_INT_STATUS, &status, 1);

  uint8_t buf[4] = {};
  if (!read_regs(fd, MAX_FIFO_DATA, buf, 4))
    return 0;

  uint16_t ir = (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
  uint16_t red = (static_cast<uint16_t>(buf[2]) << 8) | buf[3];

  if (ir < 1000)
    return 60;
  if (ir < 5000)
    return 75;
  if (ir < 10000)
    return 90;
  if (ir < 20000)
    return 110;
  if (ir < 40000)
    return 130;
  return 155;
}

int main() {
  int adxl_fd = open_i2c("/dev/i2c-1", ADXL345_ADDR);
  int max_fd = open_i2c("/dev/i2c-1", MAX30100_ADDR);

  if (adxl_fd < 0) {
    fprintf(stderr, "Error: Cannot open ADXL345 on i2c-1 (0x53)\n");
    return 1;
  }
  if (max_fd < 0) {
    fprintf(stderr, "Error: Cannot open MAX30100 on i2c-1 (0x57)\n");
    return 1;
  }

  init_adxl345(adxl_fd);
  init_max30100(max_fd);

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    return 1;

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

  fprintf(stdout, "Streaming to %s:%d\n", SERVER_IP, PORT);

  while (true) {
    SensorPacket pkt{};

    read_accel(adxl_fd, pkt.accel_x, pkt.accel_y, pkt.accel_z);
    pkt.heart_rate = read_heart_rate(max_fd);
    pkt.timestamp_us = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());

    sendto(sockfd, &pkt, sizeof(pkt), 0,
           reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));

    usleep(LOOP_US);
  }

  close(adxl_fd);
  close(max_fd);
  close(sockfd);
  return 0;
}
