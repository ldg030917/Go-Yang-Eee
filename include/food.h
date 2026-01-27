// food.h
#pragma once
#include "config.h"
#include "cat.h"

struct Food {
    float x, y;
    float vx = 0, vy = 0; // 중력용 속도
    int eatenState = 0; // 먹힌 정도

    Cat* owner = nullptr;
};