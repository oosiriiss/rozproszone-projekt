#include "packet.hpp"
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <cstdint>
#include <expected>
#include <iomanip>
#include <string>

namespace network {

void printBytes(std::string_view s) {

  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::hex << std::setfill('0') << std::setw(2);
  for (char c : s) {
    std::cout << ((uint32_t)c) % 256 << " ";
  }
  std::cout.flags(f);
  std::cout << std::endl;
}

JoinLobbyResponse::JoinLobbyResponse(
    std::unordered_map<int32_t, bool> lobbyPlayers)
    : lobbyPlayers(lobbyPlayers) {}

std::string JoinLobbyResponse::serialize() const {
  std::string s;

  internal::appendBytes(s, this->lobbyPlayers.size());
  for (auto [p, r] : this->lobbyPlayers) {
    internal::appendBytes(s, p);
    internal::appendBytes(s, r);
  }

  return s;
}

void JoinLobbyResponse::deserialize(std::string_view body) {

  LOG_DEBUG("Deserializing JoinLobbyResponse body size: ", body.size());

  this->lobbyPlayers = std::unordered_map<int32_t, bool>();

  size_t size = 0;
  size_t offset = internal::readBytes(body, size);

  LOG_DEBUG("Read size: ", size);

  for (int i = 0; i < size; ++i) {

    int32_t p = -1;
    bool r = false;

    offset += internal::readBytes(body.substr(offset, std::string::npos), p);
    offset += internal::readBytes(body.substr(offset, std::string::npos), r);

    this->lobbyPlayers[p] = r;
  }
}

EnemyUpdateResponse::EnemyUpdateResponse(std::vector<Enemy::DTO> enemies)
    : enemies(enemies) {}
std::string EnemyUpdateResponse::serialize() const {
  std::string s = internal::serialize(this->enemies);
  return s;
}

void EnemyUpdateResponse::deserialize(const std::string_view body) {
  LOG_DEBUG("Deserializing EnemyUpdateResponse");
  LOG_DEBUG("Received packet: ");
  DEBUG_ONLY(printBytes(body));
  size_t offset = internal::deserialize(body, this->enemies);
  LOG_DEBUG("Received enemies: ", enemies.size());
}

UpdateFireballsResponse::UpdateFireballsResponse(
    std::vector<Fireball::DTO> fireballs)
    : fireballs(fireballs) {}

std::string UpdateFireballsResponse::serialize() const {
  return internal::serialize(this->fireballs);
}

void UpdateFireballsResponse::deserialize(std::string_view body) {
  LOG_DEBUG("Deserializing UpdateFireballsResponse");
  LOG_DEBUG("Body size: ", body.size());
  internal::deserialize(body, this->fireballs);

  LOG_DEBUG("Deserialized");
}

namespace internal {

// Creates the header of the packet
std::string createPacketHeader(PacketType type,
                               PacketContentLength contentLength) {

  static_assert(sizeof((VERSION)) == 4, "Version length == 4");

  std::string header = std::string(HEADER_LENGTH_BYTES, '-');
  LOG_DEBUG("Encoded header length", header.size());
  std::memcpy(header.data(), VERSION, sizeof(VERSION));
  size_t offset = sizeof(VERSION);
  std::memcpy(header.data() + offset, &type, sizeof(type));
  offset += sizeof(type);
  std::memcpy(header.data() + sizeof(VERSION) + sizeof(type), &contentLength,
              sizeof(contentLength));
  offset += sizeof(contentLength);

  const auto timestamp =
      std::chrono::system_clock::now().time_since_epoch().count();
  static_assert(sizeof(timestamp) == sizeof(Timestamp));
  std::memcpy(header.data() + offset, &timestamp, sizeof(Timestamp));

  return header;
}

std::expected<PacketHeader, PacketError>
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
  PacketContentLength length = 0;
  PacketType type = 0;
  Timestamp timestamp = 0;

  size_t offset = sizeof(VERSION);

  std::memcpy(&type, packet.data() + offset, sizeof(type));
  offset += sizeof(type);
  std::memcpy(&length, packet.data() + offset, sizeof(length));
  offset += sizeof(length);
  std::memcpy(&timestamp, packet.data() + offset, sizeof(timestamp));
  return PacketHeader{
      .type = type, .contentLength = length, .timestamp = timestamp};
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
