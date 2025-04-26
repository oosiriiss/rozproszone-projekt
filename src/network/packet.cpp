#include <csignal>
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "../assert.hpp"
#include "../logging.hpp"
#include "packet.hpp"

namespace network {

constexpr std::optional<PacketType> getType(const uint16_t type);

constexpr int32_t HEADER_LENGTH_BYTES = sizeof(VERSION) + sizeof(PacketType);

void assertServerBodySize(PacketType type, uint32_t bodysize) {
  switch (type) {
  case PacketType::PlayerMoved:
    ASSERT(sizeof(PlayerMovedResponse) == bodysize);
    break;
  case PacketType::JoinLobby:
    break;
  case PacketType::StartGame:
    ASSERT(sizeof(PlayerMovedResponse) == bodysize);
    break;
  }
}

void assertClientBodySize(PacketType type, uint32_t bodysize) {

  switch (type) {
  case PacketType::PlayerMoved:
    ASSERT(sizeof(PlayerMovedRequest) == bodysize);
    break;
  case PacketType::JoinLobby:
    break;
  case PacketType::StartGame:
    UNREACHABLE;
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
  case static_cast<uint16_t>(PacketType::AcquireID):
    return PacketType::AcquireID;
  case static_cast<uint16_t>(PacketType::PlayerMoved):
    return PacketType::PlayerMoved;
  case static_cast<uint16_t>(PacketType::JoinLobby):
    return PacketType::JoinLobby;
  case static_cast<uint16_t>(PacketType::StartGame):
    return PacketType::StartGame;
  }

  UNREACHABLE;
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

        if constexpr (std::is_same_v<T, JoinLobbyRequest>) {
          return PacketType::JoinLobby;
        }

        ASSERT(!"Packet server type conversion not implemented");
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
        if constexpr (std::is_same_v<T, JoinLobbyResponse>) {
          return PacketType::JoinLobby;
        }
        ASSERT(!"Packet server type conversion not implemented");
      },
      packet);
}

std::string encodePacket(const ClientPacket &packet) {
  std::string message = createPacketHeader(getType(packet));

  LOG_DEBUG("Encoded message before size: ", message.size());

  std::visit(
      [&message](auto &&p) {
        using T = std::decay_t<decltype(p)>;
        char buf[sizeof(T)] = {0};
        std::memcpy(buf, &p, sizeof(T));
        message.append(buf, sizeof(T));
      },
      packet);
  message.append(SEPARATOR, sizeof(SEPARATOR));

  LOG_DEBUG("Encoded message size: ", message.size());

  return message;
}

std::string encodePacket(const ServerPacket &packet) {

  std::string message = createPacketHeader(getType(packet));
  LOG_DEBUG("Encoded message before size: ", message.size());

  std::visit(
      [&message](auto &&p) {
        using T = std::decay_t<decltype(p)>;
        char buf[sizeof(T)] = {0};
        std::memcpy(buf, &p, sizeof(T));
        message.append(buf, sizeof(T));
      },
      packet);

  message.append(SEPARATOR, sizeof(SEPARATOR));
  LOG_DEBUG("Encoded message size: ", message.size());
  return message;
}

std::optional<ClientPacket> decodeClientPacket(const std::string &packet) {

  LOG_DEBUG("Decoding client packet");
  auto headerResult = parseHeader(packet);

  if (!headerResult) {
    LOG_ERROR("Packet header Couldn't be parsed");
    return std::nullopt;
  }

  PacketType type = *headerResult;

  const uint32_t bodysize = packet.size() - HEADER_LENGTH_BYTES;
  assertClientBodySize(type, bodysize);

  const std::string_view &body = packet.data() + HEADER_LENGTH_BYTES;

  LOG_DEBUG("Decoded");
  switch (type) {
  case PacketType::AcquireID:
    return AcquireIDRequest{};
  case PacketType::PlayerMoved:
    PlayerMovedRequest p;
    std::memcpy(&p, body.data(), sizeof(PlayerMovedRequest));
    return p;
  case PacketType::JoinLobby:
    JoinLobbyRequest r;
    std::memcpy(&r, body.data(), sizeof(JoinLobbyRequest));
    return r;
  }
  throw std::runtime_error("Unreachable");
}

std::optional<ServerPacket> decodeServerPacket(const std::string &packet) {

  LOG_DEBUG("decoding server packet");
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

  LOG_DEBUG("Decoded");
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
  case PacketType::JoinLobby:
    JoinLobbyResponse r;
    std::memcpy(&r, body.data(), sizeof(JoinLobbyResponse));
    return r;
  }

  throw std::runtime_error("Unreachable");
}
} // namespace network
