#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace network {
// Byte sequence used to separate messages in TCP stream
constexpr char SEPARATOR[4] = {0x0A, 0x0A, 0x0A, 0x0A};
constexpr char VERSION[4] = {0, 0, 0, 1};

struct AcquireIDRequest {};
struct AcquireIDResponse {
  uint8_t id;
};

enum class PacketError { InvalidHeader };

enum class PacketType : uint16_t {
  AcquireID = 0,

};

typedef std::variant<AcquireIDRequest> ClientPacket;

typedef std::variant<AcquireIDResponse> ServerPacket;

std::string encodePacket(const ClientPacket &packet);
std::optional<ClientPacket> decodeClientPacket(const std::string &packet);

std::string encodePacket(const ServerPacket &packet);
std::optional<ServerPacket> decodeServerPacket(const std::string &packet);
} // namespace network
