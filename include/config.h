// config.h
#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <cmath>   // sin, cos 등 물리 계산용
#include <stdio.h> // sscanf 등 데이터 처리용

using namespace Gdiplus;

// 선언만 합니다 (메모리 공간을 차지하지 않음)
extern float gravity;
extern int screenW;
extern int screenH;
extern int winW;
extern int winH;

// 내 버전
#define VER_MAJOR 1
#define VER_MINOR 2
#define VER_PATCH 15
// 버전 파일이 있는 URL (Raw 텍스트여야 함)
#define VERSION_URL L"https://gist.githubusercontent.com/ldg030917/f1ba0b5ebcb8c276ddff2b7c6ecbab54/raw/version.txt"
// 다운로드 페이지 URL
#define DOWNLOAD_URL L"https://ldg030917.itch.io/go-yang-eee"
#define WM_TRAYICON (WM_USER + 1) // 트레이 아이콘 메시지 ID
#define ID_EXIT 2001
#define ID_ADD_CAT 2002
#define ID_REMOVE_CAT 2003
#define ID_TOGGLE_FISHING_ROD 2004
#define ID_HOTKEY_ADD 9001
#define ID_HOTKEY_REMOVE 9002
#define ID_HOTKEY_TOGGLE_FISHING_ROD 9003

#define MAIN_WND_CLASS L"Go-Yang-Eee"
const int ID_TRAY_ICON = 1001;    // 트레이 아이콘 식별 번호

// ★ 설정: 본인 아틀라스 이미지에 맞게 수정하세요!
const int FRAME_WIDTH = 32;   // 프레임 1개의 가로 크기
const int FRAME_HEIGHT = 32;  // 프레임 1개의 세로 크기
const int ANIM_SPEED = 96;   // 애니메이션 속도 (ms)
const int PHYSICS_SPEED = 16; // ★ 추가: 물리 갱신 속도 (약 60 FPS)
const float SCALE = 3.0f;   // 세 배로 키우기
const int MOVE_SPEED = 5;
// ★ [수정 1] 목덜미 잡기 좌표 상수 추가
const int NECK_OFFSET_X = (int)((FRAME_WIDTH * SCALE) / 2); // 가로 중심
const int NECK_OFFSET_Y = (int)(16 * SCALE);                // 위에서 10픽셀(스케일 적용) 내려온 곳
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
