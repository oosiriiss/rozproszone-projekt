#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace network {
// Byte sequence used to separate messages in TCP stream
constexpr char SEPARATOR[4] = {0x0A, 0x0A, 0x0A, 0x0A};
constexpr char VERSION[4] = {0, 0, 0, 1};

struct JoinLobbyRequest {};
struct JoinLobbyResponse {
  uint8_t connectedPlayersCount;
};

struct StartGameResponse {
  uint8_t id;
};

struct PlayerMovedRequest {
  uint8_t playerId;
  float x;
  float y;
};

struct PlayerMovedResponse {
  uint8_t playerId;
  float x;
  float y;
};

enum class PacketError { InvalidHeader };

enum class PacketType : uint16_t {
  JoinLobby = 0,
  StartGame = 1,
  PlayerMoved = 2
};

// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyRequest, PlayerMovedRequest> ClientPacket;
// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyResponse, StartGameResponse, PlayerMovedResponse>
    ServerPacket;

std::string encodePacket(const ClientPacket &packet);
std::string encodePacket(const ServerPacket &packet);
std::optional<ClientPacket> decodeClientPacket(const std::string &packet);
std::optional<ServerPacket> decodeServerPacket(const std::string &packet);
} // namespace network
