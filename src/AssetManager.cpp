#include "AssetManager.hpp"
#include "logging.hpp"
#include <filesystem>
#include <string_view>
#include <variant>

AssetManager::AssetManager(std::string_view assetsDirectory)
    : ASSETS_DIR(assetsDirectory), m_assets() {

  if (!std::filesystem::exists(assetsDirectory)) {
    LOG_ERROR("Assets directory: '", assetsDirectory, "' doesn't exist");
    exit(-1);
  }
}

AssetManager::~AssetManager() {
  LOG_ERROR("ASSET MANAGER DESTRUCTOR NOT IMPLEMENTED");
  exit(-1);
}

void AssetManager::loadFont(const std::string_view path) {

  sf::Font font;
  if (!font.openFromFile(std::filesystem::path(ASSETS_DIR).append(path))) {
    LOG_ERROR("Font: ", path, " doesn't exist");
    exit(-1);
  }

  m_assets[path] = FontAsset{.font = font};
}

const FontAsset &AssetManager::getFont(const std::string_view path) const {

  auto it = m_assets.find(path);

  if (it == m_assets.end()) {
    LOG_ERROR("TERMINATING: Couldn't find font with path: ", path);
    exit(-1);
  }

  auto asset = it->second;

  if (!std::holds_alternative<FontAsset>(asset)) {
    LOG_ERROR("path: ", path, " doesn't hold hold a font");
    exit(-1);
  }

  return std::get<FontAsset>(it->second);
}
