#include "inventory_ui.h"
#include "utils.h"
#include "resource.h"
#include <gdiplus.h>

using namespace Gdiplus;

// 이미지를 들고 있을 포인터를 전역 변수(또는 정적 변수)로 선언
// 나중에 UI 클래스를 따로 만들면 멤버 변수로 넣는 게 더 좋음
static Image* bgImage = nullptr;
static IStream* bgStream = nullptr;

LRESULT CALLBACK UIWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // 2. 창이 생성될 때 딱 한 번! 하드디스크에서 이미지를 읽어와서 메모리에 올림.
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        HINSTANCE hInst = pCreate->hInstance;

        // 1. 헬퍼 함수를 써서 배경 이미지 로드! 코드가 딱 한 줄로 끝남.
        bgImage = LoadImageFromResource(hInst, IDB_UI_BACKGROUND, &bgStream);

        // TODO: 나중에 여기에 고양이 버튼 1, 2, 3 이미지도 똑같이 로드하면 됨 
        // 버튼 생성해서 벡터에 넣어줌

        // catBtn1 = LoadImageFromResource(hInst, IDB_CAT_BTN1, &catStream1);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Graphics g(hdc);

        // 1. 인벤토리 창 배경 그리기
        if (bgImage) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            g.DrawImage(bgImage, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
        }

        // 2. 낚싯대 토글 버튼 이미지 그리기
        // 3. 보유 중인 32x32 도트 고양이 에셋들을 슬롯에 맞춰 그리기

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_LBUTTONDOWN: {
        // // 마우스 클릭한 x, y 좌표 가져오기
        // int x = GET_X_LPARAM(lParam);
        // int y = GET_Y_LPARAM(lParam);

        // // [충돌 검사] 클릭한 좌표가 어떤 버튼 영역 안에 있는지 수학적으로 계산
        
        // // ex) 낚싯대 버튼 영역(x: 10~110, y: 10~50)을 클릭했다면?
        // if (x >= 10 && x <= 110 && y >= 10 && y <= 50) {
        //     auto& gm = GameManager::get();
        //     gm.toggle_fishing_rod();
        //     InvalidateRect(hwnd, NULL, FALSE); // 버튼 상태(ON/OFF) 갱신을 위해 다시 그리기
        // }
        
        // // ex) 1번 고양이 슬롯을 클릭했다면?
        // // 메인 윈도우나 게임 로직으로 고양이 스폰 명령을 보냄
        // // SendMessage(메인창HWND, WM_COMMAND, ID_ADD_CAT, 고양이타입);
        // return 0;
    }
    // 창을 마우스로 잡고 이동할 수 있게 하려면 (타이틀바가 없는 커스텀 창인 경우)
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
        if (hit == HTCLIENT) return HTCAPTION; // 창 안쪽을 잡아도 드래그되게 만듦
        return hit;
    }

    case WM_DESTROY:
        // 인벤토리 창이 닫힐 때의 처리 (숨기기만 할지, 프로그램을 끌지 결정)
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitInventoryWindow(HINSTANCE hInstance) {
    // 1. UI 창 클래스 등록
    WNDCLASSEXW uiWc = { sizeof(WNDCLASSEXW) };
    uiWc.lpfnWndProc = UIWindowProc;  // 방금 만든 UI 전용 프로시저 연결!
    uiWc.hInstance = hInstance;
    uiWc.lpszClassName = L"InventoryWindow";
    uiWc.hCursor = LoadCursor(NULL, IDC_ARROW);
    // 윈도우 배경색을 마젠타(RGB 255, 0, 255)로 칠함 (나중에 투명해질 색상)
    uiWc.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    RegisterClassExW(&uiWc);

    // 2. UI 창 생성 및 띄우기
    HWND hInventoryWnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"InventoryWindow", L"고양이 인벤토리", 
        WS_POPUP,
        100, 100, 300, 400,               // 창 위치와 크기 (가로 300, 세로 400)
        NULL, NULL, hInstance, NULL
    );

    // 마젠타(RGB 255, 0, 255) 색상을 완전히 투명하게
    SetLayeredWindowAttributes(hInventoryWnd, RGB(255, 0, 255), 0, LWA_COLORKEY);

    ShowWindow(hInventoryWnd, SW_SHOW);
}