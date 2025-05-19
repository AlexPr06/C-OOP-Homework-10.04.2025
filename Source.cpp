// GameObject class
#include "GAME_OBJECT.h"

// Field class
#include "FIELD.h"


// PLayer
#include "PLAYER.h"





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
#include <utility>
#include <cstdlib>
#include <chrono>
#include <thread>
//#include <condition_variable>
#include <future>

#include <fstream>
#include <csignal>




using namespace std;
using namespace std::this_thread;     // for sleep_for
using namespace std::chrono_literals; // for ms




///*
//Create a mini game in the console:
//10x10 field
//Randomize items (coins: 5, enemies: 5, hp: 3, defense: 3, attack: 3)
//The user writes w/a/s/d into the console and walks with it.
//Collecting items
//Use classes as much as possible.
//*/






class Backup {
private:
	std::string filename;

public:
	Backup(const std::string& file = "savegame.json") : filename(file) {}

	template<typename T>
	void save(const T& object) {
		// Append to the file
		std::ofstream fileStream(filename, std::ios::app);
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
				nlohmann::json j;
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
Field* globalField = nullptr;
/*
	Signal handling
 */
void handleExitSignal(int signal) {
	cout << "\n[!] Отримано сигнал завершення: " << signal << std::endl;

	if (gameIsOn&& globalPlayer != nullptr && globalField != nullptr) {
		std::cout << "[*] Зберігаємо гру перед виходом...\n";
		// TODO: виклик Backup::save(player);
		Backup backup;
		// Clear the file before writing
		std::ofstream file("savegame.json", std::ios::trunc);

		if (file.is_open()) {
			file.close();
		}

		backup.save(*globalPlayer);
		backup.save(*globalField);

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
	globalField = field;
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
		if (player->getCoinsNumber() >= 5)
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

                                                                                                    
                                                                                                    
                                                                                                    