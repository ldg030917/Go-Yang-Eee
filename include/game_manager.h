// game_manager.h
#pragma once
#include <vector>
#include <unordered_map>
#include <windows.h>
#include <gdiplus.h>
#include "fishing_rod.h"
#include "cat.h"
#include "food.h"

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
    HWND gRodWnd = NULL;
    bool fishingRodActive;
    std::vector<Cat*> cats;
    std::vector<Food> foods;

    void toggleFishingRod() {
        if (fishingRodActive) {
            delete fishingRod;
            fishingRod = nullptr;
            fishingRodActive = false;
            if (gRodWnd) ShowWindow(gRodWnd, SW_HIDE);
            printf("낚싯대 OFF\n");
        } else {
            fishingRod = new FishingRod(12, 20.0f);
            fishingRodActive = true;
            if (gRodWnd) {
                ShowWindow(gRodWnd, SW_SHOW);
                SetWindowPos(gRodWnd, HWND_TOPMOST, 0, 0, 0, 0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            printf("낚싯대 ON\n");
        }
    }
    
private:
    std::unordered_map<int, CatAsset> catAssets;
    
    GameManager() : fishingRod(nullptr), fishingRodActive(false) {}
    ~GameManager() = default;
};
