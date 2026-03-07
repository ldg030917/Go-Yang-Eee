// game_manager.h
#pragma once
#include <vector>
#include "fishing_rod.h"
#include "cat.h"

class GameManager {
public:
    static GameManager& get() {
        static GameManager instance;
        return instance;
    }

    FishingRod* fishingRod;
    HWND gRodWnd = NULL;
    bool fishingRodActive;
    std::vector<Cat*> cats;
    std::vector<Food> foods;

    void toggle_fishing_rod() {
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
    GameManager() : fishingRod(nullptr), fishingRodActive(false) {}
    ~GameManager() = default;
};
