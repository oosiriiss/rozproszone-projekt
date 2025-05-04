#include "packet.hpp"
#include <cstdint>
#include <expected>
#include <iomanip>
#include <string>

namespace network {

void printBytes(const std::string &s) {

  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::hex << std::setfill('0') << std::setw(2);
  for (char c : s) {
    std::cout << ((uint32_t)c) % 256 << " ";
  }
  std::cout.flags(f);
  std::cout << std::endl;
}

std::string EnemyUpdateResponse::serialize() const {
  std::string b;
  return b;
}
void EnemyUpdateResponse::deserialize(std::string_view body) {}

namespace internal {
// Creates the header of the packet
// like assings packet type etc.
std::string createPacketHeader(PacketType type,
                               PacketContentLength contentLength) {

  static_assert(sizeof((VERSION)) == 4, "Version length == 4");

  std::string header = std::string(HEADER_LENGTH_BYTES, 'x');
  LOG_ERROR("HEADER: ", header);
  LOG_ERROR("HEADER LEN ", header.size());

  LOG_DEBUG("Encoded header length", header.size());
  std::memcpy(header.data(), VERSION, sizeof(VERSION));
  std::memcpy(header.data() + sizeof(VERSION), &type, sizeof(type));
  std::memcpy(header.data() + sizeof(VERSION) + sizeof(type), &contentLength,
              sizeof(contentLength));

  return header;
}

std::expected<HeaderParseResult, PacketError>
parseHeader(const std::string &packet) {
  if (packet.size() < HEADER_LENGTH_BYTES) {
    LOG_ERROR("Packet is too short with size of: ", packet.size());
    return std::unexpected(PacketError::InvalidLength);
  }
  if (!packet.starts_with(VERSION)) {
    LOG_ERROR("Invalid version of packet (", packet.substr(sizeof(VERSION)),
              ")");
    return std::unexpected(PacketError::InvalidVersion);
  }
  PacketType type = 0;
  std::memcpy(&type, packet.data() + sizeof(VERSION), sizeof(type));
  PacketContentLength length = 0;
  std::memcpy(&length, packet.data() + sizeof(VERSION) + sizeof(type),
              sizeof(length));
  return HeaderParseResult{.type = type, .contentLength = length};
}

void printPacket(const std::string &s) {

  std::string x;

  std::cout << "Packet version: ";

  PacketType type = 0;
  PacketContentLength length = 0;

  std::memcpy(&type, s.data() + sizeof(VERSION), sizeof(PacketType));
  std::memcpy(&length, s.data() + sizeof(VERSION) + sizeof(PacketType),
              sizeof(PacketContentLength));

  printBytes(s.substr(0, sizeof(VERSION)));
  std::cout << "Packet type: " << type << '\n';
  // bytes are reversed whne sent
  std::cout << "Packet content length: " << length << '\n';
  // bytes are reversed whne sent
  std::cout << "Packet content body (first 100 bytes): \n";

  std::ios_base::fmtflags f(std::cout.flags());

  for (int i = 0; i < s.substr(HEADER_LENGTH_BYTES).size() && i < 100; ++i) {
    std::cout << std::hex << std::setfill('0') << std::setw(2)
              << (int)s.substr(HEADER_LENGTH_BYTES)[i] << " ";
  }
  std::cout.flags(f);
  std::cout << std::endl;
}

} // namespace internal

} // namespace network
