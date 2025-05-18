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
#include <chrono>
#include <thread>
#include <mutex>
//#include <condition_variable>
#include <future>
#include "json.hpp"
#include <fstream>
#include <atomic>
#include <csignal>


#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define RESET "\033[0m"

using namespace std;
using namespace std::this_thread;     // for sleep_for
using namespace std::chrono_literals; // for ms
using json = nlohmann::json;

mutex mtx;
atomic<bool> gameIsOn(true);




/*
Create a mini game in the console:
10x10 field
Randomize items (coins: 5, enemies: 5, hp: 3, defense: 3, attack: 3)
The user writes w/a/s/d into the console and walks with it.
Collecting items
Use classes as much as possible.
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

string cellTypeToString(CellType type) {
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
	return EMPTY; // fallback
}

unordered_map<string, string> emoji = {
	{"player", "🧍"},
	{"coin", "🪙"},
	{"enemy", "👾"},
	{"hp", "❤️"},
	{"shield", "🛡️"}
};



// GameObject class

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

	void setCoinsNumber(int cn) { coinsNumber = cn; }
	int getCoinsNumber() const { return coinsNumber; }



	~GameObject() {};
};



// Game field class

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

	void addElement(int x, int y, CellType type)
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

	vector<vector<CellType>> getField() const { return field; }


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
private:
	vector<CellType> userPressedKeyList;
	bool shieldActivated = false;

public:

	Player(int h, int ap, int dp, int fw, int fh)
		: Field(fw, fh)  
	{
		setHealth(h);
		setAttackPower(ap);
		setDefensePower(dp);
		cout << "Player created with health: " << h << ", attack power: " << ap << ", defense power: " << dp << endl;
	
	}

	// Getters and setters for Player's private members
	void setUserPressedKeyList(CellType pressedKey) { userPressedKeyList.push_back(pressedKey); }
	vector<CellType>& getUserPressedKeyList() { return userPressedKeyList; }

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
			cout << RED << "Invalid key pressed. Please use w/a/s/d." << RESET << endl;
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

	void updatePlayerHealth(Player* player)
	{
		lock_guard<mutex> lock(mtx);
		if (getDefensePower() >= 3)
		{
			setDefensePower(getDefensePower() - 3);
			setShieldActivated(true);
		}
		else setHealth(getHealth() - 1);
	}

	void updateHPStatus(Player* player) {
		lock_guard<mutex> lock(mtx);
		setHealth(getHealth() + 1);
	}

	void updateDefensePower(Player* player)
	{
		lock_guard<mutex> lock(mtx);
		setDefensePower(getDefensePower() + 1);	
	}

	void updateCoinsStatus(Player* player)
	{
		lock_guard<mutex> lock(mtx);
		setCoinsNumber(getCoinsNumber() + 1);
	} 

	void writeHandledCellEvent(vector<CellType>& listOfTypes)

	{
		if (!listOfTypes.empty())
		{
			switch (listOfTypes[listOfTypes.size()-1]) {
			case EMPTY:
				cout << WHITE << "You moved to an empty cell." << RESET << endl;
				break;
			case ENEMY:
				cout << RED << "You encountered an enemy!" << RESET << endl;
				if (shieldActivated)
				{
					cout << GREEN << "You defended yourself with a shield! ( 3 shields are always used to protect your 1 health )" << RESET << endl;
					setShieldActivated(false);
				}
				else {
					cout << RED << "You lost 1 health! ( Not enough shields. Required 3 at least )" << RESET << endl;
				}
				break;
			case HP:
				cout << CYAN << "You found a health item!" << RESET << endl;
				break;
			case HP_ITEM:
				cout << GREEN << "You found a shield item!" << RESET << endl;
				break;
			case COIN:
				cout << YELLOW << "You found a coin!" << RESET << endl;
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
			cout << "You haven't moved yet." << endl;
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
			cout << "Exiting game." << endl;
			return false;
		}
		else
		{
			cout << endl << RED << "Invalid key pressed. Please use w/a/s/d. esc - exit game" << RESET<< endl;
			return false;
		}
	}


	void playerMove(int& fieldPosX, int& fieldPosY, atomic<bool>& eventToContinue, Field* gameField)
	{

		cout << "🧍 Player position: (" << fieldPosX << ", " << fieldPosY << ")" << endl;
		cout << "🗡️ Attack: " << getAttackPower() << " | 🛡️ Defense: " << getDefensePower() << endl;
		cout << "❤️ Health: " << getHealth() << endl;
		cout << "👾 Enemies encountered: " << getEnemiesNumber() << endl;
		cout << "💖 HP items collected: " << getHealth() << endl;
		cout << "💎 Coins collected: " << getCoinsNumber() << endl;
		cout << endl;

		writeHandledCellEvent(getUserPressedKeyList());

		/* ---------------------------------------------------------------------------- FOR TEST ---------------------------------------------------------------------------------------- */

		/*for (int i = 0; i < getUserPressedKeyList().size(); i++) cout << getUserPressedKeyList()[i] << " ";
		cout << endl;*/

		/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
		

		cout << endl << endl;
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

	json to_json() const
	{
		json j;
		j["health"] = getHealth();
		j["attackPower"] = getAttackPower();
		j["defensePower"] = getDefensePower();
		j["enemiesNumber"] = getEnemiesNumber();
		j["coinsNumber"] = getCoinsNumber();
		json fieldJson = json::array();
		for (const auto& row : getField()) {
			json rowJson = json::array();
			for (const auto& cell : row) {
				cout << cell << " ";
				rowJson.push_back(cellTypeToString(cell));
			}
			fieldJson.push_back(rowJson);
		}

		j["field"] = fieldJson;
		return j;
	}

	void from_json(const json& j)
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
		cout << "Player destroyed." << endl;
	}
};

class Backup {
private:
	std::string filename;

public:
	Backup(const std::string& file = "savegame.json") : filename(file) {}

	template<typename T>
	void save(const T& object) {
		std::ofstream fileStream(filename);
		if (fileStream.is_open()) {
			fileStream << object.to_json().dump(4);
			fileStream.close();
			std::cout << "✅ Game state saved to " << filename << std::endl;
		}
		else {
			std::cerr << "❌ Failed to open file for saving!" << std::endl;
		}
	}

	template<typename T>
	void load(T& object) {
		std::ifstream fileStream(filename);
		if (fileStream.is_open()) {
			try
			{
				json j;
				fileStream >> j;
				object.from_json(j);
				
				std::cout << "📂 Game state loaded from " << filename << std::endl;
			}
			catch (const std::exception& e)
			{
				std::cerr << "❌ Failed to load game state: " << e.what() << std::endl;
			}
			fileStream.close();
		}
		else {
			std::cerr << "❌ Failed to open file for loading!" << std::endl;
		}
	}
};





#pragma region "Functions Prototypes"

pair<int, int> generateRandomPosition(Field* currentField);

void clearConsole();

void showMenu();

void showGameOver();

#pragma endregion





Player* globalPlayer = nullptr;
/*
	Signal handling
 */
void handleExitSignal(int signal) {
	cout << "\n[!] Отримано сигнал завершення: " << signal << std::endl;

	if (gameIsOn&& globalPlayer != nullptr) {
		std::cout << "[*] Зберігаємо гру перед виходом...\n";
		// TODO: виклик Backup::save(player);
		Backup backup;
		backup.save(*globalPlayer);
	}

	gameIsOn = false;
	exit(0);
}

void setupSignalHandlers() {
	signal(SIGINT, handleExitSignal);   // Ctrl+C
	signal(SIGTERM, handleExitSignal);  // kill

#ifndef _WIN32
	signal(SIGQUIT, handleExitSignal);  // Ctrl+\ (тільки для Linux/macOS)
#endif
}






int main() {
	system("chcp 65001 > nul");
	enum GameProperties {
		FIELD_HEIGHT = 10,
		FIELD_WIDTH = 10,
		PLAYER_HEALTH = 3,
		PLAYER_ATTACK = 3,
		PLAYER_DEFENSE = 3,
		NUMBER_OF_ENEMIES = 5,
		NUMBER_OF_COINS = 5,
		HP = 3,
		NUMBER_OF_HP_ITEM = 4,
	};

	srand(static_cast<unsigned int>(time(0)));

	showMenu();






	// Game field creating
	Field* field = new Field(FIELD_WIDTH, FIELD_HEIGHT);
	// Player creating
	Player* player = new Player(PLAYER_HEALTH, PLAYER_ATTACK, PLAYER_DEFENSE, FIELD_WIDTH, FIELD_HEIGHT);

	/* -------- For backup ------------- */
	globalPlayer = player;
	setupSignalHandlers();
	/* -------------- For backup ------------- */

	cout << endl << endl << MAGENTA << "Game has already been rendered successfully!" << RESET << endl;
	cout << endl << endl;
	cout << BLUE << "Press any key to start the game..." << RESET << endl;

	_getch(); // Wait for user input


	clearConsole();



	cout << endl << MAGENTA << "Game started!" << RESET << endl << endl;

	// Generate field and field items

	auto XY_PositionsForUser = generateRandomPosition(field);

	// User position

	field->addElement(XY_PositionsForUser.first, XY_PositionsForUser.second, PLAYER);

	// Generate enemies
	auto enemyTask = async(launch::async, [&]()
		{
			for (int i = 0; i < NUMBER_OF_ENEMIES; ++i) {
				auto enemyPos = generateRandomPosition(field);
				field->addElement(enemyPos.first, enemyPos.second, ENEMY);
			}
		}
	);

	// Generate coins
	auto coinTask = async(launch::async, [&]()
		{
			for (int i = 0; i < NUMBER_OF_COINS; ++i) {
				auto coinPos = generateRandomPosition(field);
				field->addElement(coinPos.first, coinPos.second, COIN);
			}
		}
	);

	// Generate HP items
	auto hpTask = async(launch::async, [&]()
		{
			for (int i = 0; i < NUMBER_OF_HP_ITEM; ++i) {
				auto hpPos = generateRandomPosition(field);
				field->addElement(hpPos.first, hpPos.second, HP_ITEM);
			}
		}
	);


	enemyTask.get();
	coinTask.get();
	hpTask.get();
	field->outputField();


	// Game process

	while (gameIsOn) {
		player->playerMove(XY_PositionsForUser.first, XY_PositionsForUser.second, gameIsOn, field);
		
		clearConsole();

		if (player->getHealth() <= 0 )
		{
			clearConsole();
			showGameOver();
			cout << RED << "You lost the game!" << RESET << endl;
			break;
		}
		else if (player->getCoinsNumber() >= 5)
		{
			clearConsole();
			showGameOver();
			cout << GREEN << "You won the game!" << RESET << endl;
			break;
		}
		if (gameIsOn == false)
		{
			clearConsole();
			showGameOver();
			cout << YELLOW << "You successfully quited the game!" << RESET << endl;
			break;
		}

		field->outputField();
	}

	delete field;
	delete player;
	cout << endl << BLUE << "Game ended!" << RESET << endl;
	

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


void printWithDelay(const string& line, int delayMs = 0.9) {
	for (char ch : line) {
		cout << ch << flush;
		sleep_for(chrono::milliseconds(delayMs));
	}
	cout << endl;
}

void showGameOver()
{
	printWithDelay(" ________  ________  _____ ______   _______           ________  ___      ___ _______   ________     ");
	printWithDelay("|\\   ____\\|\\   __  \\|\\   _ \\  _   \\|\\  ___ \\         |\\   __  \\|\\  \\    /  /|\\  ___ \\ |\\   __  \\    ");
	printWithDelay("\\ \\  \\___|\\ \\  \\|\\  \\ \\  \\\\__\\ \\  \\ \\   __/|        \\ \\  \\|\\  \\ \\  \\  /  / | \\   __/|\\ \\  \\|\\  \\   ");
	printWithDelay(" \\ \\  \\  __\\ \\   __  \\ \\  \\\\|__| \\  \\ \\  \\_|/__       \\ \\  \\\\\\  \\ \\  \\/  / / \\ \\  \\_|/_\\ \\   _  _\\  ");
	printWithDelay("  \\ \\  \\|\\  \\ \\  \\ \\  \\ \\  \\    \\ \\  \\ \\  \\_|\\ \\       \\ \\  \\\\\\  \\ \\    / /   \\ \\  \\_|\\ \\ \\  \\\\  \\| ");
	printWithDelay("   \\ \\_______\\ \\__\\ \\__\\ \\__\\    \\ \\__\\ \\_______\\       \\ \\_______\\ \\__/ /     \\ \\_______\\ \\__\\\\ _\\ ");
	printWithDelay("    \\|_______|\\|__|\\|__|\\|__|     \\|__|\\|_______|        \\|_______|\\|__|/       \\|_______|\\|__|\\|__|");
	printWithDelay("░  ░  ░    ░   ▒      ░   ░ ░    ░      ░ ░ ░ ▒       ░░     ░  ░ ░ ░ ▒  ░   ▒      ░   ░ ░    ░      ░ ░");
	printWithDelay("      ░        ░  ░         ░    ░  ░       ░ ░        ░     ░      ░ ░  ");
	printWithDelay("                                                    ░                  ");
	cout << endl;
}






/*
Кількість айтемів:
5 ворогів
5 коїнів
3 здоров'я
2 захисту
3 атаки

Створити функцію, яка зберігає гру (усі лічильники гравця, усі позиції елементів на полі)
Створити функцію, яка завантажує гру

Користувач на початку гри вибирає, або нову гру або завантажити (якщо є)
*/

                                                                                                    
                                                                                                    
                                                                                                    