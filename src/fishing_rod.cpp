// src/fishing_rod.cpp
#include "fishing_rod.h"

FishingRod::FishingRod(int segments, float seg_length) 
    : length(seg_length), num_segments(segments) {
    
    // 낚싯대 세그먼트와 장난감 생성
    points.reserve(num_segments);
    for (int i = 0; i < num_segments; i++) {
        points.emplace_back(100 + i * seg_length, 100 + i * seg_length / 2);
    }
    toy = new VerletPoint(points.back().x, points.back().y + seg_length);
}

void FishingRod::Update(float dt) {
    // 1. 마우스 위치로 첫 번째 점 고정
    SetMouseTarget(0, 0); // UI에서 전달받은 마우스 좌표로 변경

    // 2. 모든 점 물리 업데이트
    for (auto& p : points) p.Update(dt);
    toy->Update(dt);

    // 3. 제약조건: 낚싯대 세그먼트 길이 유지
    for (int i = 0; i < num_segments; i++) {
        VerletPoint& p1 = points[i];
        VerletPoint* p2 = (i + 1 < num_segments) ? &points[i + 1] : toy;
        
        float dx = p2->x - p1.x;
        float dy = p2->y - p1.y;
        float dist = sqrt(dx * dx + dy * dy);
        
        if (dist > length) {
            // 거리가 너무 길면 길이 맞춰서 끌어당김
            float ratio = length / dist;
            p2->x = p1.x + dx * ratio;
            p2->y = p1.y + dy * ratio;
        }
    }

    // 4. 장난감 흔들림 추가 (스윙 효과)
    toy->AddForce(sin(GetTickCount() * 0.01f) * 2.0f, cos(GetTickCount() * 0.01f) * 1.0f);
}

void FishingRod::SetMouseTarget(float mx, float my) {
    // UI에서 전달받은 마우스 위치로 낚싯대 끝 고정
    points[0].x = mx;
    points[0].y = my;
}

void FishingRod::Render(Graphics& g) {
    Pen rodPen(Color(139, 69, 19), 3.0f); // 갈색 낚싯대
    Pen toyPen(Color(255, 255, 0), 8.0f); // 노란색 장난감

    // 낚싯대 선 그리기
    for (int i = 0; i < num_segments; i++) {
        Point p1((int)points[i].x, (int)points[i].y);
        Point p2((int)(i + 1 < num_segments ? points[i + 1].x : toy->x),
                 (int)(i + 1 < num_segments ? points[i + 1].y : toy->y));
        g.DrawLine(&rodPen, p1, p2);
    }

    // 장난감 물체 그리기 (깜빡임 효과)
    if (GetTickCount() % 400 < 200) {
        Point toyPos((int)toy->x, (int)toy->y);
        g.DrawEllipse(&toyPen, toyPos.X - 8, toyPos.Y - 8, 16, 16);
    }
}

bool FishingRod::IsToyNear(Cat* cat) {
    Point toyPos = GetToyPosition();
    float dx = toyPos.X - cat->posX;
    float dy = toyPos.Y - cat->posY;
    return sqrt(dx * dx + dy * dy) < 60.0f; // 고양이 머리 크기
}
