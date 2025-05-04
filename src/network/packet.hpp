#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <cstdint>
#include <expected>

#include "../game/Enemy.hpp"
#include "../game/Level.hpp"
#include "../game/Player.hpp"

namespace network {

class Serializable {
  virtual std::string serialize() const = 0;
  virtual void deserialize(std::string_view body) = 0;
};

struct JoinLobbyRequest {};
struct JoinLobbyResponse {
  uint8_t connectedPlayersCount;
};

struct StartGameResponse {};
struct GameReadyRequest {};

struct GameReadyResponse {
  int32_t thisPlayerID;
  sf::Vector2f thisPlayerPos;
  int32_t otherID;
  sf::Vector2f otherPlayerPos;

  // TODO ::  Change this to vector and add customserialzation for this not to
  // take millin bytes
  Level::MapData map;

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

struct EnemyUpdateResponse : Serializable {
  std::vector<sf::Vector2f> enemyPos;

  std::string serialize() const override;
  void deserialize(std::string_view body) override;
};

void printBytes(const std::string &s);

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
