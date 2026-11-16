#include "gui/core/ScreenManager.h"
#include "gui/screens/StubScreen.h"
#include "gui/screens/AnimationDemoScreen.h"

namespace vh {

ScreenManager::ScreenManager(Theme& theme) {
    for (int i = 0; i < ScreenId::COUNT; i++) {
        screens[i] = nullptr;
    }

    StubScreen* menu = new StubScreen(theme, "Vault Heist", "Choose where to enter the living vault");
    menu->addButton("Campaign", ScreenId::CAMPAIGN_MAP);
    menu->addButton("Endless", ScreenId::ENDLESS);
    menu->addButton("Sandbox", ScreenId::SANDBOX);
    menu->addButton("Leaderboard", ScreenId::LEADERBOARD);
    menu->addButton("Settings", ScreenId::SETTINGS);
    menu->addButton("Quit", ScreenId::QUIT);
    screens[ScreenId::MENU] = menu;

    StubScreen* campaign = new StubScreen(theme, "Campaign Map", "Four acts and twenty vault missions");
    campaign->addButton("Open Level Briefing", ScreenId::BRIEFING);
    campaign->addButton("Main Menu", ScreenId::MENU);
    screens[ScreenId::CAMPAIGN_MAP] = campaign;

    StubScreen* briefing = new StubScreen(theme, "Level Briefing", "Review the mission before entering");
    briefing->addButton("Start Level", ScreenId::LEVEL_PLAY);
    briefing->addButton("Campaign Map", ScreenId::CAMPAIGN_MAP);
    screens[ScreenId::BRIEFING] = briefing;

    screens[ScreenId::LEVEL_PLAY] = new AnimationDemoScreen(theme);

    StubScreen* result = new StubScreen(theme, "Result", "Stars, steps, and penalties will appear here");
    result->addButton("Retry", ScreenId::LEVEL_PLAY);
    result->addButton("Campaign Map", ScreenId::CAMPAIGN_MAP);
    screens[ScreenId::RESULT] = result;

    StubScreen* sandbox = new StubScreen(theme, "Sandbox", "Free-form data structure operations");
    sandbox->addButton("Main Menu", ScreenId::MENU);
    screens[ScreenId::SANDBOX] = sandbox;

    StubScreen* endless = new StubScreen(theme, "Endless Mode", "Survive an increasing stream of operations");
    endless->addButton("Leaderboard", ScreenId::LEADERBOARD);
    endless->addButton("Main Menu", ScreenId::MENU);
    screens[ScreenId::ENDLESS] = endless;

    StubScreen* leaderboard = new StubScreen(theme, "Leaderboard", "Local high scores will appear here");
    leaderboard->addButton("Main Menu", ScreenId::MENU);
    screens[ScreenId::LEADERBOARD] = leaderboard;

    StubScreen* settings = new StubScreen(theme, "Settings", "Display and animation settings");
    settings->addButton("Main Menu", ScreenId::MENU);
    screens[ScreenId::SETTINGS] = settings;

    activeScreen = ScreenId::MENU;
}

ScreenManager::~ScreenManager() {
    for (int i = 0; i < ScreenId::COUNT; i++) {
        delete screens[i];
    }
}

void ScreenManager::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (event.is<sf::Event::Closed>()) {
        window.close();
        return;
    }

    const sf::Event::KeyPressed* keyPress = event.getIf<sf::Event::KeyPressed>();
    if (keyPress != nullptr && keyPress->code == sf::Keyboard::Key::Escape) {
        show(ScreenId::MENU);
        return;
    }

    int requestedScreen = screens[activeScreen]->handleEvent(event, window);
    if (requestedScreen == ScreenId::QUIT) {
        window.close();
    } else if (requestedScreen != ScreenId::NONE) {
        show(requestedScreen);
    }
}

void ScreenManager::update(float deltaSeconds) {
    screens[activeScreen]->update(deltaSeconds);
}

void ScreenManager::draw(sf::RenderWindow& window) {
    screens[activeScreen]->draw(window);
}

void ScreenManager::show(int screenId) {
    if (screenId >= 0 && screenId < ScreenId::COUNT) {
        activeScreen = screenId;
    }
}

}
