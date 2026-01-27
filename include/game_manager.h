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
    bool fishingRodActive;
    std::vector<Cat*> cats;
    std::vector<Food> foods;

    void toggle_fishing_rod();
    
private:
    GameManager() : fishingRod(nullptr), fishingRodActive(false) {}
    ~GameManager() = default;
};
