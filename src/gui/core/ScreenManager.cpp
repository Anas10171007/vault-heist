#include "gui/core/ScreenManager.h"
#include "game/SaveManager.h"
#include "gui/screens/StubScreen.h"
#include "gui/screens/HeapLevelScreen.h"
#include "gui/screens/EndlessScreen.h"
#include "gui/screens/CampaignScreen.h"
#include "gui/screens/BriefingScreen.h"
#include "gui/screens/LevelScreen.h"
#include "gui/screens/LeaderboardScreen.h"
#include "gui/screens/ResultScreen.h"
#include "gui/screens/SandboxScreen.h"
#include "gui/screens/SettingsScreen.h"

namespace vh {

ScreenManager::ScreenManager(Theme& theme) {
    for (int i = 0; i < ScreenId::COUNT; i++) {
        screens[i] = nullptr;
    }

    SaveManager::loadProgress(session);

    StubScreen* menu = new StubScreen(theme, "Vault Heist", "Trees. Heaps. Zero chill.");
    menu->addButton("Start the Heist", "Campaign: 20 levels, 4 acts, one very annoying alarm",
                    ScreenId::CAMPAIGN_MAP);
    menu->addButton("Endless Chaos", "survive the stream of inserts and deletes",
                    ScreenId::ENDLESS);
    menu->addButton("Sandbox: Break Stuff", "any tree, any operation", ScreenId::SANDBOX);
    menu->addButton("Hall of Fame", "Leaderboard: your best scores", ScreenId::LEADERBOARD);
    menu->addButton("Settings", "speed, hints, and comfort", ScreenId::SETTINGS);
    menu->addButton("Bounce", "quit while you're ahead", ScreenId::QUIT);
    screens[ScreenId::MENU] = menu;

    // Campaign shell is ours. Acts 1–3 call Anas's LevelPlay through TreeMissionScreen.
    // Act 4 stays on HeapLevelScreen.
    screens[ScreenId::CAMPAIGN_MAP] = new CampaignScreen(theme, session);
    screens[ScreenId::BRIEFING] = new BriefingScreen(theme, session);
    screens[ScreenId::LEVEL_PLAY] = new LevelScreen(theme, session);
    screens[ScreenId::RESULT] = new ResultScreen(theme, session);
    screens[ScreenId::SANDBOX] = new SandboxScreen(theme);

    screens[ScreenId::ENDLESS] = new EndlessScreen(theme);

    screens[ScreenId::LEADERBOARD] = new LeaderboardScreen(theme, session);
    screens[ScreenId::SETTINGS] = new SettingsScreen(theme, session);

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

    // Esc is handled per-screen (level → campaign, campaign → menu, etc.).
    int requestedScreen = screens[activeScreen]->handleEvent(event, window);
    if (requestedScreen == ScreenId::QUIT) {
        window.close();
    } else if (requestedScreen != ScreenId::NONE) {
        show(requestedScreen);
    }
}

void ScreenManager::update(float deltaSeconds) {
    int requestedScreen = screens[activeScreen]->update(deltaSeconds);
    if (requestedScreen == ScreenId::QUIT) {
        // Quit is handled from events; ignore here.
    } else if (requestedScreen != ScreenId::NONE) {
        show(requestedScreen);
    }
}

void ScreenManager::draw(sf::RenderWindow& window) {
    screens[activeScreen]->draw(window);
}

void ScreenManager::show(int screenId) {
    if (screenId >= 0 && screenId < ScreenId::COUNT) {
        activeScreen = screenId;
        screens[activeScreen]->onShow();
    }
}

}
