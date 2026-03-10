#pragma once
#include "game_manager.h"
#include "utils.h"

void GameManager::LoadAllAssets(HINSTANCE hInstance) {
    std::vector<int> catTypes = { } //IDB_CAT_CHEESE, IDB_CAT_SIAM
    
    for (int type : catTypes) {
        CatAsset asset;
        asset.image = LoadImageFromResource(hInstance, type, &asset.stream);
        catAssets[type] = asset;
    }
}

void GameManager::ReleaseAllAssets() {
    for (auto& pair : catAssets) {
        if (pair.second.image) delete pair.second.image;
        if (pair.second.stream) pair.second.stream->Release();
    }
    catAssets.clear();
}