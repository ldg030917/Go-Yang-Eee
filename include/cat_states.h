// cat_states.h
#pragma once
#include "cat_state.h"
#include "cat.h"

// --- 기본 상태 (가만히 있기, 걷기) ---
class IdleState : public CatState {
private:
    int stateTimer; // 행동 유지 시간, 지금은 timeToThink로 같이 묶어 사용 중

public:
    void Enter(Cat* cat) override;
    void Update(Cat* cat) override;
    void Exit(Cat* cat) override {}
};

// --- 붙잡힌 상태 (드래그) ---
class GrabbedState : public CatState {
    void Enter(Cat* cat) override;
    void Update(Cat* cat) override;
    void Exit(Cat* cat) override {}
};

// --- 수면 상태 (체력 소모 시) ---
class SleepState : public CatState {
    void Enter(Cat* cat) override {
        cat->SetAction(SLEEP);
        cat->targetSpeedX = 0;
    }
    void Update(Cat* cat) override;
    void Exit(Cat* cat) override {}
};

// --- 사냥 상태 (장난감, 음식을 향해 점프) ---
class HuntState : public CatState {
private:
    bool isJump;
    int catchTimer;
public:
    void Enter(Cat* cat) override {
        cat->SetAction(JUMP);
        isJump = false;
        catchTimer = 0;
        cat->targetSpeedX = 0;
    }
    void Update(Cat* cat) override;
    void Exit(Cat* cat) override {}
};