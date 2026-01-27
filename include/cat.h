// cat.h
#pragma once
#include "config.h"

class CatState;

class Cat {
private:
    CatState* currentState;   // 현재 상태

public:
    int GetHealth() const { return health; }
    void SetHealth(int val) { health = val; if(health > maxHealth) health = maxHealth; if(health < 0) health = 0; }
    int health;     // 현재 체력
    int maxHealth = 1000;
    float speedX = 0.0f; // 수평 속도
    float speedY = 0.0f; // 수직 속도
    float targetSpeedX = 0.0f; // 목표로 하는 속도
    int timeToThink = 0; // 다음 행동 결정까지 남은 시간(프레임 수)
    bool isGrounded = false; // 땅에 붙어있는 상태인지
    bool isLookingRight;
    int currentAction = IDLE;
    int currentFrame = 0;
    int maxFrame = ACTION_FRAMES[IDLE];
    int animTimerAccumulator;
    float angle = 0.0f; // 현재 회전 각도
    float swingSpeed = 0.0f; // 흔들림 속도
    int lastCursorX, lastCursorY;
    int physicsLastX = 0; // ★ [추가] 물리 엔진 전용 좌표 기억 변수 (Update만 건드림)
    int hunger;     // 현재 배고픔
    int maxHunger = 1000;
    
    // 2. 개별 속성 (고양이마다 다르게 줄 수 있음)
    int animTimer; // 애니메이션 타이머 (개별 동작 위해)
    int catType;   // 102(치즈), 103(검정) 등 리소스 ID
    Image* myImage = nullptr; // 자기만의 이미지 포인터 (혹은 공유 가능)
    IStream* myStream = nullptr;

    // 성격 스텟
    int energy; // 활동성
    int friendliness; // 친화력
    int lazy; // 게으름

    // 쓰다듬기 관련 (개별 관리)
    int rubCount = 0;
    int rubDecayTimer = 0;
    float throwSpeedX = 0.0f, throwSpeedY = 0.0f;

    // 왜 private 쓴거야 처음에 시발
    HWND hwnd = NULL;  //고양이만의 창(window)
    int posX = 0, posY = 0; // 현재 고양이 위치
    bool isDragging; // 드래그 확인용
    POINT dragOffset;
    // 상호작용 추가
    Cat* partner = nullptr; // 현재 상호작용 중인 상대 고양이
    bool isGrooming = false; // 내가 그루밍해주는 쪽인가

    Cat(int startX, int startY, int type, HINSTANCE hInstance);
    ~Cat();
    // FSM 제어
    void ChangeState(CatState* newState);
    void SetAction(int newAction);
    void Think();
    void Update();
    void Render(HDC hdc, int w, int h); // WM_PAINT 로직을 담당
    void ApplyPhysics(); // 물리 로직 함수
    // SleepState로 전이할 지 확인하고 조건에 맞으면 전이하는 함수
    void TryEnterSleepState();
    void TryEnterHuntState();
};

extern std::vector<Cat*> cats;
float GetDistance(Cat* a, Cat* b);
