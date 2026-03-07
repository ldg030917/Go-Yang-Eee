// utils.h
#pragma once
#include <windows.h>
#include <gdiplus.h>

// 공통으로 사용할 리소스 이미지 로딩 함수 선언
Gdiplus::Image* LoadImageFromResource(HINSTANCE hInstance, int resourceId, IStream** outStream);