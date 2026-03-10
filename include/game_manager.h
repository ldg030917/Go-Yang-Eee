// game_manager.h
#pragma once
#include <vector>
#include <unordered_map>
#include <gdiplus.h>
#include "fishing_rod.h"
#include "cat.h"

struct CatAsset {
    Gdiplus::Image* image = nullptr;
    IStream* stream = nullptr;
};

class GameManager {
public:
    static GameManager& get() {
        static GameManager instance;
        return instance;
    }

    // 에셋 관리 함수
    void LoadAllAssets(HINSTANCE hInstance);
    void ReleaseAllAssets();

    // 이미지 반환용 함수
    Gdiplus::Image* GetCatImage(int catType) {
        if (catAssets.find(catType) != catAssets.end()) {
            return catAssets[catType].image;
        }
        return nullptr;
    }

    FishingRod* fishingRod;
    bool fishingRodActive;
    std::vector<Cat*> cats;

    void toggle_fishing_rod();
    
private:
    std::unordered_map<int, CatAsset> catAssets;

    GameManager() : fishingRod(nullptr), fishingRodActive(false) {}
    ~GameManager() = default;
};
