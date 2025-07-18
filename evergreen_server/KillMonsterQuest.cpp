#include "KillMonsterQuest.h"

bool KillFoxQuest::OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( key_entity->GetEntityInfo().GetObjectDetailType() == Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_FOX ) {
        m_Fox_count = std::max( 0, m_Fox_count - 1 );
        if (const auto session = clear_entity->GetSession()) {
            session->SendAsync(Create_s2c_PROCESS_COMMON_QUEST(0, (Nagox::Enum::MONSTER_TYPE)key_entity->GetEntityInfo().GetObjectDetailType()));
        }
    }
    if ( m_Fox_count == 0 ) {
        //if ( const auto session = clear_entity->GetSession() ) {
            //session->SendAsync( Create_s2c_CLEAR_QUEST( 0, false ) );
       // }
        return true;
    }
    return false;
}

void KillFoxQuest::OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( const auto session = clear_entity->GetSession() ) {
        Quest::ProcessReward(clear_entity, m_questKey);
        session->SendAsync( Create_s2c_CLEAR_QUEST( m_questKey, true ) );
    }
}

bool KillBearQuest::OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( key_entity->GetEntityInfo().GetObjectDetailType() == Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_BEAR ) {
        m_Bear_count = std::max( 0, m_Bear_count - 1 );
        if (const auto session = clear_entity->GetSession()) {
            session->SendAsync(Create_s2c_PROCESS_COMMON_QUEST(1, (Nagox::Enum::MONSTER_TYPE)key_entity->GetEntityInfo().GetObjectDetailType()));
        }
    }
    if ( m_Bear_count == 0 ) {
        //if ( const auto session = clear_entity->GetSession() ) {
            //session->SendAsync( Create_s2c_CLEAR_QUEST( 1, false ) );
       // }
        return true;
    }
    return false;
}

void KillBearQuest::OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( const auto session = clear_entity->GetSession() ) {
        Quest::ProcessReward(clear_entity, m_questKey);
        session->SendAsync( Create_s2c_CLEAR_QUEST( m_questKey, true ) );
    }
}

bool KillBearFoxQuest::OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( key_entity->GetEntityInfo().GetObjectDetailType() == Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_FOX ) {
        m_Fox_count = std::max( 0, m_Fox_count - 1 );
        if (const auto session = clear_entity->GetSession()) {
            session->SendAsync(Create_s2c_PROCESS_COMMON_QUEST(2, (Nagox::Enum::MONSTER_TYPE)key_entity->GetEntityInfo().GetObjectDetailType()));
        }
    }
    if ( key_entity->GetEntityInfo().GetObjectDetailType() == Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_BEAR ) {
        m_Bear_count = std::max( 0, m_Bear_count - 1 );
        if (const auto session = clear_entity->GetSession()) {
            session->SendAsync(Create_s2c_PROCESS_COMMON_QUEST(2, (Nagox::Enum::MONSTER_TYPE)key_entity->GetEntityInfo().GetObjectDetailType()));
        }
    }
    if ( m_Fox_count == 0 && m_Bear_count == 0 ) {
        //if ( const auto session = clear_entity->GetSession() ) {
            //session->SendAsync( Create_s2c_CLEAR_QUEST( 2, false ) );
       // }
        return true;
    }
    return false;
}

void KillBearFoxQuest::OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( const auto session = clear_entity->GetSession() ) {
        Quest::ProcessReward(clear_entity, m_questKey);
        session->SendAsync( Create_s2c_CLEAR_QUEST( m_questKey, true ) );
    }
}

bool ManyFoxKillQuest::OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( key_entity->GetEntityInfo().GetObjectDetailType() == Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_FOX ) {
        m_Fox_count = std::max( 0, m_Fox_count - 1 );
        if (const auto session = clear_entity->GetSession()) {
            session->SendAsync(Create_s2c_PROCESS_COMMON_QUEST(3, (Nagox::Enum::MONSTER_TYPE)key_entity->GetEntityInfo().GetObjectDetailType()));
        }
    }
    if ( m_Fox_count == 0 ) {
        //if ( const auto session = clear_entity->GetSession() ) {
            //session->SendAsync( Create_s2c_CLEAR_QUEST( 3, false ) );
       // }
        return true;
    }
    return false;
}

void ManyFoxKillQuest::OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept {
    if ( const auto session = clear_entity->GetSession() ) {
        Quest::ProcessReward(clear_entity, m_questKey);
        session->SendAsync( Create_s2c_CLEAR_QUEST( m_questKey, true ) );
    }
}
