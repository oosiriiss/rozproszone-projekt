#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <cstdint>
#include <expected>

#include "../game/Enemy.hpp"
#include "../game/Level.hpp"
#include "../game/Player.hpp"

namespace network {

namespace internal {

// Byte sequence used to separate messages in TCP stream
constexpr char VERSION[4] = {0, 0, 0, 1};
constexpr char SEPARATOR[4] = {0x0F, 0x00, 0x01, 0x0A};
typedef uint16_t PacketType;
typedef uint16_t PacketContentLength;
typedef uint64_t Timestamp;

struct PacketHeader {
  PacketType type;
  PacketContentLength contentLength;
  Timestamp timestamp;
};

template <typename T> struct PacketWrapper {
  PacketHeader header;
  T body;
};

template <typename T> struct PacketCompare {
  bool operator()(const PacketWrapper<T> &a, const PacketWrapper<T> &b) {
    return a.header.timestamp > b.header.timestamp;
  }
};

enum class PacketError { InvalidVersion, InvalidType, InvalidLength };

constexpr int32_t HEADER_LENGTH_BYTES = sizeof(VERSION) + sizeof(PacketType) +
                                        sizeof(PacketContentLength) +
                                        sizeof(Timestamp);

template <typename T>
constexpr inline void appendBytes(std::string &dest, const T &obj);
// returns the number of bytes read
template <typename T>
constexpr inline size_t readBytes(std::string_view src, T &outObj,
                                  size_t offset = 0);

template <typename T>
constexpr inline std::string serialize(const std::vector<T> &a);

template <typename T>
constexpr inline size_t deserialize(std::string_view s, std::vector<T> &out);

void printPacket(const std::string &s);

std::string createPacketHeader(PacketType type,
                               PacketContentLength contentLength);

std::expected<PacketHeader, PacketError> parseHeader(const std::string &packet);

} // namespace internal

class Serializable {
  virtual std::string serialize() const = 0;
  virtual void deserialize(std::string_view body) = 0;
};

struct PlayerDisconnectedResponse {
  int32_t playerID;
};

struct JoinLobbyRequest {};
struct LobbyReadyRequst {
  bool isReady;
};

struct LobbyReadyResponse {
  int32_t playerID;
  bool isReady;
};
struct JoinLobbyResponse : public Serializable {
  JoinLobbyResponse() = default;
  JoinLobbyResponse(std::unordered_map<int32_t, bool> lobbyPlayers);
  std::unordered_map<int32_t, bool> lobbyPlayers;

  std::string serialize() const override;
  void deserialize(std::string_view body) override;
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

struct EnemyUpdateResponse : public Serializable {
  EnemyUpdateResponse() = default;
  EnemyUpdateResponse(std::vector<Enemy::DTO> enemies);
  std::vector<Enemy::DTO> enemies;
  std::string serialize() const override;
  void deserialize(std::string_view body) override;
};

struct FireballShotRequest {
  int32_t playerID;
  Fireball::DTO fireball;
};

struct UpdateFireballsResponse : public Serializable {

  UpdateFireballsResponse() = default;
  UpdateFireballsResponse(std::vector<Fireball::DTO> fireballs);

  std::vector<Fireball::DTO> fireballs;

  std::string serialize() const override;
  void deserialize(std::string_view body) override;
};

struct BaseHitResponse {
  int newHealth;
};

struct GameOverResponse {
  int isWon;
};

void printBytes(std::string_view s);

// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyRequest, LobbyReadyRequst, GameReadyRequest,
                     PlayerMoveRequest, FireballShotRequest>
    ClientPacket;
// TODO :: Change this to inheritance?
typedef std::variant<PlayerDisconnectedResponse, JoinLobbyResponse,
                     LobbyReadyResponse, StartGameResponse, GameReadyResponse,
                     PlayerMoveResponse, EnemyUpdateResponse,
                     UpdateFireballsResponse, BaseHitResponse, GameOverResponse>
    ServerPacket;

template <class PACKET> std::string encodePacket(const PACKET &packet);

template <class VARIANT>
std::optional<internal::PacketWrapper<VARIANT>>
decodePacket(const std::string &packet);

} // namespace network

#include "packet_impl.hpp"
