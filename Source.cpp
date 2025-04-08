#ifdef _WIN32
#include <conio.h>
#elif __linux__
#include <ncurses.h>
#elif __APPLE__
#include <ncurses.h>
#else
#error "Unsupported platform"
#endif




#include <iostream>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstdlib>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define RESET "\033[0m"

using namespace std;

/*
 Створити міні гру в консолі:
Поле 10х10
Зарандомити айтем (коїни: 5, вороги: 5, хп: 3, захист: 3, атака: 3)
Користувач пише в консоль w/a/s/d і цим ходить.
Збір ітемів
Максимально гру зробити на класах.
 */

enum CellType
{
	EMPTY,
	NOT_EMPTY,
	ENEMY,
	HP,
	HP_ITEM,
	COIN,
	PLAYER,
};

unordered_map<string, string> emoji = {
	{"player", "🧍"},
	{"coin", "🪙"},
	{"enemy", "👾"},
	{"hp", "❤️"},
	{"shield", "🛡️"}
};


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
	{};

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



	~GameObject() {};
};



 // Клас для представлення ігрового поля

class Field : public GameObject
{
private:
	vector<vector<CellType>> field;
	int fieldWidth;
	int fieldHeight;

public:
	Field(int fw, int fh) : fieldWidth(fw), fieldHeight(fh)
	{
		for (int i = 0; i < fieldHeight; ++i)
		{
			vector<CellType> row(fieldWidth, EMPTY);
			field.push_back(row);
		}
		cout << "Field created with size: " << fieldWidth << "x" << fieldHeight << endl;
	};

	void addElemnt(int x, int y, CellType type)
	{
		if (x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight)
		{
			field[y][x] = type;
		}
		else
		{
			cout << "Invalid position!" << endl;
		}
	}

	// Getter and setter for fieldWidth and fieldHeight
	int getFieldWidth() const { return fieldWidth; }
	int getFieldHeight() const { return fieldHeight; }
	void setFieldWidth(int fw) { fieldWidth = fw; }
	void setFieldHeight(int fh) { fieldHeight = fh; }

	void outputField() {
		for (int i = 0; i < fieldHeight; ++i) {
			for (int j = 0; j < fieldWidth; ++j) {
				switch (field[i][j]) {
				case EMPTY:
					cout << WHITE << "[" << "  " << "] " << RESET; // Порожня клітинка
					break;
				case ENEMY:
					cout << RED  << "[" << emoji["enemy"] << "] " << RESET; // Ворог
					break;
				case HP:
					cout << GREEN << "[" << emoji["hp"]  << "] " << RESET; // HP
					break;
				case HP_ITEM:
					cout << GREEN << "[" << emoji["shield"] << "] " << RESET; // HP item
					break;
				case COIN:
					cout << YELLOW << "[" << emoji["coin"] << "] " << RESET; // Монета
					break;
				case PLAYER:
					cout << CYAN << "[" << emoji["player"] << "] " << RESET; // Гравець
					break;
				default:
					break;
				}
			}
			cout << endl;
		}
		cout << endl;
	}

	CellType getCell(int x, int y) const {
		if (x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight) {
			return field[y][x]; 
		}
		return NOT_EMPTY; 
	}

	void move() override {
		cout << "Moving on the field." << endl;
	}

	void attack() override {
		cout << "Attacking from the field." << endl;
	}

	void defend() override {
		cout << "Defending on the field." << endl;
	}

	void collectItem() override {
		cout << "Collecting item on the field." << endl;
	}

	void displayInfo() override {
		cout << "Field size: " << fieldWidth << "x" << fieldHeight << endl;
	}


	~Field() {};

};

class Player : public Field
{
public:

	Player(int h, int ap, int dp, int fw, int fh)
		: Field(fw, fh)  
	{
		setHealth(h);
		setAttackPower(ap);
		setDefensePower(dp);
		cout << "Player created with health: " << h << ", attack power: " << ap << ", defense power: " << dp << endl;
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
		if (pressed == 'w' || pressed == 'a' || pressed == 's' || pressed == 'd')
		{
			return true;
		}
		else if (pressed == 'W' || pressed == 'A' || pressed == 'S' || pressed == 'D')
		{
			return true;
		}
		else if (pressed == 27) // ESC key
		{
			cout << "Exiting game." << endl;
			return false;
		}
		else
		{
			cout << endl << RED << "Invalid key pressed. Please use w/a/s/d. esc - exit game" << RESET<< endl;
			return false;
		}
	}


	void playerMove(int& fieldPosX, int& fieldPosY, bool& eventToContinue, Field* gameField)
	{
		cout << "To move:\n  w - move up\n  s - move down\n  a - move left\n  d - move right" << endl;
		cout << "  Press ESC to exit the game." << endl;
		cout << "Press key: ";
		char pressedKey;
		do {
			pressedKey = this->getKeyPress();  // Перевірка натискання клавіші
		} while (pressedKey == '\0');
		

		if (pressedKey == 27) // ESC key
		{
			cout << "Exiting game..." << endl;
			eventToContinue = false;
			return;
		}

		if (validKeyHasPressed(pressedKey))
		{
			if (pressedKey == 'w' || pressedKey == 'W')
			{
				if (fieldPosY > 0)
				{
					gameField->addElemnt(fieldPosX, fieldPosY - 1, PLAYER);
					gameField->addElemnt(fieldPosX, fieldPosY, EMPTY);
					fieldPosY--;
				}
			}
			else if (pressedKey == 's' || pressedKey == 'S')
			{
				if (fieldPosY < gameField->getFieldHeight() - 1)
				{
					gameField->addElemnt(fieldPosX, fieldPosY + 1, PLAYER);
					gameField->addElemnt(fieldPosX, fieldPosY, EMPTY);
					fieldPosY++;
				}
			}
			else if (pressedKey == 'a' || pressedKey == 'A')
			{
				if (fieldPosX > 0)
				{
					gameField->addElemnt(fieldPosX - 1, fieldPosY, PLAYER);
					gameField->addElemnt(fieldPosX, fieldPosY, EMPTY);
					fieldPosX--;
				}
			}
			else if (pressedKey == 'd' || pressedKey == 'D')
			{
				if (fieldPosX < gameField->getFieldWidth() - 1)
				{
					gameField->addElemnt(fieldPosX + 1, fieldPosY, PLAYER);
					gameField->addElemnt(fieldPosX, fieldPosY, EMPTY);
					fieldPosX++;
				}
			}
			
		}

	}



	~Player()
	{
		cout << "Player destroyed." << endl;
	}
};





#pragma region "Functions Prototypes"

pair<int, int> generateRandomPosition(Field* currentField);

void clearConsole();

void showMenu();

#pragma endregion





// Головна функція

int main() {
	system("chcp 65001 > nul");
	enum GameProperties {
		FIELD_HEIGHT = 10,
		FIELD_WIDTH = 10,
		PLAYER_HEALTH = 3,
		PLAYER_ATTACK = 3,
		PLAYER_DEFENSE = 3,
		NUMBER_OF_ENEMIES = 5,
	};
	enum GameItems {
		COIN = 1,
		ENEMY_NUMBER = 2,
		HP = 3,
		HP_ITEM = 4,
	};

	srand(static_cast<unsigned int>(time(0))); // Ініціалізація генератора випадкових чисел

	showMenu();

	// Game field creating
	Field* field = new Field(FIELD_WIDTH, FIELD_HEIGHT);
	// Player creating
	Player* player = new Player(PLAYER_HEALTH, PLAYER_ATTACK, PLAYER_DEFENSE, FIELD_WIDTH, FIELD_HEIGHT);

	cout << endl << endl << MAGENTA << "Game has already been rendered successfully!" << RESET << endl;
	cout << endl << endl;
	cout << BLUE << "Press any key to start the game..." << RESET << endl;

	_getch(); // Wait for user input

	/*field->addElemnt(3, 3, ENEMY);*/

	clearConsole();


	bool gameIsOn = true;
	cout << endl << MAGENTA << "Game started!" << RESET << endl << endl;

	// Generate field and field items

	auto XY_PositionsForUser = generateRandomPosition(field);
	field->addElemnt(XY_PositionsForUser.first, XY_PositionsForUser.second, PLAYER);

	field->outputField();

	while (gameIsOn) {
		player->playerMove(XY_PositionsForUser.first, XY_PositionsForUser.second, gameIsOn, field);
		if (gameIsOn == false)
		{
			clearConsole();
			cout << MAGENTA << "Game over!" << endl << GREEN << "You successfully quited the game" << RESET << endl;
			break;
		}
		clearConsole();
		field->outputField();
		//char pressedKeyInGame = _getch();
		
	}
	

	return 0;
}

pair<int, int> generateRandomPosition(Field* currentField) {
	int x, y;
	do {
		x = rand() % 10;
		y = rand() % 10;
	} while (currentField->getCell(x, y) != EMPTY);

	return make_pair(x, y);
}

void clearConsole()
{
#ifdef _WIN32
	system("cls");
#elif __linux__ || __APPLE__
	system("clear");
#else
#error "Unsupported platform"
#endif
	//system("clear");
}

void showLanguageWarning() {
	cout << RED << "⚠️  Please make sure you're using the Latin keyboard layout!" << endl;
	cout << "⚠️  Only Latin letters are accepted (W, A, S, D or ESC)." << RESET << endl;
}


void showMenu()
{
	cout << BLUE << "\t\t\tSHORT INTRODUCTION TO GAME" << RESET << endl;
	cout << endl;
	cout << GREEN << "\t\t\tWelcome to the game!" << RESET << endl << endl;
	cout << "In this game, you will control a player character on a 10x10 grid." << endl;
	cout << "Your goal is to collect coins, defeat enemies, and manage your health." << endl;
	cout << endl;
	cout << "Use the following controls to navigate:" << endl;
	cout << "  w - move up" << endl;
	cout << "  s - move down" << endl;
	cout << "  a - move left" << endl;
	cout << "  d - move right" << endl;
	cout << "  Press ESC to exit the game." << endl;
	cout << endl << endl;
	cout << "Symbols used in the game:" << endl;
	cout << "  Player: 🧍" << endl;
	cout << "  Coin: 🪙" << endl;
	cout << "  Enemy: 👾" << endl;
	cout << "  Health(hp): ❤️" << endl;
	cout << "  Defense(shield): 🛡️  +1 hp" << endl;
	cout << endl << endl;
	showLanguageWarning();
	cout << endl << endl;
}