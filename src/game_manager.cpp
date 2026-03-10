#include "game_manager.h"
#include "utils.h"
#include "resource.h"

void GameManager::LoadAllAssets(HINSTANCE hInstance) {
    for (int i = IDB_CAT_START; i <= IDB_CAT_END; i++) {
        CatAsset asset;
        asset.image = LoadImageFromResource(hInstance, i, &asset.stream);
        if (asset.image != nullptr) {
            catAssets[i] = asset;
        }
    }
}

void GameManager::ReleaseAllAssets() {
    for (auto& pair : catAssets) {
        if (pair.second.image) delete pair.second.image;
        if (pair.second.stream) pair.second.stream->Release();
    }
    catAssets.clear();
}