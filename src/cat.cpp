// cat.cpp
#include "cat.h"
#include "cat_state.h"
#include "cat_states.h"
#include <cmath>

Cat::Cat(int startX, int startY, int type, Image* SharedImage)
    :posX(startX), 
    posY(startY), 
    catType(type), 
    speedX(0), 
    speedY(0), 
    angle(0), 
    swingSpeed(0),
    isDragging(false), 
    partner(nullptr), 
    isGrooming(false),
    currentState(nullptr)
{
    currentAction = IDLE;
    currentFrame = 0;
    maxFrame = ACTION_FRAMES[IDLE];
    timeToThink = rand() % 50;
    animTimer = rand() % 100; // 서로 다르게 움직이게 오프셋
    animTimerAccumulator = rand() % ANIM_SPEED; // 초기값 랜덤 (서로 다르게 깜빡이게)
    
    // 고양이 초기 상태 설정
    ChangeState(new IdleState());

    // 성격 설정
    switch (type) {
    case 102: // 흰 냥
        energy = 20 + (rand() % 10);
        friendliness = 20 + (rand() % 40);
        lazy = 70 + (rand() % 30);
        break;
    case 103: // 샴
        energy = 40 + (rand() % 30);
        friendliness = 60 + (rand() % 20);
        lazy = 20 + (rand() % 10);
        break;
    default:
        energy = 50;
        friendliness = 50;
        lazy = 50;
        break;
    }

    myImage = SharedImage;
}

Cat::~Cat() {
    if (currentState) {
        currentState->Exit(this);
        delete currentState;    // 상태 객체 메모리 해제
    }
}

void Cat::ChangeState(CatState* newState) {
    if (currentState) {
        currentState->Exit(this);
        delete currentState;
    }
    currentState = newState;
    currentState->Enter(this);
}

void Cat::SetAction(int newAction) {
    if (currentAction != newAction) {
        currentAction = newAction;
        currentFrame = 0; // 행동 바뀌면 처음부터 재생
        maxFrame = ACTION_FRAMES[newAction]; // 최대 프레임 수 갱신
    }
}

void Cat::Update() {
//     // 상대방이 없는데(null) 파트너라고 생각 중이면 초기화 (안전장치)
//     if (partner != nullptr && partner->partner != this) partner = nullptr;
//     if (isDragging) {
//         if (partner != nullptr) {
//             partner->partner = nullptr;
//             partner->SetAction(IDLE);
//             partner->timeToThink = 0;   // 즉시 새 행동 찾게 함

//             partner = nullptr; // 나도 솔로 복귀
//             isGrooming = false;
//         }

//         // 행잉
//         POINT pt; GetCursorPos(&pt);
//         float mouseVelX = (float)(pt.x - physicsLastX);
//         physicsLastX = pt.x; // 다음 프레임을 위해 갱신

//         float force = mouseVelX * 0.03f;
        
//         swingSpeed += force;
//         swingSpeed -= angle * 0.05f;
//         swingSpeed *= 0.95f;

//         // 6. 각도 적용
//         angle += swingSpeed;

//         // 7. 각도 제한 (목 꺾임 방지)
//         if (angle > 60.0f) { angle = 60.0f; swingSpeed = 0; }
//         if (angle < -60.0f) { angle = -60.0f; swingSpeed = 0; }

//         // 위치 반영 (마우스 따라가기)
//         posX = pt.x - NECK_OFFSET_X;
//         posY = pt.y - NECK_OFFSET_Y;

//         SetWindowPos(hwnd, NULL, posX, posY, 0, 0, 
//                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

//         throwSpeedX = mouseVelX;
//         speedX = 0; speedY = 0;
//     }
//     // [영역 1] 물리 엔진 & 이동 (매번 실행)
//     else {
//         RECT workArea;
//         SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
//         int floorY = workArea.bottom;
//         float friction = (posY + winH >= floorY) ? 0.5f : 0.05f;

//         // 드래그 아닐 때, 각도 0으로 복귀
//         if (angle != 0) {
//             angle *= 0.8f; // 빠르게 0으로
//             if (abs(angle) < 1.0f) angle = 0.0f;
//         }
//         // // 마찰력 적용 (감속)
//         // if (speedX > 0) {
//         //     speedX -= friction;
//         //     if (speedX < 0) speedX = 0;
//         // } else if (speedX < 0) {
//         //     speedX += friction;
//         //     if (speedX > 0) speedX = 0;
//         // }

//         // 중력 적용 (pCat 변수 사용)
//         speedY += gravity;
//         posY += (int)speedY;

//         // 바닥 충돌
//         if (posY + winH >= floorY) {
//             posY = floorY - winH;
//             speedX = 0.0f;
//             speedY = 0.0f;
//             if (currentAction == JUMP && isJumping) {
//                 isJumping = false;
//                 SetAction(IDLE); // 멤버 함수 호출
//             }
//         }

//         if (targetSpeedX != 0) speedX = targetSpeedX;
//         // 좌우 이동
//         posX += speedX;

//         // 벽 충돌
//         bool hitWall = false;
//         if (posX <= 0) {
//             posX = 0;
//             hitWall = true;
//         }
//         else if (posX >= screenW - winW) {
//             posX = screenW - winW;
//             hitWall = true;
//         }

//         if (hitWall && speedX != 0) {
//             targetSpeedX = 0;
//             SetAction(PAW);
//             timeToThink = 20; 
//         }

//         // 위치 반영
//         // ★ 중요: 여기서 직접 창 이동시킴 (플래그 최적화 포함)
//         SetWindowPos(hwnd, NULL, posX, posY, 0, 0, 
//             SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
//     }
    // 현재 상태의 Update 로직 실행
    if (currentState) {
        currentState->Update(this);
    }

    // [영역 2] 애니메이션 & AI (누적 시간 사용)
    animTimerAccumulator += PHYSICS_SPEED;

    if (animTimerAccumulator >= ANIM_SPEED) { 
        animTimerAccumulator = 0; 

        // 프레임 넘기기 (개별)
        currentFrame = (currentFrame + 1) % maxFrame;
        
        // 쓰다듬기 게이지 감소
        if (rubCount > 0) {
            rubDecayTimer++;
            if (rubDecayTimer > 10) {
                rubCount -= 10;
                if (rubCount < 0) rubCount = 0;
            }
        }
    }
}

void Cat::Render(HDC hdc, int w, int h) {
    // 1. 메모리 DC 및 비트맵 생성 (더블 버퍼링)
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 2. 배경 지우기 (잔상 방지)
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
    RECT rc = { 0, 0, w, h };
    FillRect(memDC, &rc, hBrush);
    DeleteObject(hBrush);

    // 3. GDI+ 초기 설정
    Graphics graphics(memDC);
    graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);
    graphics.SetPixelOffsetMode(PixelOffsetModeHalf);

    // 4. 고양이 그리기 로직
    if (myImage != nullptr) {
        int drawW = (int)(FRAME_WIDTH * SCALE);
        int drawH = (int)(FRAME_HEIGHT * SCALE);
        Rect destRect(0, 0, drawW, drawH);

        int srcX = currentFrame * FRAME_WIDTH;
        int srcY = currentAction * FRAME_HEIGHT;
        int srcW = FRAME_WIDTH;

        // 좌우 반전 처리
        if (!isLookingRight) {
            srcX += FRAME_WIDTH;
            srcW = -FRAME_WIDTH;
        }

        // 회전 처리
        GraphicsState state = graphics.Save();
        float pivotX = (float)NECK_OFFSET_X;
        float pivotY = (float)NECK_OFFSET_Y;
        
        graphics.TranslateTransform(pivotX, pivotY);
        graphics.RotateTransform(angle);
        graphics.TranslateTransform(-pivotX, -pivotY);

        // 실제 그리기
        graphics.DrawImage(myImage, destRect, srcX, srcY, srcW, FRAME_HEIGHT, UnitPixel);
        
        graphics.Restore(state);
    }

    // 5. 메모리 DC의 내용을 실제 화면 HDC로 복사
    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    // 6. 리소스 정리
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

// void Cat::Think() {
//     if (isDragging) return;
//     if (partner != nullptr) {
//         if (partner->partner == this) {
//             partner->partner = nullptr;
//             partner->timeToThink = 0; // 상대도 즉시 다른 행동 하도록
//         }
        
//         // 나도 해방
//         partner = nullptr;
//         isGrooming = false;
        
//         // 그리고 나서 아래의 일반적인 랜덤 행동 로직으로 넘어감 (자연스럽게 헤어짐)
//     }
//     // 랜덤으로 다음 행동 결정 (0: IDLE, 1: WALK_LEFT, 2: WALK_RIGHT)

//     int idleW = lazy * 2;
//     int moveW = energy * 2;
//     int sleepW = (100 - energy);
//     int cleanW = friendliness;

//     int totalW = idleW + moveW + sleepW + cleanW;
//     int choice = rand() % totalW;

//     if (choice < idleW) {
//         // 가만히 있기
//         SetAction((rand()%2 == 0) ? IDLE : IDLE2);
//         targetSpeedX = 0;
//         timeToThink = 30 + lazy;
//     }
//     else if (choice < idleW + moveW) {
//         // 이동
//         SetAction((rand()%2 == 0) ? MOVE : MOVE2);
//         targetSpeedX = (rand() % 2 == 0) ? -MOVE_SPEED : MOVE_SPEED;
//         if (energy > 80) targetSpeedX *= 1.5; // 광란의 질주
//         isLookingRight = (targetSpeedX > 0);
//         timeToThink = 10 + rand() % (100 - energy); // 에너지가 많으면 금방 다음 행동 함
//     }
//     else if (choice < idleW + moveW + sleepW) {
//         // 잠자기
//         SetAction(SLEEP);
//         targetSpeedX = 0;
//         timeToThink = 100 + (lazy * 2);
//     }
//     else {
//         // [그루밍/기타]
//         SetAction((rand() % 2 == 0) ? CLEAN : CLEAN2);
//         targetSpeedX = 0;
//         timeToThink = 20 + rand() % 40;
//     }
//     if (friendliness > 40 && rand() % 100 < 10) {
//             // 전역 벡터 'cats'에 접근해야 함 (extern 선언 필요하거나 매개변수로 받아야 함)
//         // 여기서는 편의상 전역변수 cats가 있다고 가정
//         for (Cat* other : cats) {
//             if (other == this) continue; // 나 자신은 제외
//             if (other->partner != nullptr) continue; // 쟤가 이미 딴 애랑 놀고 있으면 패스
//             if (other->isDragging) continue; // 잡려가고 있으면 패스

//             // 거리 체크 (150픽셀 이내)
//             if (GetDistance(this, other) < 150.0f) {
                
//                 // ★ 상호작용 시작! (커플 성사)
//                 this->partner = other;
//                 other->partner = this;

//                 // 1. 역할 분담
//                 this->isGrooming = true;  // 내가 해주는 쪽
//                 other->isGrooming = false; // 쟤는 받는 쪽

//                 int timeToGrooming = 120 + rand() % 100;
//                 // 2. 행동 설정 (Giver)
//                 this->SetAction(CLEAN); // 핥는 모션 (CLEAN 재활용)
//                 this->targetSpeedX = 0;
//                 this->timeToThink = timeToGrooming; // 꽤 오래 함 (약 2.5초)

//                 // 3. 행동 설정 (Receiver)
//                 other->SetAction(LIE); // 눕거나 앉아있기
//                 other->targetSpeedX = 0;
//                 other->timeToThink = timeToGrooming; // 나랑 똑같이 끝내야 함

//                 // 4. 위치 보정 (중요: 서로 바라보게 만들기)
//                 // 내가 쟤보다 왼쪽에 있으면?
//                 if (this->posX < other->posX) {
//                     this->isLookingRight = true;  // 나는 오른쪽 봄
//                     other->isLookingRight = false; // 쟤는 왼쪽 봄 (마주보기)
                    
//                     // 딱 붙여주기 (겹치지 않게 약간 거리 둠)
//                     this->posX = other->posX - 32; 
//                 } 
//                 else { // 내가 오른쪽에 있으면
//                     this->isLookingRight = false;
//                     other->isLookingRight = true;
//                     this->posX = other->posX + 32;
//                 }
                
//                 // Y축 맞추기 (바닥 높이 통일)
//                 this->posY = other->posY; 

//                 // 즉시 윈도우 이동 반영 (안 하면 텔레포트처럼 보임)
//                 SetWindowPos(this->hwnd, NULL, this->posX, this->posY, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
//                 SetWindowPos(other->hwnd, NULL, other->posX, other->posY, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

//                 return; // 상호작용 성공했으니 Think 종료
//             }
//         }
//     }
// }

void Cat::ApplyPhysics() {
    // 1. 바닥 정보 가져오기
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    int floorY = workArea.bottom;

    // 2. 각도 복구 (흔들림 멈춤)
    if (angle != 0) {
        angle *= 0.8f; 
        if (abs(angle) < 1.0f) angle = 0.0f;
    }

    // 3. 중력 및 수직 이동
    speedY += gravity;
    posY += (int)speedY;

    // 4. 바닥 충돌 처리
    if (posY + winH >= floorY) {
        posY = floorY - winH;
        speedY = 0.0f;
        isGrounded = true;
        if (currentAction == JUMP) SetAction(IDLE);
        // 바닥에 닿으면 좌우 속도 감속 (마찰력)
        speedX *= 0.8f; 
        if (abs(speedX) < 0.1f) speedX = 0.0f;
    }
    else {
        isGrounded = false;
    }

    // 5. 좌우 이동 및 벽 충돌
    if (targetSpeedX != 0) speedX = targetSpeedX;
    posX += (int)speedX;

    if (posX <= 0) {
        posX = 0;
        if (speedX < 0) { speedX = 0; targetSpeedX = 0; SetAction(PAW); timeToThink = 20; }
    }
    else if (posX >= screenW - winW) {
        posX = screenW - winW;
        if (speedX > 0) { speedX = 0; targetSpeedX = 0; SetAction(PAW); timeToThink = 20; }
    }

    // 6. 실제 윈도우 위치 반영
    SetWindowPos(hwnd, NULL, posX, posY, 0, 0, 
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
}

void Cat::TryEnterSleepState() {
    if (GetHealth() <= 0 && isGrounded) {  // 체력이 없고 땅에 닿아있으면
        ChangeState(new SleepState());
        return;
    }
};

void Cat::TryEnterHuntState() {
    
}