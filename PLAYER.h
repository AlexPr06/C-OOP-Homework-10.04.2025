#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <conio.h>

#include "GAME_OBJECT.h"

#include "FIELD.h"
#include "json.hpp"

std::mutex mtx;
std::atomic<bool> gameIsOn(true);

#ifndef PLAYER_H
#define PLAYER_H

class Player : public Field
{
private:
	std::vector<CellType> userPressedKeyList;
	bool shieldActivated = false;

public:

	Player(int h, int ap, int dp, int fw, int fh)
		: Field(fw, fh)
	{
		setHealth(h);
		setAttackPower(ap);
		setDefensePower(dp);
		std::cout << "Player created with health: " << h << ", attack power: " << ap << ", defense power: " << dp << std::endl;

	}

	// Getters and setters for Player's private members
	void setUserPressedKeyList(CellType pressedKey) { userPressedKeyList.push_back(pressedKey); }
	std::vector<CellType>& getUserPressedKeyList() { return userPressedKeyList; }

	bool getShieldActivated() { return shieldActivated; }
	void setShieldActivated(bool activated) { shieldActivated = activated; }




	void handleCellEvent(int& PosX, int& PosY, char& keyValue, Field* currentField)
	{
		int y = PosY;
		int x = PosX;
		if (keyValue == 'w')
		{
			y--;
		}
		else if (keyValue == 's')
		{
			y++;
		}
		else if (keyValue == 'a')
		{
			x--;
		}
		else if (keyValue == 'd')
		{
			x++;
		}
		else
		{
			std::cout << RED << "Invalid key pressed. Please use w/a/s/d." << RESET << std::endl;
			return;
		}

		// Check if the new position is within bounds
		switch (currentField->getField()[y][x]) {
		case EMPTY:
			break;
		case ENEMY:
			updatePlayerHealth(this);
			break;
		case HP:
			updateHPStatus(this);
			break;
		case HP_ITEM:
			updateDefensePower(this);
			break;
		case COIN:
			updateCoinsStatus(this);
			break;
		default:
			break;
		}
	}

	void updateField(Field* currentField)
	{
		for (int i = 0; i < currentField->getFieldHeight(); ++i)
		{
			for (int j = 0; j < currentField->getFieldWidth(); ++j)
			{
				if (currentField->getField()[i][j] == PLAYER)
				{
					currentField->addElement(j, i, PLAYER);
					currentField->addElement(j, i, EMPTY);
				}
			}
		}
		
	}

	void updatePlayerHealth(Player* player)
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (getDefensePower() >= 3)
		{
			setDefensePower(getDefensePower() - 3);
			setShieldActivated(true);
		}
		else setHealth(getHealth() - 1);
	}

	void updateHPStatus(Player* player) {
		std::lock_guard<std::mutex> lock(mtx);
		setHealth(getHealth() + 1);
	}

	void updateDefensePower(Player* player)
	{
		std::lock_guard<std::mutex> lock(mtx);
		setDefensePower(getDefensePower() + 1);
	}

	void updateCoinsStatus(Player* player)
	{
		std::lock_guard<std::mutex> lock(mtx);
		setCoinsNumber(getCoinsNumber() + 1);
	}

	void writeHandledCellEvent(std::vector<CellType>& listOfTypes)

	{
		if (!listOfTypes.empty())
		{
			switch (listOfTypes[listOfTypes.size() - 1]) {
			case EMPTY:
				std::cout << WHITE << "You moved to an empty cell." << RESET << std::endl;
				break;
			case ENEMY:
				std::cout << RED << "You encountered an enemy!" << RESET << std::endl;
				if (shieldActivated)
				{
					std::cout << GREEN << "You defended yourself with a shield! ( 3 shields are always used to protect your 1 health )" << RESET << std::endl;
					setShieldActivated(false);
				}
				else {
					std::cout << RED << "You lost 1 health! ( Not enough shields. Required 3 at least )" << RESET << std::endl;
				}
				break;
			case HP:
				std::cout << CYAN << "You found a health item!" << RESET << std::endl;
				break;
			case HP_ITEM:
				std::cout << GREEN << "You found a shield item!" << RESET << std::endl;
				break;
			case COIN:
				std::cout << YELLOW << "You found a coin!" << RESET << std::endl;
				break;
			default:
				break;
			}

			if (listOfTypes.size() > 10)
			{
				listOfTypes.erase(listOfTypes.begin(), listOfTypes.begin() + 9);
			}
		}
		else
		{
			std::cout << "You haven't moved yet." << std::endl;
		}
	}

	char getKeyPress()
	{
		char ch = '\0';

#ifdef _WIN32
		if (_kbhit()) {
			ch = _getch();  // Windows
		}
#elif __linux__ || __APPLE__
		initscr();           // Ініціалізація ncurses
		raw();               // Вимикає буферизацію вводу
		timeout(0);          // Функція повертає відразу без блокування
		ch = getch();       // Читання клавіші
		endwin();
#endif
		return ch;
	}

	bool validKeyHasPressed(char& pressed)
	{
		pressed = tolower(pressed);

		if (pressed == 'w' || pressed == 'a' || pressed == 's' || pressed == 'd')
		{
			return true;
		}
		else if (pressed == 27) // ESC key
		{
			std::cout << "Exiting game." << std::endl;
			return false;
		}
		else
		{
			std::cout << std::endl << RED << "Invalid key pressed. Please use w/a/s/d. esc - exit game" << RESET << std::endl;
			return false;
		}
	}


	void playerMove(int& fieldPosX, int& fieldPosY, std::atomic<bool>& eventToContinue, Field* gameField)
	{
		std::cout << "🧍 Player position: (" << fieldPosX << ", " << fieldPosY << ")" << std::endl;
		std::cout << "🗡️ Attack: " << getAttackPower() << " | 🛡️ Defense: " << getDefensePower() << std::endl;
		std::cout << "❤️ Health: " << getHealth() << std::endl;
		std::cout << "👾 Enemies encountered: " << getEnemiesNumber() << std::endl;
		std::cout << "💖 HP items collected: " << getHealth() << std::endl;
		std::cout << "💎 Coins collected: " << getCoinsNumber() << std::endl;
		std::cout << std::endl;

		writeHandledCellEvent(getUserPressedKeyList());

		/* ---------------------------------------------------------------------------- FOR TEST ---------------------------------------------------------------------------------------- */

		/*for (int i = 0; i < getUserPressedKeyList().size(); i++) cout << getUserPressedKeyList()[i] << " ";
		cout << endl;*/

		/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */


		std::cout << std::endl << std::endl;
		std::cout << "To move:\n  w - move up\n  s - move down\n  a - move left\n  d - move right" << std::endl;
		std::cout << "  Press ESC to exit the game." << std::endl;
		std::cout << "Press key: ";

		char pressedKey;
		do {
			pressedKey = this->getKeyPress();  // Перевірка натискання клавіші
		} while (pressedKey == '\0');


		if (pressedKey == 27) // ESC key
		{
			std::cout << "Exiting game..." << std::endl;
			eventToContinue = false;
			return;
		}

		if (validKeyHasPressed(pressedKey))
		{
			if (pressedKey == 'w')
			{
				if (fieldPosY > 0)
				{
					handleCellEvent(fieldPosX, fieldPosY, pressedKey, gameField);
					setUserPressedKeyList(gameField->getField()[fieldPosY - 1][fieldPosX]);
					gameField->addElement(fieldPosX, fieldPosY - 1, PLAYER);
					gameField->addElement(fieldPosX, fieldPosY, EMPTY);
					fieldPosY--;

				}
			}
			else if (pressedKey == 's')
			{
				if (fieldPosY < gameField->getFieldHeight() - 1)
				{
					handleCellEvent(fieldPosX, fieldPosY, pressedKey, gameField);
					setUserPressedKeyList(gameField->getField()[fieldPosY + 1][fieldPosX]);
					gameField->addElement(fieldPosX, fieldPosY + 1, PLAYER);
					gameField->addElement(fieldPosX, fieldPosY, EMPTY);
					fieldPosY++;

				}
			}
			else if (pressedKey == 'a')
			{
				if (fieldPosX > 0)
				{
					handleCellEvent(fieldPosX, fieldPosY, pressedKey, gameField);
					setUserPressedKeyList(gameField->getField()[fieldPosY][fieldPosX - 1]);
					gameField->addElement(fieldPosX - 1, fieldPosY, PLAYER);
					gameField->addElement(fieldPosX, fieldPosY, EMPTY);
					fieldPosX--;
				}
			}
			else if (pressedKey == 'd')
			{
				if (fieldPosX < gameField->getFieldWidth() - 1)
				{
					handleCellEvent(fieldPosX, fieldPosY, pressedKey, gameField);
					setUserPressedKeyList(gameField->getField()[fieldPosY][fieldPosX + 1]);
					gameField->addElement(fieldPosX + 1, fieldPosY, PLAYER);
					gameField->addElement(fieldPosX, fieldPosY, EMPTY);
					fieldPosX++;;
				}
			}

		}

	}




	/*
	 BACKUP with JSON
	*/
	/* -------------------------------------------------------------------------------- */

	nlohmann::json to_json() const
	{
		nlohmann::json j;
		j["health"] = getHealth();
		j["attackPower"] = getAttackPower();
		j["defensePower"] = getDefensePower();
		j["enemiesNumber"] = getEnemiesNumber();
		j["coinsNumber"] = getCoinsNumber();
		return j;
	}

	void from_json(const nlohmann::json& j)
	{
		setHealth(j["health"]);
		setAttackPower(j["attackPower"]);
		setDefensePower(j["defensePower"]);
		setEnemiesNumber(j["enemiesNumber"]);
		setCoinsNumber(j["coinsNumber"]);
	}


	/* -------------------------------------------------------------------------------- */
	~Player()
	{
		std::cout << "Player destroyed." << std::endl;
	}
};


#endif // PLAYER_H
