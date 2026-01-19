// cat_state.h
#pragma once

class Cat;

class CatState {
private:
    /* data */
public:
    virtual ~CatState() {}
    virtual void Enter(Cat* cat) = 0;   // 상태 진입 시 (한 번)
    virtual void Update(Cat* cat) = 0;  // 매 프레임 실행
    virtual void Exit(Cat* cat) = 0;    // 상태 탈출 시 (한 번)
};