#pragma once
#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

// 인벤토리 창의 메시지를 처리할 윈도우 프로시저
LRESULT CALLBACK UIWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 필요하다면 인벤토리 창 등록/생성 함수도 뺄 수 있음
void InitInventoryWindow(HINSTANCE hInstance);

class UIButton {
public:
    int x, y;
    int width = 32;  // 기본 크기 고정!
    int height = 32;
    bool isHovered = false;
    bool isActive = false;  // 각 버튼이 각 고양이를 소환한 상태인지 아닌지 정하는 변수
    int catType;     // 이 버튼이 어떤 고양이를 소환하는지 (예: IDB_CAT_SIAM)
    Image* imgNormal;
    Image* imgHover;

    UIButton(int _x, int _y, int _catType, Image* _normal, Image* _hover) 
        : x(_x), y(_y), catType(_catType), imgNormal(_normal), imgHover(_hover) {}

    // 💡 핵심: 마우스 좌표가 버튼 안에 있는지 확인하는 함수
    bool Contains(int mx, int my) {
        return (mx >= x && mx <= x + width && my >= y && my <= y + height);
    }

    // 그리기 함수
    void Render(Graphics& g) {
        if (isHovered && imgHover) {
            g.DrawImage(imgHover, x, y, width, height);
        } else if (imgNormal) {
            g.DrawImage(imgNormal, x, y, width, height);
        }
    }
};