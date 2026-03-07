#pragma once
#include <windows.h>

// 인벤토리 창의 메시지를 처리할 윈도우 프로시저
LRESULT CALLBACK UIWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 필요하다면 인벤토리 창 등록/생성 함수도 뺄 수 있음
void InitInventoryWindow(HINSTANCE hInstance);