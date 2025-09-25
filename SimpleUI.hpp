#pragma once

#include <iostream>
#include <vector>
#include <initializer_list>
#include <functional>
#ifdef _WIN32
	#include <conio.h> // Windows needs this for QuickMenu::_getch()
#else
	#include <termios.h> // MacOS/Linux need these for _getch()
	#include <unistd.h>
#endif

void cls() { // Cross-platform console clear function. Useful in-tandem with Menus.
	#ifdef _WIN32
		system("CLS"); // For Windows
	#else
		system("clear"); // For MacOS and Linux
	#endif
}

namespace ANSI { // Set std::cout colors
	const std::string RESET = "\033[0m";
	const std::string BLACK = "\033[30m";
	const std::string RED = "\033[31m"; 
	const std::string GREEN = "\033[32m";
	const std::string YELLOW = "\033[33m";
	const std::string BLUE = "\033[34m"; 
	const std::string MAGENTA ="\033[35m";
	const std::string CYAN = "\033[36m";
}

class Menu { // Generates a simple, flexible menu. Accepts integers as input.
	protected:
		std::string _prompt = "";
		std::vector<std::string> _options;
		std::vector<std::function<void()> > _actions;
		std::function<void()> _header;
		int _prev_option = -1;
		std::string _prev_string = "";
		virtual void _print();

	public:
		Menu();
		Menu(const std::string& prompt);
		Menu& option(const std::string& label, std::function<void()> action);
		Menu& submenu(const std::string& label, Menu& submenu);
		Menu& header(std::function<void()> header);
		Menu& title(const std::string& prompt);
		virtual int get_option();
		std::string get_title(int i) { return _options.at(i); }
		const int& recall_option() const { return _prev_option; }
		std::string get_string();
		const std::string& recall_string() const { return _prev_string; }
		virtual void run();
};

Menu::Menu() {
	_options.push_back("Exit");
	_actions.push_back([]{ cls(); });
}

Menu::Menu(const std::string& prompt) {
	_prompt = prompt;
	_options.push_back("Exit");
	_actions.push_back([]{ cls(); });
}

Menu& Menu::option(const std::string& label, std::function<void()> action) {
	_options.push_back(label);
	_actions.push_back(action);
	return *this;
}

Menu& Menu::submenu(const std::string& label, Menu& submenu) {
	option(label, [&submenu]{ submenu.run(); });
	return *this;
}

Menu& Menu::header(std::function<void()> header) {
	_header = std::move(header);
	return *this;
}

Menu& Menu::title(const std::string& prompt) {
	_prompt = prompt;
	return *this;
}

void Menu::_print() { // Print the menu (Prompt + options)
	if (not _prompt.empty()) std::cout << _prompt << "\n";
	if (_header) {
		if (not _prompt.empty()) std::cout << ANSI::YELLOW;
		_header();
		std::cout << ANSI::RESET << "\n";
	}

	if (_options.empty()) return;

	std::cout << ANSI::CYAN;

	for (int i = 1; i < _options.size(); i++) 
		std::cout << i << ". " << _options.at(i) << "\n";

	std::cout << ANSI::MAGENTA << "0. " << _options.at(0) // Print 0 last. Reserved for Back/Quit.
		  << ANSI::RESET << "\n> "; // Reset std::cout color, print input line.
}

std::string Menu::get_string() { // Prompt the user for string input, then return the string.
	_print();
	std::string input;
	std::cin >> input;
	std::cout << "\n";
	_prev_string = input;

	return input;
}

int Menu::get_option() { // Return a user-inputted option as an int from the list of options.
	int option = -1;

	// User inputs options until a valid option is selected (in range of _options vector)
	while (std::cin >> option and (option < 0 or option >= _options.size())) 
		std::cout << ANSI::RED << "Invalid option.\n" << ANSI::RESET << "> ";

	std::cout << "\n";
	_prev_option = option;

	return option;
}

void Menu::run() {
	cls();
	int option = -1;
	_print();
	while (option != 0) {
		option = get_option();
		cls();
		_actions.at(option)();
		if (option != 0) _print();
	}
}

class SubMenu : public Menu { // SubMenus are Menus that must be run by a parent Menu.
	private:
		using Menu::run; // Prevent running SubMenu outside parent Menu/SubMenu.
	public:
		explicit SubMenu() : Menu() { _options.at(0) = "Back"; }
		explicit SubMenu(const std::string& prompt) : Menu(prompt) { _options.at(0) = "Back"; }
};

class Prompt : public Menu { // Prompts are Menus that ask for string input and use optional validation methods.
	private:
		std::function<bool(const std::string&)> _is_valid = [](const std::string&) { return true; };
		using Menu::get_option;
		using Menu::get_string;
		using Menu::recall_option;
		using Menu::recall_string;
		using Menu::run;

	public:
		explicit Prompt(const std::string& prompt) : Menu(prompt) {}
		explicit Prompt(const std::string& prompt, std::function<bool(const std::string&)> validation_method) 
			: Menu(prompt), _is_valid(validation_method) {}
		void _print() override;
		std::string get();
};

void Prompt::_print() { // Print the menu (Prompt + options)
	if (not _prompt.empty()) std::cout << _prompt << "\n";
	if (_header) {
		if (not _prompt.empty()) std::cout << ANSI::YELLOW;
		_header();
		std::cout << ANSI::RESET << "\n";
	}

	if (_options.empty()) return;

	std::cout << ANSI::RESET << "> "; // Reset std::cout color, print input line.
}

std::string Prompt::get() { // Get a valid string from the user.
	cls();
	std::string input = get_string();
	while (!_is_valid(input)) {
		std::cout << ANSI::RED << "Invalid input.\n" << ANSI::RESET;
		input = get_string();
	}
	cls();

	return input;
}

class QuickMenu : public Menu { // Create a menu that reads characters directly from the keyboard.
	public:
		explicit QuickMenu() : Menu() {}
		explicit QuickMenu(const std::string& prompt) : Menu(prompt) {}
		int get_option() override;
	private:
		char _getch();
};

char QuickMenu::_getch() { // Read directly from the keyboard instead of waiting for enter/return key.
	char buf = 0;
	struct termios old;
	memset(&old, 0, sizeof(old));   // Modification I added to prevent a warning

	// Get terminal attributes
	if (tcgetattr(STDIN_FILENO, &old) < 0)
		perror("tcgetattr");

 	// Set terminal to raw mode
	old.c_lflag &= ~ICANON; // Disable canonical mode
	old.c_lflag &= ~ECHO;   // Disable echoing
	old.c_cc[VMIN] = 1;     // Minimum input is 1 char
	old.c_cc[VTIME] = 0;    // No timeout

	if (tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0)
		perror("tcsetattr");

	// Read one character
	if (read(STDIN_FILENO, &buf, 1) < 0)
		perror("read");

	// Restore terminal attributes
	old.c_lflag |= ICANON;
	old.c_lflag |= ECHO;

	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old) < 0)
		perror("tcsetattr");

	return buf;	
}

int QuickMenu::get_option() { // Return a user-inputted option as an int from the list of options.
	int option = -1;

	// User inputs options until a valid option is selected (in range of _options vector)
	while ((option = _getch() - '0') and (option < 0 or option >= _options.size())) 
		std::cout << ANSI::RED << "Invalid option.\n" << ANSI::RESET;

	std::cout << "\n";
	_prev_option = option;

	return option;
}

class SubQuickMenu : public QuickMenu { // SubQuickMenus are SubMenus of QuickMenus (must be run by parent)
	private:
		using QuickMenu::run;
	public:
		explicit SubQuickMenu() : QuickMenu() { _options.at(0) = "Back"; }
		explicit SubQuickMenu(const std::string& prompt) : QuickMenu(prompt) { _options.at(0) = "Back"; }
};

