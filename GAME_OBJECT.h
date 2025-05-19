#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "GAME_OBJECT.h"

class GameObject
{
private:
	int health;
	int attackPower;
	int defensePower;
	int enemiesNumber;
	int coinsNumber;

public:
	// Default values
	GameObject() : health(0), attackPower(0), defensePower(0), enemiesNumber(0), coinsNumber(0)
	{
	};

	virtual void move() = 0;
	virtual void attack() = 0;
	virtual void defend() = 0;
	virtual void collectItem() = 0;
	virtual void displayInfo() = 0;

	// Setters and getters for GameObject's private members
	void setHealth(int h) { health = h; }
	int getHealth() const { return health; }

	void setAttackPower(int ap) { attackPower = ap; }
	int getAttackPower() const { return attackPower; }

	void setDefensePower(int dp) { defensePower = dp; }
	int getDefensePower() const { return defensePower; }

	void setEnemiesNumber(int en) { enemiesNumber = en; }
	int getEnemiesNumber() const { return enemiesNumber; }

	void setCoinsNumber(int cn) { coinsNumber = cn; }
	int getCoinsNumber() const { return coinsNumber; }



	~GameObject() {};
};

#endif // GAME_OBJECT





