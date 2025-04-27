#include "packet.hpp"
#include <iomanip>

namespace network {

void printBytes(std::string s) {
  for (char c : s) {
    std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)c << " ";
  }
  std::cout << std::endl;
}

namespace internal {
// Creates the header of the packet
// like assings packet type etc.
std::string createPacketHeader(PacketType type,
                               PacketContentLength contentLength) {

  static_assert(sizeof((VERSION)) == 4, "Version length != 4");

  std::string header;

  LOG_DEBUG("size of type,length: ", sizeof(type), " ", sizeof(contentLength));
  header.reserve(HEADER_LENGTH_BYTES);
  LOG_DEBUG("Encoded header length", header.size());
  header.append(VERSION, sizeof(VERSION));

  for (int i = 0; i < sizeof(type); ++i) {
    header.push_back((uint8_t)(type >> i * 8));
  }
  LOG_DEBUG("Encoded header length", header.size());

  for (int i = 0; i < sizeof(contentLength); ++i) {
    header.push_back((uint8_t)(contentLength >> i * 8));
  }

  LOG_DEBUG("Encoded header length", header.size());

  return header;
}
} // namespace internal
} // namespace network
