#pragma once
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

#include "../game/Level.hpp"
#include "../game/Player.hpp"

namespace network {
struct JoinLobbyRequest {};
struct JoinLobbyResponse {
  uint8_t connectedPlayersCount;
};

struct StartGameResponse {};

struct GameReadyRequest {};

struct GameReadyResponse {
  int32_t playerID;
  Player p1;
  Player p2;
  Level::MapData map;

  // TODO :: IMPLEMENT CUSTOM SERIALIZATION FOR THIS TO NOT TAKE MILION BYETS
  // SPACES
  // TODO :: IMPLEMENT CUSTOM SERIALIZATION FOR THIS TO NOT TAKE MILION BYETS
  // SPACES
  // TODO :: IMPLEMENT CUSTOM SERIALIZATION FOR THIS TO NOT TAKE MILION BYETS
  // SPACES
  // TODO :: IMPLEMENT CUSTOM SERIALIZATION FOR THIS TO NOT TAKE MILION BYETS
  // SPACES
  // TODO :: IMPLEMENT CUSTOM SERIALIZATION FOR THIS TO NOT TAKE MILION BYETS
  // SPACES

  // std::string serialize() const;
  // void deserialize(std::string_view body);
};

struct PlayerMoveRequest {
  Direction direction;
};

struct PlayerMoveResponse {
  int32_t playerID;
  sf::Vector2f newPos;
};

// struct FullPlayerSyncRequest {
//   uint8_t playerID;
//   sf::Vector2f playerPos;
// };

// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyRequest, GameReadyRequest, PlayerMoveRequest>
    ClientPacket;
// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyResponse, StartGameResponse, GameReadyResponse,
                     PlayerMoveResponse>
    ServerPacket;

template <class PACKET> std::string encodePacket(const PACKET &packet);

template <class VARIANT>
std::optional<VARIANT> decodePacket(const std::string &packet);

} // namespace network

#include "packet_impl.hpp"
