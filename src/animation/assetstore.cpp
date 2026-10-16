#include "assetstore.hpp"

#include <filesystem>
#include <iostream>
#include <utility>

AssetStore::AssetStore()
  : texture_store()
  , sprite_sheets()
{
}

bool
AssetStore::init(SDL_Renderer* renderer)
{
  if (!renderer) {
    std::cerr << "Failed to load assets: renderer is not initialized\n";
    return false;
  }

  try {
    for (const auto& entry : std::filesystem::directory_iterator("assets/")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".json") {
        continue;
      }

      const auto name = entry.path().stem().string();
      if (sprite_sheets.contains(name)) {
        continue;
      }

      SpriteSheet sheet(entry.path().string().c_str());
      sheet.load();

      auto texture_path = entry.path();
      texture_path.replace_extension(".png");
      const auto result =
        texture_store.load_from_file(texture_path.string().c_str(), renderer);
      if (!result.success) {
        std::cerr << "Failed to load texture " << texture_path << ": "
                  << SDL_GetError() << '\n';
        return false;
      }

      sprite_sheets.emplace(name, std::move(sheet));
    }
  } catch (const std::exception& e) {
    std::cerr << "Failed to load assets: " << e.what() << '\n';
    return false;
  }
  return true;
}

SpriteSheet*
AssetStore::get_sheet(const char* name)
{
  const auto sheet = sprite_sheets.find(name);
  return sheet != sprite_sheets.end() ? &sheet->second : nullptr;
}
