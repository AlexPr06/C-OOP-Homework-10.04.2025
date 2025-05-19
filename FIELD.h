#pragma once
#include <iostream>
#include <vector>
#include "CELL_TYPE.h"
#include "COLORS.h"
#include "EMOJIS.h"
#include "FUNCTIONS_FOR_TRANSLATION_EMOJIS.h"
#include "json.hpp"

#ifndef FIELD_H
#define FIELD_H

class Field : public GameObject
{
private:
	std::vector<std::vector<CellType>> field;
	int fieldWidth;
	int fieldHeight;

public:
	Field(int fw, int fh) : fieldWidth(fw), fieldHeight(fh)
	{
		for (int i = 0; i < fieldHeight; ++i)
		{
			std::vector<CellType> row(fieldWidth, EMPTY);
			field.push_back(row);
		}
		std::cout << "Field created with size: " << fieldWidth << "x" << fieldHeight << std::endl;
	};

	void addElement(int x, int y, CellType type)
	{
		if (x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight)
		{
			field[y][x] = type;
		}
		else
		{
			std::cout << "Invalid position!" << std::endl;
		}
	}

	// Getter and setter for fieldWidth and fieldHeight
	int getFieldWidth() const { return fieldWidth; }
	int getFieldHeight() const { return fieldHeight; }
	void setFieldWidth(int fw) { fieldWidth = fw; }
	void setFieldHeight(int fh) { fieldHeight = fh; }

	std::vector<std::vector<CellType>> getField() const { return field; }


	void outputField() {
		for (int i = 0; i < fieldHeight; ++i) {
			for (int j = 0; j < fieldWidth; ++j) {
				switch (field[i][j]) {
				case EMPTY:
					std::cout << WHITE << "[" << "  " << "] " << RESET; // Порожня клітинка
					break;
				case ENEMY:
					std::cout << RED << "[" << emoji["enemy"] << "] " << RESET; // Ворог
					break;
				case HP:
					std::cout << GREEN << "[" << emoji["hp"] << "] " << RESET; // HP
					break;
				case HP_ITEM:
					std::cout << GREEN << "[" << emoji["shield"] << "] " << RESET; // HP item
					break;
				case COIN:
					std::cout << YELLOW << "[" << emoji["coin"] << "] " << RESET; // Монета
					break;
				case PLAYER:
					std::cout << CYAN << "[" << emoji["player"] << "] " << RESET; // Гравець
					break;
				default:
					break;
				}
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	CellType getCell(int x, int y) const {
		if (x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight) {
			return field[y][x];
		}
		return NOT_EMPTY;
	}

	void move() override {
		std::cout << "Moving on the field." << std::endl;
	}

	void attack() override {
		std::cout << "Attacking from the field." << std::endl;
	}

	void defend() override {
		std::cout << "Defending on the field." << std::endl;
	}

	void collectItem() override {
		std::cout << "Collecting item on the field." << std::endl;
	}

	void displayInfo() override {
		std::cout << "Field size: " << fieldWidth << "x" << fieldHeight << std::endl;
	}

	nlohmann::json to_json() const
	{
		nlohmann::json j;
		j["fieldWidth"] = fieldWidth;
		j["fieldHeight"] = fieldHeight;
		nlohmann::json fieldJson = nlohmann::json::array();
		for (const auto& row : field) {
			nlohmann::json rowJson = nlohmann::json::array();
			for (const auto& cell : row) {
				rowJson.push_back(cellTypeToString(cell));
			}
			fieldJson.push_back(rowJson);
		}
		j["field"] = fieldJson;
		return j;
	}

	~Field() {};
};


#endif // FIELD_H