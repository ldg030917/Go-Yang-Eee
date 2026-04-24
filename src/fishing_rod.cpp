// src/fishing_rod.cpp
#include "fishing_rod.h"

FishingRod::FishingRod(int segments, float seg_length) 
    : length(seg_length), num_segments(segments) {
    
    // 낚싯대 세그먼트와 장난감 생성
    points.reserve(num_segments);
    for (int i = 0; i < num_segments; i++) {
        points.emplace_back(100 + i * seg_length, 100 + i * seg_length / 2);
    }
    toy = std::make_unique<VerletPoint>(points.back().x, points.back().y + seg_length);
}

void FishingRod::Update(float dt) {
    //printf("update rod");

    // points[1]~points[n], toy 중력 적용
    for (int i = 1; i < num_segments; i++) {
        points[i].AddForce(0.0f, gravity);
    }
    toy->AddForce(0.0f, gravity * 5);

    // 2. 모든 점 물리 업데이트
    for (auto& p : points) p.Update(dt);
    toy->Update(dt);

    // 3. 제약조건: 낚싯대 세그먼트 길이 유지
    for (int k = 0; k < 10; k++) {
        for (int i = 0; i < num_segments; i++) {
            VerletPoint& p1 = points[i];
            VerletPoint* p2 = (i + 1 < num_segments) ? &points[i + 1] : toy.get();
            
            float dx = p2->x - p1.x;
            float dy = p2->y - p1.y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < 0.001f) continue; // 거리가 0이면 나누기 오류 방지
            
            float diff = (dist - length) / dist;
            float offsetX = dx * diff * 0.5f;
            float offsetY = dy * diff * 0.5f;

            // points[0]은 마우스 고정점 — 절대 움직이면 안 됨
            if (i > 0) {
                p1.x += offsetX;
                p1.y += offsetY;
            }
            p2->x -= offsetX;
            p2->y -= offsetY;
        }
    }
    // 4. 장난감 흔들림 추가 (스윙 효과)
    DWORD tick = GetTickCount(); // 한 번만 호출해서 캐싱
    toy->AddForce(sin(tick * 0.003f) * 0.8f, 0.0f);
}

void FishingRod::SetMouseTarget(float mx, float my) {
    // UI에서 전달받은 마우스 위치로 낚싯대 끝 고정
    points[0].x = mx;
    points[0].y = my;
    points[0].old_x = mx;
    points[0].old_y = my;
}

void FishingRod::Render(Graphics& g) {
    Pen rodPen(Color(255, 139, 69, 19), 3.0f); // 갈색 낚싯대
    Pen toyPen(Color(255, 255, 255, 0), 8.0f); // 노란색 장난감

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
    POINT toyPos = GetToyPosition();
    float dx = toyPos.x - cat->posX;
    float dy = toyPos.y - cat->posY;
    return sqrt(dx * dx + dy * dy) < 300.0f;
}
