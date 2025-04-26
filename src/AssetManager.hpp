#pragma once

#include <SFML/Graphics/Font.hpp>
#include <string_view>
#include <variant>

struct FontAsset {
  sf::Font font;
};

struct AssetManager {

  AssetManager(std::string_view assetsDirectory);
  ~AssetManager();

  // Loads asasets within /asset direcotry

  void loadFont(const std::string_view path);
  const FontAsset &getFont(const std::string_view path) const;

  const std::string_view ASSETS_DIR;

private:

  typedef std::variant<FontAsset> Asset;

  std::unordered_map<std::string_view, Asset> m_assets;
};
