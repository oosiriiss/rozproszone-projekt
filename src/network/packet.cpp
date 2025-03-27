#include "packet.hpp"
#include "../assert.hpp"
#include "../logging.hpp"
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace network {

constexpr std::optional<PacketType> getType(const uint16_t type);

constexpr int32_t HEADER_LENGTH_BYTES = sizeof(VERSION) + sizeof(PacketType);

void assertServerBodySize(PacketType type, uint32_t bodysize) {

  switch (type) {
  case PacketType::AcquireID:
    break;
  case PacketType::PlayerMoved:
    assertEqual(bodysize, sizeof(PlayerMovedResponse),
                "Player moved response size");
    break;
  }
}

void assertClientBodySize(PacketType type, uint32_t bodysize) {

  switch (type) {
  case PacketType::AcquireID:
    // AcquireIdRequest has no body
    break;
  case PacketType::PlayerMoved:
    assertEqual(bodysize, sizeof(PlayerMovedRequest));
    break;
  }
}

// Creates the header of the packet
// like assings packet type etc.
std::string createPacketHeader(PacketType type) {

  static_assert(sizeof((VERSION)) == 4, "Version length != 4");
  static_assert(sizeof(std::to_underlying(type)) == 2,
                "Size of PacketType underlying type != 2");

  std::string header;
  header.reserve(HEADER_LENGTH_BYTES);

  header.append(VERSION, sizeof(VERSION));

  uint16_t typeID = std::to_underlying(type);

  header.push_back((uint8_t)(typeID >> 8));
  header.push_back((uint8_t)typeID);

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
  case static_cast<int>(PacketType::PlayerMoved):
    return PacketType::PlayerMoved;
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
        if constexpr (std::is_same_v<T, PlayerMovedRequest>) {
          return PacketType::PlayerMoved;
        }

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
        if constexpr (std::is_same_v<T, PlayerMovedResponse>) {
          return PacketType::PlayerMoved;
        }

        LOG_ERROR("");
        throw std::invalid_argument("PACKET TYPE CONVERSION NOT IMPLEMENTED");
      },
      packet);
}

std::string encodePacket(const ClientPacket &packet) {
  std::string message = createPacketHeader(getType(packet));
  // Todo :: Create one buffer for al lvariants?

  if (const auto *acquireID = std::get_if<AcquireIDRequest>(&packet)) {
    //
  } else if (const auto *playerMoved =
                 std::get_if<PlayerMovedRequest>(&packet)) {
    char buf[sizeof(PlayerMovedRequest)] = {0};
    std::memcpy(buf, playerMoved, sizeof(PlayerMovedRequest));
    message.append(buf, sizeof(PlayerMovedRequest));
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

  const uint32_t bodysize = packet.size() - HEADER_LENGTH_BYTES;
  assertClientBodySize(type, bodysize);

  const std::string_view &body = packet.data() + HEADER_LENGTH_BYTES;

  switch (type) {
  case PacketType::AcquireID:
    return AcquireIDRequest{};
  case PacketType::PlayerMoved:
    PlayerMovedRequest p;
    std::memcpy(&p, body.data(), sizeof(PlayerMovedRequest));
    return p;

  default:
    LOG_ERROR("Unknown player packet id: ", std::to_underlying(type));
    throw std::invalid_argument("Invalid client packet");
    break;
  }
  throw std::runtime_error("Unreachable");
}

std::string encodePacket(const ServerPacket &packet) {

  std::string message = createPacketHeader(getType(packet));

  if (const auto *acquireID = std::get_if<AcquireIDResponse>(&packet)) {
    char buf[sizeof(AcquireIDResponse)] = {0};
    std::memcpy(buf, acquireID, sizeof(AcquireIDResponse));
    message.append(buf, sizeof(AcquireIDResponse));

    LOG_INFO("Size of id response str no head",
             message.size() - HEADER_LENGTH_BYTES);

  } else if (const auto *playerMoved =
                 std::get_if<PlayerMovedResponse>(&packet)) {
    char buf[sizeof(PlayerMovedResponse)] = {0};
    std::memcpy(buf, playerMoved, sizeof(PlayerMovedResponse));
    message.append(buf, sizeof(PlayerMovedResponse));
  } else {
    assertEqual(true, false, "Packet type not implemented");
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

  const uint32_t bodysize = packet.size() - HEADER_LENGTH_BYTES;
  // Make sure the packet has enough data
  assertServerBodySize(type, bodysize);

  const std::string_view &body = packet.data() + HEADER_LENGTH_BYTES;

  switch (type) {
  case PacketType::AcquireID: {
    AcquireIDResponse i;
    std::memcpy(&i, body.data(), sizeof(AcquireIDResponse));
    return i;
  }
  case PacketType::PlayerMoved:
    PlayerMovedResponse p;
    std::memcpy(&p, body.data(), sizeof(PlayerMovedResponse));
    return p;
    break;
  }

  throw std::runtime_error("Unreachable");
}
} // namespace network
