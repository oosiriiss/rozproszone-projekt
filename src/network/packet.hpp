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
  EnemyUpdateResponse() = default;
  EnemyUpdateResponse(std::vector<sf::Vector2f> positions);
  std::vector<sf::Vector2f> enemyPos;
  std::string serialize() const override;
  void deserialize(std::string_view body) override;
};

struct FireballShotRequest {
  int32_t playerID;
  sf::Vector2f pos;
  sf::Vector2f direction;
};

struct UpdateFireballsResponse : public Serializable {

  UpdateFireballsResponse() = default;
  UpdateFireballsResponse(std::vector<sf::Vector2f> positions,
                          std::vector<sf::Vector2f> directions);

  // Todo :: introduce some kind of dto
  std::vector<sf::Vector2f> positions;
  std::vector<sf::Vector2f> directions;

  std::string serialize() const override;
  void deserialize(std::string_view body) override;
};

void printBytes(std::string_view s);

// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyRequest, GameReadyRequest, PlayerMoveRequest,
                     FireballShotRequest>
    ClientPacket;
// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyResponse, StartGameResponse, GameReadyResponse,
                     PlayerMoveResponse, EnemyUpdateResponse,
                     UpdateFireballsResponse>
    ServerPacket;

template <class PACKET> std::string encodePacket(const PACKET &packet);

template <class VARIANT>
std::optional<VARIANT> decodePacket(const std::string &packet);

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

std::expected<HeaderParseResult, PacketError>
parseHeader(const std::string &packet);

} // namespace internal

} // namespace network

#include "packet_impl.hpp"
