import os
import json
from PyQt5.QtWidgets import (
    QApplication, QWidget, QLabel, QLineEdit, QPushButton,
    QVBoxLayout, QHBoxLayout, QSpinBox, QComboBox, QMessageBox
)

ITEM_JSON_PATH = "../resource/json/Item.json"
RECIPE_JSON_PATH = "../resource/json/ItemRecipe.json"

class RecipeEditor(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Recipe Editor")
        self.layout = QVBoxLayout()

        self.material_inputs = {}
        self.load_items()
        self.setup_ui()
        self.setLayout(self.layout)

    def load_items(self):
        with open(ITEM_JSON_PATH, encoding="utf-8") as f:
            self.item_data = json.load(f)
        self.item_names = list(self.item_data.keys())

    def setup_ui(self):
        self.layout.addWidget(QLabel("재료 아이템 설정:"))
        for item in self.item_names:
            row = QHBoxLayout()
            label = QLabel(item)
            spin = QSpinBox()
            spin.setRange(0, 999)
            self.material_inputs[item] = spin
            row.addWidget(label)
            row.addWidget(spin)
            self.layout.addLayout(row)

        self.layout.addWidget(QLabel("결과 아이템 선택:"))
        result_row = QHBoxLayout()
        self.result_item_combo = QComboBox()
        self.result_item_combo.addItems(self.item_names)
        result_row.addWidget(self.result_item_combo)

        self.result_count_spin = QSpinBox()
        self.result_count_spin.setRange(1, 999)
        result_row.addWidget(QLabel("개수:"))
        result_row.addWidget(self.result_count_spin)

        self.layout.addLayout(result_row)

        save_button = QPushButton("레시피 저장")
        save_button.clicked.connect(self.save_recipe)
        self.layout.addWidget(save_button)

    def save_recipe(self):
        materials = {
            name: spin.value()
            for name, spin in self.material_inputs.items()
            if spin.value() > 0
        }
        if not materials:
            QMessageBox.warning(self, "경고", "적어도 하나 이상의 재료를 선택해야 합니다.")
            return

        result_item = self.result_item_combo.currentText()
        result_count = self.result_count_spin.value()

        # 기존 레시피 불러오기
        recipe_data = {}
        if os.path.exists(RECIPE_JSON_PATH):
            with open(RECIPE_JSON_PATH, encoding="utf-8") as f:
                recipe_data = json.load(f)

        # 가장 큰 인덱스를 찾아 다음 번호 지정
        existing_indices = [
            int(k.split("_")[1]) for k in recipe_data.keys()
            if k.startswith("Recipe_") and k.split("_")[1].isdigit()
        ]
        next_index = max(existing_indices, default=-1) + 1
        recipe_id = f"Recipe_{next_index}"

        # 레시피 구성 및 저장
        recipe_data[recipe_id] = materials
        recipe_data[recipe_id]["Result Item"] = result_item
        recipe_data[recipe_id]["Result Num Of Item"] = result_count

        os.makedirs(os.path.dirname(RECIPE_JSON_PATH), exist_ok=True)
        with open(RECIPE_JSON_PATH, "w", encoding="utf-8") as f:
            json.dump(recipe_data, f, ensure_ascii=False, indent=2)

        QMessageBox.information(self, "성공", f"{recipe_id} 레시피가 저장되었습니다!")

# 실행
app = QApplication([])
window = RecipeEditor()
window.show()
app.exec_()
