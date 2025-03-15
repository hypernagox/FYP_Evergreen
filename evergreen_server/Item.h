#pragma once

// TODO: 기획따라 만들기
// TODO: 서버와 클라가 공유할 아이템 식별자 필요
// 아이디어: enum 두개 쓰기 / 예) 8바이트중 앞 4바이트는 아이템타입, 뒤4바이트는 그 아이템의 유니크한 식별자
// 아이템 식별자 필요
class Item
{
public:
	virtual bool UseItem()noexcept {}
private:

};

