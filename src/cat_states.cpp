// cat_states.cpp
#include "cat_states.h"
#include "cat.h"
#include <iostream>

using namespace std;

void IdleState::Enter(Cat* cat) {
    // 상태 진입 시 초기 행동 설정
    cat->SetAction(IDLE);
    cat->targetSpeedX = 0;
}

void IdleState::Update(Cat* cat) {
    // 1. 상태 전환 조건 체크 (체력)
    if (cat->GetHealth() <= 0) {
        cat->ChangeState(new SleepState());
        return;
    }

    // 2. 물리 및 이동 로직 (기존 Update의 else 부분)
    // 중력 및 바닥/벽 충돌 로직은 Cat::Update에 남겨두거나 
    // 모든 상태에서 공통으로 쓰인다면 별도 함수로 뺍니다.
    std::cout << cat->timeToThink << std::endl;
    cout << "health: " << cat->GetHealth() << endl;
    cat->ApplyPhysics(); // 중력 처리
    // 3. 행동 결정 (기존 Think 로직)
    if (cat->timeToThink <= 0) {
        // int idleW = cat->lazy * 2;
        // int moveW = cat->energy * 2;
        // int sleepW = (100 - cat->energy); // 자연스럽게 졸려짐
        // int cleanW = cat->friendliness;

        // int totalW = idleW + moveW + sleepW + cleanW;
        // int choice = rand() % totalW;

        // if (choice < idleW) {
        //     cat->SetAction((rand() % 2 == 0) ? IDLE : IDLE2);
        //     cat->targetSpeedX = 0;
        //     cat->timeToThink = 30 + cat->lazy;
        // }
        // else if (choice < idleW + moveW) {
        //     cat->SetAction((rand() % 2 == 0) ? MOVE : MOVE2);
        //     cat->targetSpeedX = (rand() % 2 == 0) ? -MOVE_SPEED : MOVE_SPEED;
        //     cat->isLookingRight = (cat->targetSpeedX > 0);
        //     cat->timeToThink = 10 + rand() % (100 - cat->energy);
        // }
        // else if (choice < idleW + moveW + sleepW) {
        //     cat->ChangeState(new SleepState()); // 바로 잠자기 상태로 전환
        //     return;
        // }
        // else {
        //     cat->SetAction((rand() % 2 == 0) ? CLEAN : CLEAN2);
        //     cat->targetSpeedX = 0;
        //     cat->timeToThink = 20 + rand() % 40;
        // }
        cat->SetAction((rand() % 2 == 0) ? IDLE : MOVE);
        cat->timeToThink = 20;
    }

    cat->timeToThink -= 1;

    // 4. 체력 조금씩 소모 (활동에 따라)
    cat->SetHealth(cat->GetHealth() - 1); 
}

void GrabbedState::Enter(Cat* cat) {
    cat->SetAction(GRABBED);
    cat->targetSpeedX = 0;
}

void GrabbedState::Update(Cat* cat) {
    // 마우스 위치 계산 및 행잉 물리 로직
    POINT pt; GetCursorPos(&pt);
    float mouseVelX = (float)(pt.x - cat->physicsLastX);
    cat->physicsLastX = pt.x;

    // 흔들림 계산 (기존 코드의 물리 로직)
    cat->swingSpeed += mouseVelX * 0.03f;
    cat->swingSpeed -= cat->angle * 0.05f;
    cat->swingSpeed *= 0.95f;
    cat->angle += cat->swingSpeed;

    // 위치 갱신
    cat->posX = pt.x - NECK_OFFSET_X;
    cat->posY = pt.y - NECK_OFFSET_Y;

    // 던지기 속도 기록 (나중에 UP일 때 사용)
    cat->throwSpeedX = mouseVelX;
    cat->throwSpeedY = 0; // 필요시 Y축도 계산

    cat->lastCursorX = pt.x;
    cat->lastCursorY = pt.y;
    cat->physicsLastX = pt.x; // ★ [추가] 물리용 초기화 (이거 안하면 잡는 순간 미친듯이 돔)
    SetWindowPos(cat->hwnd, NULL, cat->posX, cat->posY, 0, 0, 
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SleepState::Update(Cat* cat) {
    //cat->targetSpeedX = 0;
    // 잠을 자서 체력을 전부 채우면, 기상
    if (cat->GetHealth() == cat->maxHealth) {
        cat->ChangeState(new IdleState());
        return;
    }

    cout << "health: " << cat->GetHealth() << endl;

    // 체력 회복
    cat->SetHealth(cat->GetHealth() + 1);
}