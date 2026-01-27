// cat_states.cpp
#include "cat_states.h"
#include "cat.h"
#include <iostream>
#include "game_manager.h"

using namespace std;

void IdleState::Enter(Cat* cat) {
    // 상태 진입 시 초기 행동 설정
    cat->SetAction(IDLE);
    cat->targetSpeedX = 0;
}

void IdleState::Update(Cat* cat) {
    auto& gm = GameManager::get();
    cat->TryEnterSleepState(); // 잠잘지 확인
    cat->ApplyPhysics(); // 중력 처리

    if (gm.fishingRodActive && gm.fishingRod && gm.fishingRod->IsToyNear(cat)) {
        cat->ChangeState(new HuntState());
        return;
    }

    // std::cout << cat->timeToThink << std::endl;
    // cout << "health: " << cat->GetHealth() << endl;

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

    // State 강제 변환용 코드
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) { // 컨트롤 키 누르면 사냥 시작
    cat->ChangeState(new HuntState());
    return;
}
}

void GrabbedState::Enter(Cat* cat) {
    cat->SetAction(GRABBED);
    cat->targetSpeedX = 0;
    cat->isGrounded = false;
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

void HuntState::Update(Cat* cat) {
    POINT pt;
    GetCursorPos(&pt); // 마우스 위치 가져오기
    
    auto& gm = GameManager::get();
    POINT toy_pos = gm.fishingRod.GetToyPosition();

    float dx = (float)(toy_pos.x - (cat->posX + NECK_OFFSET_X));
    float dy = (float)(toy_pos.y - (cat->posY + NECK_OFFSET_Y));
    float distance = sqrt(dx * dx + dy * dy);

    cat->ApplyPhysics();

    // 1. 이미 점프 중일 때 로직
    if (!cat->isGrounded) {

        // 공중에서 마우스와 충분히 가까워지면 낚아채기
        if (distance < 50.0f && catchTimer <= 0) {
            cat->SetAction(PAW); // 앞발 휘두르기
            catchTimer = 20;     // 낚아채기 동작 유지
            cout << "dd!" << endl;
        }
        
        if (catchTimer > 0) catchTimer--;
        return;
    }

    // 바닥에 착지하면 점프 상태 해제
    if (cat->isGrounded && hasJumped) {
        hasJumped = false;
        cat->targetSpeedX = 0;
        cat->ChangeState(new IdleState());
        return;
    }

    // 2. 바닥에서 마우스를 향해 준비/이동
    cat->isLookingRight = (dx > 0);

    // 거리가 너무 멀면 일단 걷기
    if (distance > 300.0f) {
        cat->SetAction(MOVE);
        cat->targetSpeedX = (dx > 0) ? MOVE_SPEED : -MOVE_SPEED;
    } 
    // 적정 거리(100~300px)면 점프 공격!
    else if (distance > 50.0f) {
        cat->SetAction(JUMP);

        // 마우스 거리에 비례한 동적 점프 힘 계산
        float jumpPowerY = 10.0f + (abs(dy) * 0.05f); 
        float jumpPowerX = (dx / 15.0f); // 거리에 따른 수평 속도

        cat->speedY = -jumpPowerY;
        cat->targetSpeedX = jumpPowerX;
        hasJumped = true;
    }
    // 너무 가까우면 그냥 솜방망이질
    else {
        cat->SetAction(PAW);
        cat->targetSpeedX = 0;
    }

    // 마우스가 너무 멀어지면 사냥 포기 (테스트용)
    if (distance > 800.0f) {
        cat->ChangeState(new IdleState());
        return;
    }
}

// 음식을 탐색하는 헬퍼 함수
Food* HuntState::findNearestFood(Cat* cat) {
    auto& gm = GameManager::get();
    Food* nearest = nullptr;
    float minDist = 99999;

    for (auto& food : gm.foods) {
        if (food.owner == nullptr) continue;

        float dist = distance(cat->x, cat->y, food.x, food.y);
        if (dist < minDist && dist < 200) {
            minDist = dist;
            nearest = &food;
        }
    }
    return nearest;
}

void EatState::Update(Cat* cat) {
    // cat hunger 증가
    
}