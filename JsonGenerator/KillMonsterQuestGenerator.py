import os
import json
from PyQt5.QtWidgets import (
    QApplication, QWidget, QLabel, QLineEdit, QPushButton,
    QVBoxLayout, QHBoxLayout, QSpinBox, QMessageBox
)

QUEST_HEADER_FILE = "../evergreen_server/KillMonsterQuest.h"
QUEST_CPP_FILE = "../evergreen_server/KillMonsterQuest.cpp"
QUEST_JSON_FILE = "../resource/quest_data/quest_data.json"
ITEM_JSON_PATH = "../resource/json/Item.json"
MONSTER_JSON_PATH = "../resource/json/Monster.json"


class QuestGenerator(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("KillMonsterGenerator")
        self.layout = QVBoxLayout()

        self.class_name_input = QLineEdit()
        self.display_name_input = QLineEdit()
        self.giver_input = QLineEdit()
        self.destination_input = QLineEdit()
        self.monster_inputs = {}
        self.reward_inputs = {}

        self.load_data()
        self.setup_ui()

    def load_data(self):
        with open(ITEM_JSON_PATH, encoding="utf-8") as f:
            self.item_data = json.load(f)
        with open(MONSTER_JSON_PATH, encoding="utf-8") as f:
            self.monster_data = json.load(f)

    def setup_ui(self):
        self.layout.addWidget(QLabel("C++ 퀘스트 클래스 이름:"))
        self.layout.addWidget(self.class_name_input)

        self.layout.addWidget(QLabel("클라이언트 표시용 퀘스트 한글 이름:"))
        self.layout.addWidget(self.display_name_input)

        self.layout.addWidget(QLabel("퀘스트 제공자 (선택):"))
        self.layout.addWidget(self.giver_input)

        self.layout.addWidget(QLabel("퀘스트 목적지 (선택):"))
        self.layout.addWidget(self.destination_input)

        self.layout.addWidget(QLabel("몬스터 처치 수 설정:"))
        for monster in self.monster_data:
            row = QHBoxLayout()
            label = QLabel(monster)
            spin = QSpinBox()
            spin.setRange(0, 999)
            self.monster_inputs[monster] = spin
            row.addWidget(label)
            row.addWidget(spin)
            self.layout.addLayout(row)

        self.layout.addWidget(QLabel("보상 아이템 설정:"))
        for item in self.item_data:
            row = QHBoxLayout()
            label = QLabel(item)
            spin = QSpinBox()
            spin.setRange(0, 999)
            self.reward_inputs[item] = spin
            row.addWidget(label)
            row.addWidget(spin)
            self.layout.addLayout(row)

        self.layout.addWidget(QLabel("보상 골드:"))
        self.gold_input = QSpinBox()
        self.gold_input.setRange(0, 999999)
        self.layout.addWidget(self.gold_input)

        json_button = QPushButton("JSON에 퀘스트 추가")
        json_button.clicked.connect(self.append_to_json)
        self.layout.addWidget(json_button)

        generate_button = QPushButton("C++ 헤더/CPP 생성")
        generate_button.clicked.connect(self.generate_cpp_files)
        self.layout.addWidget(generate_button)

        self.setLayout(self.layout)

    def append_to_json(self):
        class_name = self.class_name_input.text().strip()
        display_name = self.display_name_input.text().strip()
        giver = self.giver_input.text().strip()
        destination = self.destination_input.text().strip()

        if not class_name or not display_name:
            QMessageBox.warning(self, "입력 오류", "클래스 이름과 퀘스트 이름을 모두 입력하세요.")
            return

        monsters = {m: s.value() for m, s in self.monster_inputs.items() if s.value() > 0}
        rewards = {i: s.value() for i, s in self.reward_inputs.items() if s.value() > 0}
        gold = self.gold_input.value()

        quest_id = 0
        if os.path.exists(QUEST_JSON_FILE):
            with open(QUEST_JSON_FILE, encoding="utf-8") as f:
                data = json.load(f)
                if data:
                    quest_id = max(q["id"] for q in data) + 1
        else:
            data = []

        quest_json = {
            "id": quest_id,
            "class_name": class_name,
            "name": display_name,
            "monsters": monsters,
            "rewards": rewards,
            "gold": gold,
            "giver": giver,
            "destination": destination,
            "type": "KillMonster"
        }
        data.append(quest_json)

        os.makedirs(os.path.dirname(QUEST_JSON_FILE), exist_ok=True)
        with open(QUEST_JSON_FILE, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=2)

        QMessageBox.information(self, "성공", "JSON에 퀘스트가 추가되었습니다!")

    def generate_cpp_files(self):
        if not os.path.exists(QUEST_JSON_FILE):
            QMessageBox.warning(self, "오류", "JSON 파일이 존재하지 않습니다.")
            return

        with open(QUEST_JSON_FILE, encoding="utf-8") as f:
            data = json.load(f)

        with open(QUEST_HEADER_FILE, "w", encoding="utf-8") as hfile, \
                open(QUEST_CPP_FILE, "w", encoding="utf-8") as cppfile:

            hfile.write("#pragma once\n#include \"pch.h\"\n#include \"Quest.h\"\n")
            cppfile.write("#include \"KillMonsterQuest.h\"\n")

            for quest in data:
                cname = quest["class_name"]
                midict = quest["monsters"]
                qid = quest["id"]

                hfile.write(f"\nclass {cname} : public Quest {{\n")
                hfile.write(f"public:\n    {cname}() noexcept : Quest({qid}) {{}}\n")
                hfile.write(
                    "public:\n    virtual bool OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept override;\n")
                hfile.write(
                    "    virtual void OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept override;\n")
                for m, c in midict.items():
                    hfile.write(f"    int m_{m}_count = {c};\n")
                hfile.write("};\n")

                cppfile.write(
                    f"\nbool {cname}::OnAchieve( NagiocpX::ContentsEntity* const key_entity, NagiocpX::ContentsEntity* const clear_entity ) noexcept {{\n")
                for m in midict:
                    cppfile.write(
                        f"    if ( key_entity->GetEntityInfo().GetObjectDetailType() == Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_{m.upper()} ) {{\n")
                    cppfile.write(f"        m_{m}_count = std::max( 0, m_{m}_count - 1 );\n")
                    cppfile.write("    }\n")
                conds = " && ".join([f"m_{m}_count == 0" for m in midict])
                cppfile.write(f"    if ( {conds} ) {{\n")
                cppfile.write("        if ( const auto session = clear_entity->GetSession() ) {\n")
                cppfile.write(f"            session->SendAsync( Create_s2c_CLEAR_QUEST( {qid}, false ) );\n")
                cppfile.write("        }\n        return true;\n    }\n")
                cppfile.write("    return false;\n}\n")

                cppfile.write(f"\nvoid {cname}::OnReward( NagiocpX::ContentsEntity* const clear_entity ) noexcept {{\n")
                cppfile.write("    if ( const auto session = clear_entity->GetSession() ) {\n")
                cppfile.write("        Quest::ProcessReward(clear_entity, m_questKey);\n        session->SendAsync( Create_s2c_CLEAR_QUEST( m_questKey, true ) );\n")
                cppfile.write("    }\n}\n")

        QUEST_FACTORY_FILE = "../evergreen_server/Quest.cpp"
        sorted_data = sorted(data, key=lambda q: q["id"])

        with open(QUEST_FACTORY_FILE, "w", encoding="utf-8") as f:
            f.write("#include \"pch.h\"\n")
            f.write("#include \"Quest.h\"\n")
            f.write("#include \"KillMonsterQuest.h\"\n")
            f.write("#include \"Inventory.h\"\n")
            f.write("#include \"CommonQuestTable.h\"\n")
            f.write("#include \"DataRegistry.h\"\n")
            f.write("\n")
            f.write("Quest* const Quest::CreateQuest(const uint64_t quest_id) noexcept\n")
            f.write("{\n")
            f.write("    static const std::function<Quest* (void)> g_quest_list[] =\n")
            f.write("    {\n")
            for q in sorted_data:
                f.write(f"        NagiocpX::xnew<{q['class_name']}>,\n")
            f.write("    };\n\n")
            f.write("    return g_quest_list[quest_id]();\n")
            f.write("}\n")

            f.write(
                "void Quest::ProcessReward(NagiocpX::ContentsEntity* const clear_entity, const uint64_t quest_id) noexcept\n")
            f.write("{\n")
            f.write("    const auto& quest_reward_info = Common::CommonQuestTable::GetCommonQuestInfo(quest_id);\n")
            f.write("    const auto entity_inventory = clear_entity->GetComp<Inventory>();\n")
            f.write("    for (const auto& [item_name, item_id, amount] : quest_reward_info.reward_info)\n")
            f.write("    {\n")
            f.write("        entity_inventory->AddItem(item_id, amount);\n")
            f.write("    }\n")
            f.write("}\n")


        QMessageBox.information(self, "성공", "헤더, CPP, 팩토리 파일이 생성되었습니다!")


# 앱 실행
app = QApplication([])
window = QuestGenerator()
window.show()
app.exec_()
