#pragma once
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "../debug.hpp"
#include "../logging.hpp"

#include "packet.hpp"

namespace network {

namespace internal {

template <typename T>
constexpr inline void appendBytes(std::string &dest, const T &obj) {
  dest.append((char *)&obj, sizeof(obj));
}

template <typename T>
constexpr inline size_t readBytes(std::string_view src, T &obj, size_t offset) {
  ASSERT(src.size() > offset && "Offset is smaller than string");
  ASSERT(sizeof(T) > (src.size() - offset) && "Enough bytes to read");

  std::memcpy(&obj, src.data() + offset, sizeof(T));

  return sizeof(T);
}

template <typename T>
constexpr inline std::string serialize(const std::vector<T> &a) {

  std::string b;
  appendBytes(b, a.size());
  for (const auto &x : a) {
    appendBytes(b, x);
  }

  return b;
}

template <typename T>
constexpr inline std::vector<T> deserialize(std::string_view s) {

  size_t offset = 0;

  size_t count = 0;
  offset += readBytes(s, count);

  std::vector<T> ts = std::vector<T>(count);

  ASSERT(s.size() - offset >= count * sizeof(T) && "Enough elements to read");

  for (int i = 0; i < count; ++i) {
    T x;
    offset += readBytes(s, x, offset);
    ts.push_back(x);
  }
  return ts;
}

template <typename T>
concept HasCustomSerialization =
    requires(T t) { std::is_base_of_v<Serializable, T>(); };

template <typename VARIANT>
constexpr std::string serializePacket(const VARIANT &packet) {

  std::string packetBody;
  std::visit(
      [&packetBody](auto &&p) {
        using T = std::decay_t<decltype(p)>;
        if constexpr (HasCustomSerialization<T>) {

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

      if constexpr (HasCustomSerialization<alt>) {
        obj.deserialize(data);
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

  auto headerResult = internal::parseHeader(packet);

  if (!headerResult) {
    LOG_ERROR("Packet header Couldn't be parsed");
    return std::nullopt;
  }

  if (headerResult->type >= std::variant_size_v<VARIANT> ||
      headerResult->type < 0) {
    LOG_ERROR("Invalid packet type: ", headerResult->type);
    // return std::unexpected(PacketError::InvalidType);
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
