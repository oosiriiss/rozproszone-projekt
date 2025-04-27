#pragma once
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <string>
#include <variant>

namespace network {
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

// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyRequest, PlayerMovedRequest> ClientPacket;
// TODO :: Change this to inheritance?
typedef std::variant<JoinLobbyResponse, StartGameResponse, PlayerMovedResponse>
    ServerPacket;

template <class PACKET> std::string encodePacket(const PACKET &packet);

template <class VARIANT>
std::optional<VARIANT> decodePacket(const std::string &packet);

} // namespace network

#include "packet_impl.hpp"
