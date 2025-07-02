#pragma once
#include "pch.h"
#include "Quest.h"

class KillFoxQuest : public Quest {
public:
    KillFoxQuest() noexcept : Quest(0) {}
public:
    virtual bool OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    virtual void OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    int m_Fox_count = 1;
};

class KillBearQuest : public Quest {
public:
    KillBearQuest() noexcept : Quest(1) {}
public:
    virtual bool OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    virtual void OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    int m_Bear_count = 1;
};

class KillBearFoxQuest : public Quest {
public:
    KillBearFoxQuest() noexcept : Quest(2) {}
public:
    virtual bool OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    virtual void OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    int m_Fox_count = 1;
    int m_Bear_count = 1;
};

class ManyFoxKillQuest : public Quest {
public:
    ManyFoxKillQuest() noexcept : Quest(3) {}
public:
    virtual bool OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    virtual void OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept override;
    int m_Fox_count = 5;
};
