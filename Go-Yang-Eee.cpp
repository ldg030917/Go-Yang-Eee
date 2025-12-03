// ... (위쪽 헤더는 동일) ...
#include <windows.h>
#include <gdiplus.h>
#include <time.h> // 맨 위에 추가
#include <shlwapi.h> // IStream 변환용 (필요시)

// ... (링크 설정 동일) ...

using namespace Gdiplus;

// ★ 설정: 본인 아틀라스 이미지에 맞게 수정하세요!
const int FRAME_WIDTH = 32;   // 프레임 1개의 가로 크기
const int FRAME_HEIGHT = 32;  // 프레임 1개의 세로 크기
const int ANIM_SPEED = 100;   // 애니메이션 속도 (ms)
const float SCALE = 3.0f;   // 두 배로 키우기
const int MOVE_SPEED = 10;

int speedX = 0; // 0: 정지, -5: 왼쪽, 5: 오른쪽
int timeToThink = 0; // 다음 행동 결정까지 남은 시간(프레임 수)

// 화면 크기 (나중에 화면 밖으로 나가는 거 막으려고)
int screenW = GetSystemMetrics(SM_CXSCREEN);
int screenH = GetSystemMetrics(SM_CYSCREEN);

bool isLookingRight = true; // true: 오른쪽, false: 왼쪽

bool isDragging = false; // 지금 잡혀있는지?
POINT dragOffset;

enum ActionType {
    IDLE = 0,
    IDLE2,
    CLEAN,
    CLEAN2,
    MOVE,
    MOVE2,
    SLEEP,
    PAW,
    JUMP,
    SCARED,
    MAX_ACTIONS
};

const int ACTION_FRAMES[MAX_ACTIONS] = {4, 4, 4, 4, 8, 8, 4, 6, 7, 8};
int currentAction = IDLE;
int maxFrame = ACTION_FRAMES[IDLE]; // 현재 행동의 최대 프레임 수

// 전역 변수
const wchar_t CLASS_NAME[] = L"MyDesktopPetClass";
Image* petImage = nullptr;
int currentFrame = 0; // 현재 보여줄 프레임 번호 (0 ~ FRAME_COUNT-1)

void SetAction(int newAction) {
    if (currentAction != newAction) {
        currentAction = newAction;
        currentFrame = 0; // 행동 바뀌면 처음부터 재생
        maxFrame = ACTION_FRAMES[newAction]; // 최대 프레임 수 갱신
    }
}

void Think() {
    if (isDragging) return;
    // 랜덤으로 다음 행동 결정 (0: IDLE, 1: WALK_LEFT, 2: WALK_RIGHT)
    int choice = rand() % 5; 
    // 다음 생각할 시간 설정 (예: 20~50 프레임 뒤에 다시 생각)
    timeToThink = 20 + (rand() % 30);
    switch (choice) {
    case 0: // 가만히 있기
        SetAction((rand()%2 == 0) ? IDLE : IDLE2);
        speedX = 0;
        break;

    case 1: // 왼쪽으로 걷기
        SetAction((rand()%2 == 0) ? MOVE : MOVE2); // 걷는 모션 (상수 없으면 RUN 등으로 대체)
        speedX = -MOVE_SPEED;     // 왼쪽 이동
        isLookingRight = false;
        timeToThink = 10 + (rand() % 20);
        break;

    case 2: // 오른쪽으로 걷기
        SetAction((rand()%2 == 0) ? MOVE : MOVE2);
        speedX = MOVE_SPEED;      // 오른쪽 이동
        isLookingRight = true;
        timeToThink = 10 + (rand() % 20);
        break;
    case 3: // 잠자기
        SetAction(SLEEP);
        speedX = 0;
        break;
    case 4: // 핥기
        SetAction((rand()%2 == 0) ? CLEAN : CLEAN2);
        speedX = 0;
        break;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // ★ 타이머 시작 (ID: 1, 시간: ANIM_SPEED ms)
        SetTimer(hwnd, 1, ANIM_SPEED, NULL);
        return 0;

    case WM_TIMER:
        if (wParam == 1) {
            // 프레임 번호 증가 (0 -> 1 -> 2 -> 3 -> 0 ...)
            currentFrame = (currentFrame + 1) % maxFrame;
            
            // 2. AI 생각하기
            if (timeToThink > 0) {
                timeToThink--; // 생각할 시간 카운트다운
            } else {
                Think(); // 시간 다 됐으면 새로운 행동 결정!
            }

            // 3. 실제 이동 (걷는 상태라면 창 위치 옮기기)
            if (speedX != 0) {
                RECT rect;
                GetWindowRect(hwnd, &rect); // 현재 창 위치 가져오기
                
                int newX = rect.left + speedX;
                
                // 화면 밖으로 나가는지 체크 (간단하게)
                if (newX < 0) newX = 0;
                if (newX > screenW - 100) newX = screenW - 100;

                // 창의 가로 크기
                int winW = (int)(FRAME_WIDTH * SCALE);

                // ★ 벽 감지 로직
                bool hitWall = false;
                int currentWinWidth = rect.right - rect.left;
                // 1. 왼쪽 벽 충돌
                if (newX <= 0) {
                    newX = 0;
                    hitWall = true;
                }
                // 2. 오른쪽 벽 충돌
                else if (newX >= screenW - currentWinWidth - 10) {
                    newX = screenW - currentWinWidth;
                    hitWall = true;
                }

                // ★ 벽에 부딪혔다면? -> 이동 멈추고 "때리기(PAW)" 행동 개시
                if (hitWall) {
                    speedX = 0;         // 멈춤
                    SetAction(PAW);     // 벽 긁기 모션
                    timeToThink = 20;   // 20프레임 동안 긁기
                } 
                else {
                    // 벽이 아니면 정상 이동
                    SetWindowPos(hwnd, NULL, newX, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                }
            }

            // 화면 전체를 다시 그리라고 요청 (WM_PAINT 유발)
            InvalidateRect(hwnd, NULL, TRUE); 
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, 1); // 타이머 끄기
        PostQuitMessage(0);
        return 0;
    
    // ... (ESC 종료 코드는 동일) ...

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 1. 더블 버퍼링용 메모리 DC 만들기
        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        HDC memDC = CreateCompatibleDC(hdc);          // 가상의 그리기 도구
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h); // 가상의 종이
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // 2. 메모리 DC에 배경 칠하기 (투명색 마젠타로)
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
        FillRect(memDC, &rc, hBrush);
        DeleteObject(hBrush);

        // 3. GDI+로 메모리 DC에 그림 그리기
        Graphics graphics(memDC); // ★ hdc 대신 memDC 사용
        graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);
        graphics.SetPixelOffsetMode(PixelOffsetModeHalf);

        if (petImage != nullptr) {
            // ... (그리기 좌표 계산 로직 동일) ...
            int drawW = (int)(FRAME_WIDTH * SCALE);
            int drawH = (int)(FRAME_HEIGHT * SCALE);
            Rect destRect(0, 0, drawW, drawH);

            int srcX = currentFrame * FRAME_WIDTH;
            int srcY = currentAction * FRAME_HEIGHT;
            int srcW = FRAME_WIDTH;

            if (!isLookingRight) {
                srcX += FRAME_WIDTH;
                srcW = -FRAME_WIDTH;
            }

            graphics.DrawImage(petImage, destRect, srcX, srcY, srcW, FRAME_HEIGHT, UnitPixel);
        }

        // 4. 완성된 그림을 실제 화면에 복사 (BitBlt)
        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

        // 5. 정리
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    // ★ 중요: 배경 지우기 방지 (WM_ERASEBKGND)
    case WM_ERASEBKGND:
        return 1; // "내가 이미 다 지웠으니까 윈도우 너는 아무것도 하지 마" (깜빡임 원인 제거)

    case WM_RBUTTONUP: {
        // 1. 빈 팝업 메뉴 만들기
        HMENU hMenu = CreatePopupMenu();
        
        // 2. 메뉴 항목 추가 (ID: 1001)
        AppendMenuW(hMenu, MF_STRING, 1001, L"종료(Exit)");
        
        // 3. 마우스 위치에 메뉴 띄우기
        POINT pt;
        GetCursorPos(&pt); // 현재 마우스 좌표 가져오기
        
        // 메뉴 보여주고 선택 기다리기 (Blocking 아님)
        SetForegroundWindow(hwnd); // 메뉴 띄우기 전 포커스 잡기 (버그 방지)
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        
        DestroyMenu(hMenu); // 다 쓴 메뉴 껍데기 삭제
        return 0;
    }

    // 1. 마우스 왼쪽 버튼 누름 (잡기 시작)
    case WM_LBUTTONDOWN: {
        isDragging = true;
        speedX = 0;
        // 잡힌 모션으로 변경 (GRABBED 같은 액션 미리 만들어둬야 함)
        // 없다면 일단 IDLE이나 놀란 표정(SCARED) 등으로
        SetAction(SCARED); // ★ 잡힌 모션 상수로 변경 필요
        
        // 현재 마우스 위치 기억 (고양이의 '어디'를 잡았는지 계산)
        POINT pt;
        GetCursorPos(&pt);
        
        RECT rect;
        GetWindowRect(hwnd, &rect);
        
        // 오차 저장 (마우스좌표 - 창시작점)
        dragOffset.x = pt.x - rect.left;
        dragOffset.y = pt.y - rect.top;
        
        // 마우스가 창 밖으로 나가도 계속 잡고 있게 함 (필수)
        SetCapture(hwnd); 
        return 0;
    }

    // 2. 마우스 움직임 (드래그 중)
    case WM_MOUSEMOVE: {
        if (isDragging) {
            POINT pt;
            GetCursorPos(&pt); // 현재 마우스 위치(전체화면 기준)
            
            // 새 위치 = 현재 마우스 - 아까 저장한 오차
            int newX = pt.x - dragOffset.x;
            int newY = pt.y - dragOffset.y;
            
            // 창 위치 옮기기
            SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        return 0;
    }

    // 3. 마우스 왼쪽 버튼 뗌 (놓아주기)
    case WM_LBUTTONUP: {
        if (isDragging) {
            isDragging = false;
            ReleaseCapture(); // 마우스 캡쳐 해제
            
            // 땅에 떨어지는 모션이나 IDLE로 복귀
            SetAction(IDLE); 
            
            // 바로 딴짓 못하게 딜레이 좀 주기
            timeToThink = 30;
        }
        return 0;
    }

    case WM_COMMAND: {
        // 메뉴에서 무언가 선택했을 때 실행됨
        if (LOWORD(wParam) == 1001) { // 아까 정한 ID가 1001이면
            DestroyWindow(hwnd); // 종료
        }
        return 0;
    }

    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    // ... (GdiplusStartup 부분 동일) ...
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    // WinMain 시작 부분
    srand((unsigned int)time(NULL)); // 랜덤 시드 초기화
    // WinMain 함수 시작 부분

    // // 1. user32.dll에서 함수 주소 가져오기 (동적 로딩)
    // HMODULE hUser32 = LoadLibraryW(L"user32.dll");
    // if (hUser32) {
    //     typedef BOOL (WINAPI *LPSETPROCESSDPIAWARE)(void);
    //     LPSETPROCESSDPIAWARE lpSetProcessDPIAware = (LPSETPROCESSDPIAWARE)GetProcAddress(hUser32, "SetProcessDPIAware");
        
    //     if (lpSetProcessDPIAware) {
    //         lpSetProcessDPIAware(); // 함수 실행!
    //     }
    //     FreeLibrary(hUser32);
    // }

    //SetProcessDPIAware();

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255, 0, 255)); // 배경색

    RegisterClassExW(&wc);
    // WinMain에서 CreateWindowExW 호출하기 직전에 계산
    // 1. 화면 크기 구하기
    int scrW = GetSystemMetrics(SM_CXSCREEN);
    int scrH = GetSystemMetrics(SM_CYSCREEN);

    // 2. 창 크기 (아까 계산해둔 winW, winH 사용)
    int winW = (int)(FRAME_WIDTH * SCALE);
    int winH = (int)(FRAME_HEIGHT * SCALE);

    // 3. 시작 위치 계산 (우측 하단)
    int startX = scrW - winW - 50; // 오른쪽에서 50px 떨어짐
    int startY = scrH - winH - 50; // 아래쪽에서 50px 떨어짐 (작업표시줄 고려)

    // 4. 창 생성 (좌표 부분에 변수 넣기)
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"My Pet", WS_POPUP,
        startX, startY, // ★ 여기를 100, 100 대신 변수로 교체
        winW, winH,
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);

    petImage = Image::FromFile(L"pet.png");

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete petImage;
    GdiplusShutdown(gdiplusToken);
    return 0;
}
