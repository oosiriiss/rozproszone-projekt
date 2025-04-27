#pragma once
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <string>
#include <variant>

#include "../game/Level.hpp"

namespace network {
struct JoinLobbyRequest {};
struct JoinLobbyResponse {
  uint8_t connectedPlayersCount;
};

struct StartGameResponse {};

struct GameReadyRequest {};

struct GameReadyResponse {
  uint8_t playerID;
  Level::MapData map;

  // std::string serialize() const;
  // void deserialize(std::string_view body);
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

// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyRequest, PlayerMovedRequest, GameReadyRequest>
    ClientPacket;
// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyResponse, StartGameResponse, PlayerMovedResponse,
                     GameReadyResponse>
    ServerPacket;

template <class PACKET> std::string encodePacket(const PACKET &packet);

template <class VARIANT>
std::optional<VARIANT> decodePacket(const std::string &packet);

} // namespace network

#include "packet_impl.hpp"
