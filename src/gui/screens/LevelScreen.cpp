#include "gui/screens/LevelScreen.h"
#include "game/Leaderboard.h"
#include "game/SaveManager.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"

namespace vh {

LevelScreen::LevelScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession), heapScreen(sharedTheme),
      mission(sharedTheme), heapMode(false), treeMode(false) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) break;
}

LevelScreen::~LevelScreen() {}

void LevelScreen::prepareTreeLevel(const LevelDef& level) {
    treeMode = true;
    heapMode = false;
    session.hasLevelReplay = false;
    session.reviewReplay = false;
    if (!mission.loadLevel(level.id)) {
        message = "Could not load tree level " + level.id;
        treeMode = false;
        return;
    }
    message.clear();
}

void LevelScreen::onShow() {
    heapMode = false;
    treeMode = false;
    message.clear();
    const LevelDef* level = levels.load(session.selectedLevel);
    if (!level) {
        message = "Level data unavailable.";
        return;
    }

    if (level->act == 4) {
        heapMode = true;
        heapScreen.setAnimationSpeed(session.animationSpeed);
        if (session.reviewReplay) {
            session.reviewReplay = false;
            if (heapScreen.startReviewReplay()) return;
        }
        session.hasLevelReplay = false;
        heapScreen.selectLevel(level->id);
        return;
    }

    prepareTreeLevel(*level);
}

void LevelScreen::storeResult(const ScoreSystem& score) {
    session.score = score.score();
    session.stars = score.stars();
    session.steps = score.steps();
    session.optimalSteps = score.optimalSteps();
    session.penalties = score.penalties();
    session.resultMessage = "Mission " + session.selectedLevel + " complete";
    session.sortedArrayText.clear();
    session.finalVaultUnlocked = false;
    session.nextLevelId.clear();
    session.recordStars(session.selectedLevel, score.stars());
    SaveManager::saveProgress(session);

    Leaderboard board;
    board.load();
    board.addScore(session.selectedLevel, score.score());
    board.save();

    for (int index = 0; index < levels.size(); index++) {
        if (levels.at(index).id == session.selectedLevel && index + 1 < levels.size()) {
            session.nextLevelId = levels.at(index + 1).id;
            break;
        }
    }
}

int LevelScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (heapMode) {
        int target = heapScreen.handleEvent(event, window);
        if (target == ScreenId::RESULT && !heapScreen.isReviewMode()) {
            storeResult(heapScreen.score());
            session.hasLevelReplay = heapScreen.canReviewReplay();
            if (heapScreen.isFinalVaultLevel()) {
                session.finalVaultUnlocked = true;
                session.sortedArrayText = heapScreen.sortedArrayText();
                session.resultMessage = "Final Vault Unlocked";
            }
        }
        return target;
    }

    if (treeMode) return mission.handleEvent(event, window);

    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
    return ScreenId::NONE;
}

int LevelScreen::update(float seconds) {
    if (heapMode) {
        int target = heapScreen.update(seconds);
        if (target == ScreenId::RESULT && !heapScreen.isReviewMode()) {
            storeResult(heapScreen.score());
            session.hasLevelReplay = heapScreen.canReviewReplay();
            if (heapScreen.isFinalVaultLevel()) {
                session.finalVaultUnlocked = true;
                session.sortedArrayText = heapScreen.sortedArrayText();
                session.resultMessage = "Final Vault Unlocked";
            }
        }
        return target;
    }
    if (treeMode) {
        int target = mission.update(seconds);
        if (target == ScreenId::RESULT && mission.hasLevel()) {
            storeResult(mission.score());
            if (!mission.statusMessage().empty()) session.resultMessage = mission.statusMessage();
        }
        return target;
    }
    return ScreenId::NONE;
}

void LevelScreen::draw(sf::RenderWindow& window) {
    if (heapMode) {
        heapScreen.draw(window);
        return;
    }
    if (treeMode) {
        mission.draw(window);
        return;
    }
    theme.drawBackdrop(window);
    Renderer renderer(window, theme.getFont());
    renderer.drawText(message.empty() ? "No level loaded." : message,
                      {550.f, 330.f}, 20, Palette::VIOLATION, true);
}

}
