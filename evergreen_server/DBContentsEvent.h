#pragma once
#include "pch.h"
#include "ClientSession.h"

class DB_PlayerLogin :
    public NagiocpX::DBEvent
{
public:
    DB_PlayerLogin(S_ptr<NagiocpX::Session> pPlayerSession)
        : NagiocpX::DBEvent{ std::move(pPlayerSession) }
    {
    }
private:
    virtual void ExecuteQuery() noexcept override;
    virtual void Dispatch(NagiocpX::IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept override;

public:
    enum class LoginResult
    {
        NONE,
        NEW_USER_CREATED,
        SUCCESS,
        INVALID_PASSWORD
    };

    std::wstring m_name;
    std::wstring m_pw;
    LoginResult m_result = LoginResult::NONE;
    int64_t m_db_uid = 0;
    XVector<int> m_item_ids;
    XVector<int> m_item_counts;
    int m_weapon_id = -1;
    int m_armor_id = -1;
    // 유저 존재 여부 확인
    NagiocpX::DBBindRAII<1, 1> dbCheck{ L"SELECT COUNT(*) FROM Users WHERE Name = ?" };

    // 신규 유저 등록
    // Step 1: INSERT (반환 없음)
    NagiocpX::DBBindRAII<5, 0> dbInsertUser{
     L"INSERT INTO Users (Name, Pw, Type, EquippedWeaponID, EquippedArmorID) VALUES (?, ?, ?, ?, ?)"
    };

    NagiocpX::DBBindRAII<1, 1> dbSelectUID{
    L"SELECT UID FROM Users WHERE Name = ?"
    };

    NagiocpX::DBBindRAII<1, 2> dbSelectItems{
     L"SELECT ItemID, Count FROM InventoryItems WHERE CharacterUID = ?"
    };

    // Step 3: 비밀번호 조회
    NagiocpX::DBBindRAII<1, 1> dbPwCheck{ L"SELECT Pw FROM Users WHERE Name = ?" };

    NagiocpX::DBBindRAII<1, 2> dbSelectEquip{
    L"SELECT EquippedWeaponID, EquippedArmorID FROM Users WHERE UID = ?"
    };
};