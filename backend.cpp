#include <array>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include "sensor_packet.hpp"

constexpr int         PORT     = 8080;
constexpr const char* SHM_NAME = "/sport_tech_shm";

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) return 1;

    ftruncate(shm_fd, sizeof(SensorPacket));
    auto* shm_ptr = static_cast<SensorPacket*>(
        mmap(nullptr, sizeof(SensorPacket), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)
    );
    if (shm_ptr == MAP_FAILED) return 1;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return 1;

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(sockfd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) return 1;

    std::array<char, 1024> buffer{};

    while (true) {
        ssize_t bytes = recvfrom(sockfd, buffer.data(), buffer.size(), 0, nullptr, nullptr);
        if (bytes == sizeof(SensorPacket)) {
            std::memcpy(shm_ptr, buffer.data(), sizeof(SensorPacket));
        }
    }

    return 0;
}
