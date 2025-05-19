#pragma once

#include <iostream>
#include "CELL_TYPE.h"

#ifndef FUNCTIONS_FOR_TRANSLATION_EMOJIS_H
#define FUNCTIONS_FOR_TRANSLATION_EMOJIS_H

std::string cellTypeToString(CellType type) {
	switch (type) {
	case EMPTY: return "EMPTY";
	case NOT_EMPTY: return "NOT_EMPTY";
	case ENEMY: return "ENEMY";
	case HP: return "HP";
	case HP_ITEM: return "HP_ITEM";
	case COIN: return "COIN";
	case PLAYER: return "PLAYER";
	default: return "UNKNOWN";
	}
}

CellType stringToCellType(const std::string& str) {
	if (str == "EMPTY") return EMPTY;
	if (str == "NOT_EMPTY") return NOT_EMPTY;
	if (str == "ENEMY") return ENEMY;
	if (str == "HP") return HP;
	if (str == "HP_ITEM") return HP_ITEM;
	if (str == "COIN") return COIN;
	if (str == "PLAYER") return PLAYER;
	return EMPTY; // FUNCTIONS_FOR_TRANSLATION_EMOJIS_H
}
#endif


