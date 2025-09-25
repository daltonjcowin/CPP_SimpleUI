# C++ SimpleUI
A lightweight, single-header C++ library for building polished command-line menus and prompts.
It eliminates boilerplate switch statements and input loops so you can focus on logic, not wiring.

• Header-only — just #include "SimpleUI.hpp"

• Fast setup — define menus and prompts in a few lines.

• Flexible actions — options and headers accpet lambda functions.

• Cross-platform — works on Linux, macOS, and Windows.

• Readable main() — code reads like a specification, not a state machine.

# Features
• Numbered menus (Menu)

• Numbered menus with direct keyboard read (QuickMenu)

• Text prompts with optional validation parameters (Prompt)

• SubMenus/SubQuickMenus with built-in "Back"

• Optional header callbacks (live status/info)

• Input recall for quick re-entry

• Minimal dependencies (standard C++ & OS APIs)

# Example
```
#include "SimpleUI.hpp"
#include <iostream>

using namespace std;

struct Config {
  int traffic_density = 2;
  bool dedicated_left = false;
  string street_name = "Hollywood Blvd."
};
Config config;

void run() {
  ...
}

int main() {
  QuickMenu main_menu;
  SubQuickMenu density_menu, left_menu, name_menu;
  main_menu
    .title("Main Menu")
    .option("Run simulation", &run)
    .submenu("Traffic density", density_menu)
    .submenu("Dedicated left turn lane", left_menu)
    .submenu("Street name", name_menu");
  density_menu
    .title("Set Traffic Density")
    .header([]{
      cout << "Current: << density_menu.get_title(config.traffic_density);
    })
    .option("Light", []{ config.traffic_density = 0; })
    .option("Moderate", []{ config.traffic_density = 1; })
    .option("Heavy", []{ config.traffic_density = 2; });
  left_menu
    .title("Enable Dedicated Left Turn Lane?")
    .header([]{
      cout << "Current: " << (config.dedicated_left ? "Enabled" : "Disabled");
    })
    .option("Enable", []{ config.dedicated_left = true; })
    .option("Disable", []{ config.dedicated_left = false; });
  name_menu
    .title("Set Street Name")
    .header([]{ cout << "Current: " << config.street_name })
    .option("Change", []{
      Prompt name("New name (16 characters max)", [](string input){ return input <= 16; })"
      config.street_name = name.get();
    })

  main_menu.run();
  return 0;
}
```
Main QuickMenu
```
Main Menu:
1. Run simulation
2. Traffic density
3. Dedicated left turn lane
4. Street name
0. Exit
```
Traffic Density SubQuickMenu
```
Set Traffic Density:
Current: [Heavy]
1. Light
2. Moderate
3. Heavy
0. Back
```
Street Name Prompt
```
New name (16 characters max)
>
```
