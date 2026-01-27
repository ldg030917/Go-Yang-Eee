// fishing_rod.h
#pragma once
#include "config.h"
#include <vector>
#include "cat.h"

struct VerletPoint {
    float x, y;
    float old_x, old_y;
    float vx, vy; // 속도 (캐싱용)
    
    VerletPoint(float px, float py) : x(px), y(py), old_x(px), old_y(py), vx(0), vy(0) {}
    
    void Update(float dt) {
        // Verlet Integration 핵심: 위치를 속도로 환산하여 물리 계산
        vx = (x - old_x) * 0.99f; // 공기저항
        vy = (y - old_y) * 0.99f;
        
        old_x = x;
        old_y = y;
        x += vx;
        y += vy;
    }
    
    void AddForce(float fx, float fy) {
        x += fx;
        y += fy;
    }
};

class FishingRod {
private:
    std::vector<VerletPoint> points; // 낚싯대 조각들 (세그먼트)
    VerletPoint* toy; // 장난감 물체 (끝에 달린 것)
    
    float length; // 한 세그먼트 길이
    int num_segments; // 낚싯대 세그먼트 수

public:
    FishingRod(int segments = 10, float seg_length = 25.0f);
    void Update(float dt);
    void SetMouseTarget(float mx, float my); // UI에서 마우스 좌표 전달
    void Render(Graphics& g); // 낚싯대와 장난감 그리기
    bool IsToyNear(Cat* cat); // 고양이가 장난감 근처에 있는지 체크
    POINT GetToyPosition() { 
        POINT pt;
        pt.x = toy->x;
        pt.y = toy->y;
        return pt;
    }
};