// main.cpp
#include <time.h> // srand 초기화용
#include <shlwapi.h> // IStream 변환용
#include <shellapi.h> // Shell_NotifyIcon 용 (트레이 아이콘)
#include <wininet.h> // 업데이트 체크용 인터넷 라이브러리
#include "config.h"
#include "cat_states.h"
#include "game_manager.h"
#include "inventory_ui.h"

#include <iostream>


#pragma comment(lib, "wininet.lib")

//taskkill /IM Go-Yang-Eee.exe /F

using namespace Gdiplus;

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

// 전역 변수
const wchar_t CLASS_NAME[] = L"Go-Yang-Eee";

// 버전 체크 함수
void CheckForUpdate(HWND hwnd) {
    HINTERNET hInternet, hFile;
    char buffer[1024];
    DWORD bytesRead;

    // 1. 인터넷 연결 초기화
    hInternet = InternetOpenW(L"MyPetUpdater", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return; // 인터넷 안 되면 조용히 넘어감

    // 2. 버전 파일 읽기
    hFile = InternetOpenUrlW(hInternet, VERSION_URL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hFile) {
        if (InternetReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead)) {
            buffer[bytesRead] = '\0'; // 문자열 끝 처리
            
            // 줄바꿈 문자 제거 (혹시 모르니)
            char* p = strchr(buffer, '\n'); if(p) *p = 0;
            p = strchr(buffer, '\r'); if(p) *p = 0;

            int sMajor=0, sMinor=0, sPatch=0;
            // %d.%d.%d 패턴으로 숫자 3개 추출 (실패 시 0)
            sscanf(buffer, "%d.%d.%d", &sMajor, &sMinor, &sPatch);

            // 버전 비교 (앞에서부터 차례대로)
            bool update = false;
            if (sMajor > VER_MAJOR) update = true;
            else if (sMajor == VER_MAJOR && sMinor > VER_MINOR) update = true;
            else if (sMajor == VER_MAJOR && sMinor == VER_MINOR && sPatch > VER_PATCH) update = true;

            if (update) {
                // 업데이트 발견!
                WCHAR msg[256];
                wsprintfW(msg, L"새로운 버전(%S)이 있습니다!\n지금 다운로드 하시겠습니까?", buffer);
                
                if (MessageBoxW(hwnd, msg, L"업데이트 알림", MB_YESNO | MB_ICONASTERISK) == IDYES) {
                    // 웹브라우저 열기
                    ShellExecuteW(NULL, L"open", DOWNLOAD_URL, NULL, NULL, SW_SHOWNORMAL);
                }
            }
        }
        InternetCloseHandle(hFile);
    }
    InternetCloseHandle(hInternet);
}

std::vector<Cat*> cats;
FishingRod* fishingRod = nullptr; // 낚싯대 포인터
bool fishingRodActive = false;    // 활성화 여부

void ToggleFishingRod() {
    auto& gm = GameManager::get();
    if (gm.fishingRodActive) {
        delete gm.fishingRod;
        gm.fishingRod = nullptr;
        gm.fishingRodActive = false;
        printf("낚싯대 OFF\n");
    } else {
        gm.fishingRod = new FishingRod(12, 20.0f);
        gm.fishingRodActive = true;
        printf("낚싯대 ON (Ctrl+Alt+F)\n");
    }
}

// 두 고양이 사이의 거리(픽셀) 반환
float GetDistance(Cat* a, Cat* b) {
    float dx = (float)(a->posX - b->posX);
    float dy = (float)(a->posY - b->posY);
    return sqrt(dx*dx + dy*dy);
}

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
    lstrcpyW(nid.szTip, L"Go-Yang-Eee");

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
            RegisterHotKey(hwnd, ID_HOTKEY_TOGGLE_FISHING_ROD, MOD_CONTROL | MOD_ALT, 'F'); // Ctrl+Alt+F
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
        
        auto& gm = GameManager::get();
        if (gm.fishingRodActive && gm.fishingRod) {
            Graphics g(hdc);
            gm.fishingRod->Render(g);
        }
        
        pCat->Render(hdc, w, h);
        
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_LBUTTONDOWN: {
        pCat->isDragging = true;
        pCat->targetSpeedX = 0;
        pCat->SetAction(GRABBED);

        pCat->ChangeState(new GrabbedState());
    
        
        SetWindowPos(hwnd, NULL, pCat->posX, pCat->posY, 0, 0, 
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

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
            // int newX = pt.x - pCat->dragOffset.x;
            // int newY = pt.y - pCat->dragOffset.y;
            
            // pCat->posX = newX;
            // pCat->posY = newY;
            // pCat->speedY = 0.0f;

            // // 던지기 속도 계산
            // pCat->throwSpeedX = (float)(pt.x - pCat->lastCursorX);
            // pCat->throwSpeedY = (float)(pt.y - pCat->lastCursorY);
            // // 좌표가 화면 밖으로 튀면 소리 재생 + 복구
            // if (newX < -200 || newX > 5000) {
            //     MessageBeep(MB_ICONHAND); // 경고음!
            //     pCat->posX = 100; // 강제 복구
            //     pCat->isDragging = false; // 드래그 강제 해제
            // }

            // SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        else {
            // 쓰다듬기 로직 (간소화)
            int dx = abs(pt.x - pCat->lastCursorX);
            int dy = abs(pt.y - pCat->lastCursorY);
            
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
        pCat->lastCursorX = pt.x;
        pCat->lastCursorY = pt.y;
        return 0;
    }

    case WM_LBUTTONUP: {
        if (pCat->isDragging) {
            pCat->isDragging = false;
            ReleaseCapture();
            pCat->ChangeState(new IdleState());

            pCat->speedX = pCat->throwSpeedX;
            pCat->speedY = pCat->throwSpeedY;

            // 너무 빠르면 제한 (선택사항)
            // if (pCat->speedX > 20) pCat->speedX = 20;
            // if (pCat->speedX < -20) pCat->speedX = -20;
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
            PostQuitMessage(0);
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
                cats.pop_back(); // 리스트에서 제거
                // 윈도우 파괴
                if (victim->hwnd) DestroyWindow(victim->hwnd);
                
                // 메모리 정리
                if (victim->myImage) delete victim->myImage;
                if (victim->myStream) victim->myStream->Release();
                delete victim;                
                
                // 0마리 되면 종료할지? (선택사항)
                if (cats.empty()) PostQuitMessage(0);
            }
            break;
        }
        case ID_TOGGLE_FISHING_ROD: {
            auto& gm = GameManager::get();
            gm.toggle_fishing_rod();
            // 토글 추가
            std::cout << "AA" << std::endl;
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
        else if (wParam == ID_HOTKEY_TOGGLE_FISHING_ROD) {
            SendMessage(hwnd, WM_COMMAND, ID_TOGGLE_FISHING_ROD, 0);
        }
        return 0;
    }

    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK RodWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 전체화면 채우기
        RECT rc; GetClientRect(hwnd, &rc);

        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);

        // 낚싯대 그리기
        auto& gm = GameManager::get();
        if (gm.fishingRodActive && gm.fishingRod) {
            Graphics g(hdc);
            gm.fishingRod->Render(g);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;   // 깜빡임 방지
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
    
    // 업데이트 확인
    CheckForUpdate(NULL); 

    RegisterClassExW(&wc);

    // 3. 화면 크기 및 창 크기 계산
    screenW = GetSystemMetrics(SM_CXSCREEN);
    screenH = GetSystemMetrics(SM_CYSCREEN);
    winW = (int)(FRAME_WIDTH * SCALE);
    winH = (int)(FRAME_HEIGHT * SCALE);
    POINT pt;
    GetCursorPos(&pt);

    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    // 고양이 추가하기 전에 낚싯대 클래스 등록
    WNDCLASSEXW rodWc = { sizeof(WNDCLASSEXW) };
    rodWc.lpfnWndProc = RodWindowProc;
    rodWc.hInstance = hInstance;
    rodWc.lpszClassName = L"RodOverlayWindow";
    rodWc.hCursor = LoadCursor(NULL, IDC_ARROW);
    rodWc.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    RegisterClassExW(&rodWc);

    // 낚싯대 윈도우 생성 TODO
    HWND hRodWnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"RodOverlayWindow", L"RodOverlayWindow", WS_POPUP,
        0, 0, screenW, screenH, // 화면 전체 크기
        NULL, NULL, hInstance, NULL
    );
    // 투명화 설정
    SetLayeredWindowAttributes(hRodWnd, RGB(255, 0, 255), 0, LWA_COLORKEY);
    auto& gm = GameManager::get();
    gm.gRodWnd = hRodWnd;
    //ShowWindow(hRodWnd, SW_SHOW);
    //showfishingrod는 toggle에서 추가

    InitInventoryWindow(hInstance);

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
                

                auto& gm = GameManager::get();

                // 낚싯대 마우스 따라다니기
                if (gm.fishingRodActive && gm.fishingRod) {
                    gm.fishingRod->Update(16);
                }

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
    //MessageBoxW(NULL, L"정상 종료됨", L"알림", MB_OK); // 이거 뜨면 정상
    return 0;
}

