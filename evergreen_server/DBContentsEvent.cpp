#include "pch.h"
#include "DBContentsEvent.h"
#include "Inventory.h"
#include "EquipmentSystem.h"

void DB_PlayerLogin::ExecuteQuery() noexcept
{
    // Step 1: 유저 존재 여부 확인
    int32 count = 0;
    
   
    dbCheck.BindParam(0, m_name);
    dbCheck.BindCol(0, count);

    const auto b1 = dbCheck.Execute();

    while (dbCheck.Fetch());

    UnBind();
    if (0 == count)
    {
        m_result = LoginResult::NONE;
        //const auto type = Common::DataRegistry::Wstr2Str(m_class_type);
        //const auto init_weapon = GET_DATA(std::string, "Player", type, "InitWeaponKey");
        //m_weapon_id = DATA_TABLE->GetWeaponIDInt(init_weapon);
        //const auto init_armor = GET_DATA(std::string, "Player", type, "InitArmorKey");
        //m_armor_id = DATA_TABLE->GetArmorIDInt(init_armor);
        //
        //// Step 2: 신규 유저 등록
        //dbInsertUser.BindParam(0, m_name);
        //dbInsertUser.BindParam(1, m_pw);
        //dbInsertUser.BindParam(2, m_class_type);
        //
        //dbInsertUser.BindParam(3, m_weapon_id);
        //dbInsertUser.BindParam(4, m_armor_id);
        //
        //
        //const auto v2 = dbInsertUser.Execute();
        //
        //dbSelectUID.BindParam(0, m_name);
        //dbSelectUID.BindCol(0, m_db_uid);
        //const auto v3 = dbSelectUID.Execute();
        //
        //while (dbSelectUID.Fetch());
        //UnBind();
        //
        //if (0 != m_db_uid && LoginResult::NEW_USER_CREATED == m_result)
        //{
        //    m_result = LoginResult::SUCCESS;
        //}
        //else if (0 != m_db_uid)
        //{
        //    m_result = LoginResult::NEW_USER_CREATED;
        //}
        //else
        //{
        //    m_result = LoginResult::NONE;
        //}
    }
    else
    {
        // Step 3: 비밀번호 확인
        wchar_t storedPw[64] = {};
        dbPwCheck.BindParam(0, m_name);
        dbPwCheck.BindCol(0, storedPw);
        dbPwCheck.Execute();
    
        
        dbPwCheck.Fetch();
        UnBind();

        if (m_pw == storedPw)
        {
            dbSelectUID.BindParam(0, m_name);
            dbSelectUID.BindCol(0, m_db_uid);
            dbSelectUID.BindCol(1, m_class_type);
            const auto v3 = dbSelectUID.Execute();
            while (dbSelectUID.Fetch());
            if (0 != m_db_uid)
            {
                UnBind();
                dbSelectItems.BindParam(0, m_db_uid);
                int itemID = 0;
                int count = 0; 
                dbSelectItems.BindCol(0, itemID);
                dbSelectItems.BindCol(1, count);
                dbSelectItems.Execute();
                while (dbSelectItems.Fetch())
                {
                    m_item_ids.emplace_back(itemID);
                    m_item_counts.emplace_back(count);
                }
            }

            UnBind();
            dbSelectEquip.BindParam(0, m_db_uid);

            int weaponID = -1;
            int armorID = -1;

            dbSelectEquip.BindCol(0, weaponID);
            dbSelectEquip.BindCol(1, armorID);

            if (dbSelectEquip.Execute() && dbSelectEquip.Fetch())
            {
                m_weapon_id = weaponID;
                m_armor_id = armorID;
            }

            m_result = LoginResult::SUCCESS;
        }
        else
        {
            m_result = LoginResult::INVALID_PASSWORD;
        }
    }
}

void DB_PlayerLogin::Dispatch(NagiocpX::IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
{
    const S_ptr<DB_PlayerLogin> db_event{ iocpEvent_->PassIocpObject() };
    db_event->GetClientSession()->m_db_uid = m_db_uid;
    Nagox::Enum::LOGIN_RESULT res = Nagox::Enum::LOGIN_RESULT::LOGIN_RESULT_NONE;
    const auto owner_entity = db_event->GetClientSession()->GetOwnerEntity();
    XString type = "";
    switch (m_result)
    {
    case LoginResult::NONE:
    {
        // 없는 회원임
    }
        break;
    
    case LoginResult::SUCCESS:
    {
        res = Nagox::Enum::LOGIN_RESULT::LOGIN_RESULT_SUCCESS;
        const auto inventory = owner_entity->GetComp<Inventory>();
        const auto num = (int)m_item_ids.size();
        for (int i = 0; i < num; ++i)
        {
            inventory->AddItem(m_item_ids[i], m_item_counts[i], false);
        }

        const auto equip_sys = inventory->GetEquipmentSystem();
        type = Common::DataRegistry::Wstr2Str(m_class_type);
      
        if (-1 == m_weapon_id)
        {
            const auto init_weapon = GET_DATA(std::string, "Player", type, "InitWeaponKey");
            m_weapon_id = DATA_TABLE->GetWeaponIDInt(init_weapon);
        }
        if (-1 == m_armor_id)
        {
            const auto init_armor = GET_DATA(std::string, "Player", type, "InitArmorKey");
            m_armor_id = DATA_TABLE->GetArmorIDInt(init_armor);
        }
        equip_sys->SwapEquipment(owner_entity, Nagox::Enum::EQUIPMENT_TYPE_WEAPON, m_weapon_id, false);
        equip_sys->SwapEquipment(owner_entity, Nagox::Enum::EQUIPMENT_TYPE_ARMOR, m_armor_id, false);
    }
        break;
    case LoginResult::INVALID_PASSWORD:
    {
        res = Nagox::Enum::LOGIN_RESULT::LOGIN_RESULT_FAIL;


    }
        break;
    default:
        break;
    }

    owner_entity->GetSession()->SendAsync(
        Create_s2c_LOGIN(
            owner_entity->GetObjectID(),
            Mgr(TimeMgr)->GetServerTimeStamp(),
            res,
            std::move(m_item_ids),
            std::move(m_item_counts),
            m_weapon_id,
            m_armor_id,
            type
        )
    );
}

void DB_NewPlayerInsert::ExecuteQuery() noexcept
{
    // Step 1: 유저 존재 여부 확인
    int32 count = 0;


    dbCheck.BindParam(0, m_name);
    dbCheck.BindCol(0, count);

    const auto b1 = dbCheck.Execute();

    while (dbCheck.Fetch());

    UnBind();

   
    if (0 == count)
    {
        // 같은 아이디의 유저가 없으니 새 계정 생성 시도
       
        const auto type = Common::DataRegistry::Wstr2Str(m_class_type);
        const auto init_weapon = GET_DATA(std::string, "Player", type, "InitWeaponKey");
        m_weapon_id = DATA_TABLE->GetWeaponIDInt(init_weapon);
        const auto init_armor = GET_DATA(std::string, "Player", type, "InitArmorKey");
        m_armor_id = DATA_TABLE->GetArmorIDInt(init_armor);
        
        // Step 2: 신규 유저 등록
        dbInsertUser.BindParam(0, m_name);
        dbInsertUser.BindParam(1, m_pw);
        dbInsertUser.BindParam(2, m_class_type);
        
        dbInsertUser.BindParam(3, m_weapon_id);
        dbInsertUser.BindParam(4, m_armor_id);
        
        
        const auto v2 = dbInsertUser.Execute();
        
        dbSelectUID.BindParam(0, m_name);
        dbSelectUID.BindCol(0, m_db_uid);
        const auto v3 = dbSelectUID.Execute();
        
        while (dbSelectUID.Fetch());
        UnBind();
        
        if (0 != m_db_uid)
        {
            m_result = RegisterResult::SUCCESS;
        }
        else
        {
            m_result = RegisterResult::NONE;
        }
    }
    else
    {
        // 중복된 아이디의 유저가 존재한다. 실패사실을 알린다.
        m_result = RegisterResult::DUPLICATE;
    }
}

void DB_NewPlayerInsert::Dispatch(NagiocpX::IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
{
    const S_ptr<DB_NewPlayerInsert> db_event{ iocpEvent_->PassIocpObject() };
    db_event->GetClientSession()->m_db_uid = m_db_uid;
    Nagox::Enum::LOGIN_RESULT res = Nagox::Enum::LOGIN_RESULT::LOGIN_RESULT_NONE;
    const auto owner_entity = db_event->GetClientSession()->GetOwnerEntity();
    XString type = "";
    switch (m_result)
    {
    case DB_NewPlayerInsert::RegisterResult::NONE:
    {
        res = Nagox::Enum::LOGIN_RESULT::LOGIN_RESULT_FAIL;
    }
        break;
    case DB_NewPlayerInsert::RegisterResult::DUPLICATE:
    {
        res = Nagox::Enum::LOGIN_RESULT_DUPLICATE;
    }
        break;
    case DB_NewPlayerInsert::RegisterResult::SUCCESS:
    {
        const auto inventory = owner_entity->GetComp<Inventory>();
        const auto equip_sys = inventory->GetEquipmentSystem();
        type = Common::DataRegistry::Wstr2Str(m_class_type);

        const auto init_weapon = GET_DATA(std::string, "Player", type, "InitWeaponKey");
        m_weapon_id = DATA_TABLE->GetWeaponIDInt(init_weapon);
        const auto init_armor = GET_DATA(std::string, "Player", type, "InitArmorKey");
        m_armor_id = DATA_TABLE->GetArmorIDInt(init_armor);
        equip_sys->SwapEquipment(owner_entity, Nagox::Enum::EQUIPMENT_TYPE_WEAPON, m_weapon_id, false);
        equip_sys->SwapEquipment(owner_entity, Nagox::Enum::EQUIPMENT_TYPE_ARMOR, m_armor_id, false);
        res = Nagox::Enum::LOGIN_RESULT::LOGIN_RESULT_SUCCESS;
    }
        break;
    default:
        break;
    }
    owner_entity->GetSession()->SendAsync(
        Create_s2c_LOGIN(
            owner_entity->GetObjectID(),
            Mgr(TimeMgr)->GetServerTimeStamp(),
            res,
            std::move(m_item_ids),
            std::move(m_item_counts),
            m_weapon_id,
            m_armor_id,
            type
        )
    );
}
