// ... (위쪽 헤더는 동일) ...
#include <windows.h>
#include <gdiplus.h>
#include <time.h> // 맨 위에 추가
#include <shlwapi.h> // IStream 변환용 (필요시)
#include <shellapi.h> // Shell_NotifyIcon 용
#include <vector>

using namespace Gdiplus;

#define WM_TRAYICON (WM_USER + 1) // 트레이 아이콘 메시지 ID
#define ID_MY_ICON 101
#define ID_EXIT 2001
#define ID_ADD_CAT 2002
#define ID_REMOVE_CAT 2003
#define ID_HOTKEY_ADD 9001
#define ID_HOTKEY_REMOVE 9002
const int ID_TRAY_ICON = 1001;    // 트레이 아이콘 식별 번호

// ★ 설정: 본인 아틀라스 이미지에 맞게 수정하세요!
const int FRAME_WIDTH = 32;   // 프레임 1개의 가로 크기
const int FRAME_HEIGHT = 32;  // 프레임 1개의 세로 크기
const int ANIM_SPEED = 96;   // 애니메이션 속도 (ms)
const int PHYSICS_SPEED = 16; // ★ 추가: 물리 갱신 속도 (약 60 FPS)
const float SCALE = 3.0f;   // 세 배로 키우기
const int MOVE_SPEED = 5;


float gravity = 0.8f; // 중력 가속도

// 화면 크기 (나중에 화면 밖으로 나가는 거 막으려고)
int screenW = GetSystemMetrics(SM_CXSCREEN);
int screenH = GetSystemMetrics(SM_CYSCREEN);

// ★ [수정] 위치와 크기를 관리할 전역 변수 추가
int posX = 0; // 현재 윈도우 X 위치
int posY = 0; // 현재 윈도우 Y 위치
int winW = 0; // 창 가로 크기 (계산된 값)
int winH = 0; // 창 세로 크기 (계산된 값)

int debugDX = 0;
int debugDY = 0;

int lastCursorX = 0;     // 마우스 움직임 계산용
int lastCursorY = 0;
const int RUB_THRESHOLD = 250; // 이만큼 문지르면 기분 좋아짐

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
    WIP,
    GRABBED,
    LIE,
    YAWN,
    MAX_ACTIONS
};

const int ACTION_FRAMES[MAX_ACTIONS] = {4, 4, 4, 4, 8, 8, 4, 6, 7, 8, 4, 8, 4, 5};
// 전역 변수
const wchar_t CLASS_NAME[] = L"MyDesktopPetClass";

// 리소스에서 이미지를 불러오는 함수
Image* LoadImageFromResource(HINSTANCE hInstance, int resId, IStream** outStream) {
    // 1. 리소스 찾기
    HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resId), RT_RCDATA);
    if (!hResource) return nullptr;

    // 2. 크기 확인 및 로드
    DWORD imageSize = SizeofResource(hInstance, hResource);
    HGLOBAL hGlobal = LoadResource(hInstance, hResource);
    void* pData = LockResource(hGlobal);

    // 3. 메모리 할당 및 복사
    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (!hBuffer) return nullptr;

    void* pBuffer = GlobalLock(hBuffer);
    CopyMemory(pBuffer, pData, imageSize);
    GlobalUnlock(hBuffer);

    // 4. 스트림 생성 (CreateStreamOnHGlobal)
    IStream* pStream = nullptr; // 지역 변수로 생성
    if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK) {
        Image* pImage = Image::FromStream(pStream);

        // ★ 중요: 만든 스트림을 밖으로 전달!
        if (outStream != nullptr) {
            *outStream = pStream; 
        }

        return pImage;
    }
    
    GlobalFree(hBuffer);
    return nullptr;
}

struct Cat {
    HWND hwnd = NULL;  //고양이만의 창(window)
    int posX = 0, posY = 0; // 현재 고양이 위치
    int speedX = 0; // 0: 정지, -5: 왼쪽, 5: 오른쪽
    float vy = 0.0f; // 수직 속도
    int timeToThink = 0; // 다음 행동 결정까지 남은 시간(프레임 수)
    bool isJumping = false; // 점프/낙하 상태 확인용
    bool isLookingRight = true;
    int currentAction = IDLE;
    int currentFrame = 0;
    int maxFrame = ACTION_FRAMES[IDLE];
    int animTimerAccumulator;

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

    bool isDragging; // 드래그 확인용
    POINT dragOffset;

    Cat(int startX, int startY, int type, HINSTANCE hInstance) {
        posX = startX;
        posY = startY;
        catType = type;
        vy = 0;
        speedX = 0;
        isJumping = false;
        isLookingRight = true;
        isDragging = false;
        currentAction = IDLE;
        currentFrame = 0;
        maxFrame = ACTION_FRAMES[IDLE];
        timeToThink = rand() % 50;
        animTimer = rand() % 100; // 서로 다르게 움직이게 오프셋
        animTimerAccumulator = rand() % ANIM_SPEED; // 초기값 랜덤 (서로 다르게 깜빡이게)
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
        // 이미지 로딩 (함수 재사용)
        myImage = LoadImageFromResource(hInstance, catType, &myStream); 
    }

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

        int idleW = lazy * 2;
        int moveW = energy * 2;
        int sleepW = (100 - energy);
        int cleanW = friendliness;

        int totalW = idleW + moveW + sleepW + cleanW;
        int choice = rand() % totalW;

        if (choice < idleW) {
            // 가만히 있기
            SetAction((rand()%2 == 0) ? IDLE : IDLE2);
            speedX = 0;
            timeToThink = 30 + lazy;
        }
        else if (choice < idleW + moveW) {
            // 이동
            SetAction((rand()%2 == 0) ? MOVE : MOVE2);
            speedX = (rand() % 2 == 0) ? -MOVE_SPEED : MOVE_SPEED;
            if (energy > 80) speedX *= 1.5; // 광란의 질주
            isLookingRight = (speedX > 0);
            timeToThink = 10 + rand() % (100 - energy); // 에너지가 많으면 금방 다음 행동 함
        }
        else if (choice < idleW + moveW + sleepW) {
            // 잠자기
            SetAction(SLEEP);
            speedX = 0;
            timeToThink = 100 + (lazy * 2);
        }
        else {
            // [그루밍/기타]
            SetAction((rand() % 2 == 0) ? CLEAN : CLEAN2);
            speedX = 0;
            timeToThink = 20 + rand() % 40;
        }
    }

    void Update() {
        // [물리 엔진 로직 이동]
        // WindowProc에 있던 pCat->posX += ... 코드들을 여기에 복사
        // hwnd 대신 this->hwnd, pCat-> 대신 this-> 사용
        
        // [영역 1] 물리 엔진 & 이동 (매번 실행)
        if (!isDragging) {
            RECT workArea;
            SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
            int floorY = workArea.bottom;

            // 중력 적용 (pCat 변수 사용)
            vy += gravity;
            posY += (int)vy;

            // 바닥 충돌
            if (posY + winH >= floorY) {
                posY = floorY - winH;
                vy = 0.0f;
                if (currentAction == JUMP && isJumping) {
                    isJumping = false;
                    SetAction(IDLE); // 멤버 함수 호출
                }
            }

            // 좌우 이동
            posX += speedX;

            // 벽 충돌
            bool hitWall = false;
            if (posX <= 0) {
                posX = 0;
                hitWall = true;
            }
            else if (posX >= screenW - winW) {
                posX = screenW - winW;
                hitWall = true;
            }

            if (hitWall && speedX != 0) {
                speedX = 0;
                SetAction(PAW);
                timeToThink = 20; 
            }

            // 위치 반영
            // ★ 중요: 여기서 직접 창 이동시킴 (플래그 최적화 포함)
            SetWindowPos(hwnd, NULL, posX, posY, 0, 0, 
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
        }

        // [영역 2] 애니메이션 & AI (누적 시간 사용)
        animTimerAccumulator += PHYSICS_SPEED;

        if (animTimerAccumulator >= ANIM_SPEED) { 
            animTimerAccumulator = 0; 

            // 프레임 넘기기 (개별)
            currentFrame = (currentFrame + 1) % maxFrame;

            // AI 생각
            if (!isDragging) {
                if (timeToThink > 0) timeToThink--;
                else Think(); // 멤버 함수 호출
            }
            
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
};

std::vector<Cat*> cats;

void InitTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON; // 이 메시지로 알림을 받겠다
    
    // 아이콘 로드 (본인 아이콘 있으면 LoadImage로 교체, 지금은 기본 느낌표 아이콘)
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_MY_ICON)); 
    
    // 마우스 올렸을 때 툴팁
    lstrcpyW(nid.szTip, L"My Desktop Pet");

    Shell_NotifyIconW(NIM_ADD, &nid);
}

void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Cat* pCat = NULL;
    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pCat = (Cat*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCat);
        
        static bool isFirstWindow = true;
        if (isFirstWindow) {
            isFirstWindow = false;
            InitTrayIcon(hwnd);
            // 핫키 등록...
            RegisterHotKey(hwnd, ID_HOTKEY_ADD, MOD_CONTROL | MOD_ALT, 'C');
            RegisterHotKey(hwnd, ID_HOTKEY_REMOVE, MOD_CONTROL | MOD_ALT, 'D');
        }
        return 0;
    }
    else {
        // WM_CREATE가 아니면 저장된 포인터를 꺼내옴
        pCat = (Cat*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    // ★ 안전장치: pCat이 없으면(NULL) 아무것도 하지 말고 기본 처리
    // (이거 없으면 WM_PAINT 등에서 터짐)
    if (!pCat) return DefWindowProcW(hwnd, uMsg, wParam, lParam);

    switch (uMsg) {
    case WM_DESTROY: {
        RemoveTrayIcon(hwnd);
        KillTimer(hwnd, 1);
        
        if (cats.empty()) {
            UnregisterHotKey(hwnd, ID_HOTKEY_ADD); // 해제
            UnregisterHotKey(hwnd, ID_HOTKEY_REMOVE);
            PostQuitMessage(0);
        }
        return 0;
    }
    
    case WM_TRAYICON: {
        if (lParam == WM_RBUTTONUP) {
            HMENU hMenu = CreatePopupMenu();

            AppendMenuW(hMenu, MF_STRING, ID_ADD_CAT, L"고양이 추가 (CTRL+ALT+C)");
            AppendMenuW(hMenu, MF_STRING, ID_REMOVE_CAT, L"고양이 보내기 (CTRL+ALT+D)");
            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL); // 줄 긋기
            AppendMenuW(hMenu, MF_STRING, ID_EXIT, L"종료 (Exit)");

            POINT pt; GetCursorPos(&pt);
            SetForegroundWindow(hwnd); 
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        return 0;
    }
    
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc; GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
        FillRect(memDC, &rc, hBrush);
        DeleteObject(hBrush);

        Graphics graphics(memDC);
        graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);
        graphics.SetPixelOffsetMode(PixelOffsetModeHalf);

        // ★ pCat의 이미지 그리기
        if (pCat->myImage != nullptr) {
            int drawW = (int)(FRAME_WIDTH * SCALE);
            int drawH = (int)(FRAME_HEIGHT * SCALE);
            Rect destRect(0, 0, drawW, drawH);

            int srcX = pCat->currentFrame * FRAME_WIDTH;
            int srcY = pCat->currentAction * FRAME_HEIGHT;
            int srcW = FRAME_WIDTH;

            if (!pCat->isLookingRight) {
                srcX += FRAME_WIDTH;
                srcW = -FRAME_WIDTH; // 뒤집기
            }

            graphics.DrawImage(pCat->myImage, destRect, srcX, srcY, srcW, FRAME_HEIGHT, UnitPixel);
        }

        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_LBUTTONDOWN: {
        pCat->isDragging = true;
        pCat->speedX = 0;
        pCat->SetAction(GRABBED);
        
        POINT pt; GetCursorPos(&pt);
        RECT rect; GetWindowRect(hwnd, &rect);
        
        // 구조체 멤버 dragOffset 사용
        pCat->dragOffset.x = pt.x - rect.left;
        pCat->dragOffset.y = pt.y - rect.top;
        
        SetCapture(hwnd);
        return 0;
    }

    case WM_MOUSEMOVE: {
        // 안전장치: 포인터 체크
        if (!pCat) return 0;
        POINT pt; GetCursorPos(&pt);
        // lastCursorX도 개별 고양이마다 다를 수 있으니 pCat에 넣는 게 좋지만
        // 일단은 계산용으로 쓰임 (단, 동시 쓰다듬기 시 버그 가능성 있음)
        // 여기서는 간단히 처리:
        
        if (pCat->isDragging) {            
            int newX = pt.x - pCat->dragOffset.x;
            int newY = pt.y - pCat->dragOffset.y;
            
            pCat->posX = newX;
            pCat->posY = newY;
            pCat->vy = 0.0f;
            // 좌표가 화면 밖으로 튀면 소리 재생 + 복구
            if (newX < -200 || newX > 5000) {
                MessageBeep(MB_ICONHAND); // 경고음!
                pCat->posX = 100; // 강제 복구
                pCat->isDragging = false; // 드래그 강제 해제
            }

            SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        else {
            // 쓰다듬기 로직 (간소화)
            int dx = abs(pt.x - lastCursorX); // 전역 변수 사용 (주의)
            int dy = abs(pt.y - lastCursorY);
            
            if (dx + dy > 0 && dx + dy < 100) {
                pCat->rubCount += (dx + dy);
                InvalidateRect(hwnd, NULL, FALSE);
                
                if (pCat->rubCount > RUB_THRESHOLD) {
                    if (pCat->currentAction != SLEEP && pCat->currentAction != PAW) {
                        pCat->SetAction(CLEAN2);
                        pCat->speedX = 0;
                        pCat->timeToThink = 20;
                        pCat->rubCount = 0;
                    }
                }
            }
        }
        lastCursorX = pt.x;
        lastCursorY = pt.y;
        return 0;
    }

    case WM_LBUTTONUP: {
        if (pCat->isDragging) {
            pCat->isDragging = false;
            ReleaseCapture();
            pCat->SetAction(IDLE); 
            pCat->timeToThink = 30;
        }
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        
        switch (id) {
        case ID_EXIT:
            // 모든 고양이 정리 후 종료하는 게 안전함
            for (Cat* c : cats) {
                if (c->hwnd) DestroyWindow(c->hwnd);
            }
            cats.clear(); // 바로 비워서 WM_DESTROY에서 empty() 체크 통과하게 함
            break;

        case ID_ADD_CAT: {
            // [고양이 추가]
            RECT workArea; 
            SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
            
            // 약간 랜덤한 위치에 스폰 (겹침 방지)
            int startX = (workArea.right / 2) + (rand() % 200 - 100);
            int startY = workArea.bottom - 200;
            int type = (rand() % 2 == 0) ? 102 : 103;
            
            HINSTANCE hInst = GetModuleHandle(NULL);
            Cat* newCat = new Cat(startX, startY, type, hInst);
            
            newCat->hwnd = CreateWindowExW(
                WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                CLASS_NAME, L"My Pet", WS_POPUP,
                newCat->posX, newCat->posY, winW, winH,
                NULL, NULL, hInst, newCat
            );
            
            SetLayeredWindowAttributes(newCat->hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
            ShowWindow(newCat->hwnd, SW_SHOW);
            
            cats.push_back(newCat);
            break;
        }

        case ID_REMOVE_CAT: {
            // [고양이 삭제]
            if (!cats.empty()) {
                Cat* victim = cats.back(); // 마지막 녀석 선택
                
                // 윈도우 파괴
                if (victim->hwnd) DestroyWindow(victim->hwnd);
                
                // 메모리 정리
                if (victim->myImage) delete victim->myImage;
                if (victim->myStream) victim->myStream->Release();
                delete victim;
                
                cats.pop_back(); // 리스트에서 제거
                
                // 0마리 되면 종료할지? (선택사항)
                // if (cats.empty()) PostQuitMessage(0);
            }
            break;
        }
        } // end switch
        return 0;
    }

    case WM_HOTKEY: {
        if (wParam == ID_HOTKEY_ADD) {
            // 고양이 추가 로직 실행!
            // (코드를 복사하지 말고, WM_COMMAND를 강제로 호출하는 게 깔끔함)
            SendMessage(hwnd, WM_COMMAND, ID_ADD_CAT, 0);
        }
        else if (wParam == ID_HOTKEY_REMOVE) {
            SendMessage(hwnd, WM_COMMAND, ID_REMOVE_CAT, 0);
        }
        return 0;
    }

    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    // 1. GDI+ 초기화
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    srand((unsigned int)time(NULL));

    // 2. 윈도우 클래스 등록
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_MY_ICON));

    RegisterClassExW(&wc);

    // 3. 화면 크기 및 창 크기 계산
    screenW = GetSystemMetrics(SM_CXSCREEN);
    screenH = GetSystemMetrics(SM_CYSCREEN);
    winW = (int)(FRAME_WIDTH * SCALE);
    winH = (int)(FRAME_HEIGHT * SCALE);
    POINT pt;
    GetCursorPos(&pt);
    lastCursorX = pt.x;
    lastCursorY = pt.y;

    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    // ★ 4. 고양이 3마리 생성 및 창 띄우기
    int startX = workArea.right - winW - 500;
    int startY = workArea.bottom - winH;

    for (int i = 0; i < 1; i++) {
        // (1) 고양이 객체 생성
        // 리소스 ID: 랜덤
        int resId = rand() % 2 == 1 ? 103 : 102; 
        
        // 생성자에서 posX, posY, 리소스 로딩까지 다 함
        Cat* newCat = new Cat(startX - (i * 10), startY, resId, hInstance);
        
        // (2) 윈도우 생성 (중요: 마지막 인자에 newCat 포인터 전달)
        newCat->hwnd = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            CLASS_NAME, L"My Pet", WS_POPUP,
            newCat->posX, newCat->posY, 
            winW, winH,
            NULL, NULL, hInstance, 
            newCat // ★ WM_CREATE의 lParam으로 전달됨
        );

        // (3) 투명화 및 표시
        SetLayeredWindowAttributes(newCat->hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
        ShowWindow(newCat->hwnd, nCmdShow);

        // (4) 관리 리스트에 추가
        cats.push_back(newCat);
    }

    // 5. 메시지 루프 (모든 창의 메시지를 여기서 처리)
    // [수정: 게임 루프]
    MSG msg = { };
    DWORD lastTime = GetTickCount(); // 현재 시간 저장

    while (true) {
        // 1. 메시지 처리 (있으면 처리하고, 없으면 다음으로 넘어감)
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // 2. 메시지가 없을 때 -> 여기서 게임 로직을 돌림 (무한 반복)
            DWORD currentTime = GetTickCount();
            
            // 16ms(60FPS)가 지났는지 확인
            if (currentTime - lastTime >= 16) {
                lastTime = currentTime; // 시간 갱신
                
                // ★ 모든 고양이 업데이트 (Update 함수 별도 분리 필요)
                for (Cat* cat : cats) {
                    cat->Update(); // 물리, AI 등 계산
                }
                
                // ★ 모든 고양이 그리기 요청 (Render)
                // (주의: 여기서 InvalidateRect만 호출하고 실제 그리기는 WM_PAINT에서 함)
                for (Cat* cat : cats) {
                    if (cat->hwnd) InvalidateRect(cat->hwnd, NULL, FALSE);
                }
            }
            else {
                // 시간이 안 됐으면 CPU 쉬게 해줌 (필수! 안 하면 CPU 100% 찍음)
                Sleep(1); 
            }
        }
    }

    // 6. 종료 정리 (모든 고양이 메모리 해제)
    for (Cat* c : cats) {
        if (c->myImage) delete c->myImage;
        if (c->myStream) c->myStream->Release();
        delete c;
    }
    cats.clear();

    GdiplusShutdown(gdiplusToken);
    return 0;
}

