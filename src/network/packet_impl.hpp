#pragma once
#include <cstdint>
#include <cstring>
#include <expected>
#include <stdexcept>
#include <string>
#include <variant>

#include "../debug.hpp"
#include "../logging.hpp"

namespace network {

void printBytes(const std::string &s);

namespace internal {

// Byte sequence used to separate messages in TCP stream
constexpr char VERSION[4] = {0, 0, 0, 1};
constexpr char SEPARATOR[4] = {0x0F, 0x00, 0x01, 0x0A};
typedef uint16_t PacketType;
typedef uint16_t PacketContentLength;

struct HeaderParseResult {
  PacketType type;
  PacketContentLength contentLength;
};

enum class PacketError { InvalidVersion, InvalidType, InvalidLength };

constexpr int32_t HEADER_LENGTH_BYTES =
    sizeof(VERSION) + sizeof(PacketType) + sizeof(PacketContentLength);

std::string createPacketHeader(PacketType type,
                               PacketContentLength contentLength);

void printPacket(const std::string &s);

template <typename T>
concept HasCustomSerialization = requires(T t) {
  { t.serialize() } -> std::same_as<std::string>;
};
template <typename T>
concept HasCustomDeserialization = requires(T t) {
  { t.deserialize(std::declval<std::string>()) } -> std::same_as<void>;
};

template <typename VARIANT>
constexpr std::string serializePacket(const VARIANT &packet) {

  std::string packetBody;
  std::visit(
      [&packetBody](auto &&p) {
        using T = std::decay_t<decltype(p)>;
        if constexpr (HasCustomSerialization<T>) {
          static_assert(
              HasCustomDeserialization<T>,
              "Has custom serialization but doesn't have deserialization");

          packetBody.append(p.serialize());
        } else {
          char buf[sizeof(T)] = {0};
          std::memcpy(buf, &p, sizeof(T));
          packetBody.append(buf, sizeof(T));
        }
      },
      packet);

  return packetBody;
}

template <class VARIANT, PacketType INDEX = 0>
VARIANT deserializePacket(PacketType index, const std::string &data) {
  if constexpr (INDEX < std::variant_size_v<VARIANT>) {
    if (index == INDEX) {
      using alt = std::variant_alternative_t<INDEX, VARIANT>;
      LOG_DEBUG("DATA_SIZE: ", data.size());
      LOG_DEBUG("altSize: ", sizeof(alt));
      alt obj;

      if constexpr (HasCustomDeserialization<alt>) {
        static_assert(
            HasCustomSerialization<alt>,
            "Has custom Deserialization but doesn't have Serialization");

        obj.serialize(data);
      } else {
        ASSERT(data.size() == sizeof(alt));
        std::memcpy(&obj, data.data(), sizeof(alt));
      }

      return VARIANT{std::move(obj)};
    } else {
      return deserializePacket<VARIANT, INDEX + 1>(index, data);
    }
  } else {
    throw std::out_of_range("Invalid variant index");
  }
}

template <typename VARIANT>
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

  if (type >= std::variant_size_v<VARIANT> || type < 0) {
    LOG_ERROR("Invalid packet type: ", type);
    return std::unexpected(PacketError::InvalidType);
  }
  PacketContentLength length = 0;

  std::memcpy(&length, packet.data() + sizeof(VERSION) + sizeof(type),
              sizeof(length));

  return HeaderParseResult{.type = type, .contentLength = length};
}
} // namespace internal

template <class PACKET> std::string encodePacket(const PACKET &packet) {
  std::string body = internal::serializePacket(packet);

  LOG_DEBUG("Body size bytes: ", body.size());

  std::string msg = internal::createPacketHeader(
      (internal::PacketType)packet.index(), body.size());

  LOG_DEBUG("Encoded Header bytes:");
  DEBUG_ONLY(printBytes(msg));

  LOG_DEBUG("Packet index: ", packet.index(), " body size: ", body.size());
  msg.append(body);
  msg.append(internal::SEPARATOR, sizeof(internal::SEPARATOR));

  DEBUG_ONLY(internal::printPacket(msg));
  LOG_DEBUG("Encoded message size (with separator): ", msg.size());

  return msg;
}
template <class VARIANT>
std::optional<VARIANT> decodePacket(const std::string &packet) {

  DEBUG_ONLY(internal::printPacket(packet));

  auto headerResult = internal::parseHeader<VARIANT>(packet);

  if (!headerResult) {
    LOG_ERROR("Packet header Couldn't be parsed");
    return std::nullopt;
  }

  LOG_DEBUG("Decoded header bytes:");

  DEBUG_ONLY(printBytes(packet.substr(0, internal::HEADER_LENGTH_BYTES)));

  internal::PacketType type = (*headerResult).type;
  internal::PacketContentLength length = (*headerResult).contentLength;

  LOG_DEBUG("Decoded contenetlength: ", length);

  const uint32_t bodysize = packet.size() - internal::HEADER_LENGTH_BYTES;

  LOG_DEBUG("Packet size:", packet.size(), " type: ", type, " length ", length);

  ASSERT(bodysize == length &&
         "Packet content length is equal to specified in header");

  const std::string &body =
      packet.substr(internal::HEADER_LENGTH_BYTES, std::string::npos);

  LOG_DEBUG("Body size: ", body.size());

  auto decoded = internal::deserializePacket<VARIANT>(type, body);

  LOG_DEBUG("Packet decoded");

  return decoded;
}

} // namespace network
