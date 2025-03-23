#include "packet.hpp"
#include "../logging.hpp"
#include <cassert>
#include <cstdint>
#include <expected>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace network {
constexpr std::optional<PacketType> getType(const uint16_t type);

constexpr int32_t HEADER_LENGTH_BYTES = sizeof(VERSION) + sizeof(PacketType);

// Creates the header of the packet
// like assings packet type etc.
std::string createPacketHeader(PacketType type) {

  static_assert(sizeof((VERSION)) == 4, "Version length != 4");
  static_assert(sizeof(std::to_underlying(type)) == 2,
                "Size of PacketType underlying type != 2");

  std::string header;
  header.reserve(HEADER_LENGTH_BYTES);

  LOG_INFO("HEADER LEN BYTES: ", HEADER_LENGTH_BYTES);
  header.append(VERSION, sizeof(VERSION));
  LOG_INFO("Header len1: ", header.size());

  uint16_t typeID = std::to_underlying(type);

  header.push_back((uint8_t)(typeID >> 8));
  header.push_back((uint8_t)typeID);

  LOG_INFO("Header len2: ", header.size());

  return header;
}

std::expected<PacketType, PacketError> parseHeader(const std::string &packet) {

  if (packet.size() < HEADER_LENGTH_BYTES) {
    LOG_ERROR("Packet is too short");
    return std::unexpected(PacketError::InvalidHeader);
  }

  if (!packet.substr(0, sizeof(VERSION)).starts_with(VERSION)) {
    LOG_ERROR("Invalid version of packet");
    return std::unexpected(PacketError::InvalidHeader);
  }

  std::string typeSubstring =
      packet.substr(sizeof(VERSION), sizeof(PacketType));

  uint16_t x = (int)typeSubstring[0];
  x = x << 8;
  x += typeSubstring[1];

  auto typeResult = getType(x);

  if (!typeResult.has_value()) {
    LOG_ERROR("Invalid packet type");
    return std::unexpected(PacketError::InvalidHeader);
  }

  return *typeResult;
}

constexpr std::optional<PacketType> getType(const uint16_t type) {

  switch (type) {
  case static_cast<int>(PacketType::AcquireID):
    return PacketType::AcquireID;
  default:
    return std::nullopt;
  }
}

constexpr PacketType getType(const ClientPacket &packet) {

  return std::visit(
      [](const auto &p) -> PacketType {
        using T = std::decay_t<decltype(p)>;
        if constexpr (std::is_same_v<T, AcquireIDRequest>) {
          return PacketType::AcquireID;
        }

        LOG_ERROR("");
        throw std::invalid_argument("PACKET TYPE CONVERSION NOT IMPLEMENTED");
      },
      packet);
}

constexpr PacketType getType(const ServerPacket &packet) {

  return std::visit(
      [](const auto &p) -> PacketType {
        using T = std::decay_t<decltype(p)>;
        if constexpr (std::is_same_v<T, AcquireIDResponse>) {
          return PacketType::AcquireID;
        }

        LOG_ERROR("");
        throw std::invalid_argument("PACKET TYPE CONVERSION NOT IMPLEMENTED");
      },
      packet);
}

std::string encodePacket(const ClientPacket &packet) {
  std::string message = createPacketHeader(getType(packet));

  if (const auto *request = std::get_if<AcquireIDRequest>(&packet)) {
  }

  message.append(SEPARATOR);
  return message;
}
std::optional<ClientPacket> decodeClientPacket(const std::string &packet) {

  auto headerResult = parseHeader(packet);

  if (!headerResult) {
    LOG_ERROR("Packet header Couldn't be parsed");
    return std::nullopt;
  }

  PacketType type = *headerResult;
  uint32_t BODY_START = HEADER_LENGTH_BYTES;

  switch (type) {
  case PacketType::AcquireID:
    return AcquireIDRequest{};
  }

  throw std::runtime_error("Unreachable");
}

std::string encodePacket(const ServerPacket &packet) {
  std::string message = createPacketHeader(getType(packet));

  if (const auto *response = std::get_if<AcquireIDResponse>(&packet)) {
    message.push_back(response->id);
  }

  message.append(SEPARATOR);
  return message;
}
std::optional<ServerPacket> decodeServerPacket(const std::string &packet) {

  auto headerResult = parseHeader(packet);

  if (!headerResult) {
    LOG_ERROR("Packet peader Couldn't be parsed");
    return std::nullopt;
  }

  PacketType type = *headerResult;
  uint32_t BODY_START = HEADER_LENGTH_BYTES;

  switch (type) {
  case PacketType::AcquireID: {
    uint8_t id = packet[BODY_START];

    return AcquireIDResponse{.id = id};
  }
  }

  throw std::runtime_error("Unreachable");
}
} // namespace network
