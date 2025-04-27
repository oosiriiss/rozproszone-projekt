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
  sf::Vector2f playerPos;
  Level::MapData map;

  // std::string serialize() const;
  // void deserialize(std::string_view body);
};

struct FullPlayerSyncRequest {
  uint8_t playerID;
  sf::Vector2f playerPos;
};

struct FullPlayerSyncResponse {
  uint8_t playerID;
  sf::Vector2f playerPos;
};

// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyRequest, GameReadyRequest, FullPlayerSyncRequest>
    ClientPacket;
// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyResponse, StartGameResponse, GameReadyResponse,
                     FullPlayerSyncResponse>
    ServerPacket;

template <class PACKET> std::string encodePacket(const PACKET &packet);

template <class VARIANT>
std::optional<VARIANT> decodePacket(const std::string &packet);

} // namespace network

#include "packet_impl.hpp"
