#include "gui/screens/BriefingScreen.h"
#include "gui/core/Renderer.h"
#include "gui/core/ScreenId.h"
#include "gui/screens/TreeMissionScreen.h"

namespace {

string friendlyTask(const string& task) {
    string result;
    for (std::size_t i = 0; i < task.size(); i++) {
        char c = task[i];
        if (c == '_') {
            result += ' ';
        } else {
            if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 32);
            result += c;
        }
    }
    return result;
}

} // namespace

namespace vh {
BriefingScreen::BriefingScreen(Theme& sharedTheme, GameSession& sharedSession)
    : theme(sharedTheme), session(sharedSession) {
    const char* paths[] = {"data/levels", "../data/levels", "../../data/levels"};
    for (const char* path : paths) if (levels.loadAll(path)) break;
}
sf::FloatRect BriefingScreen::startButton() const { return {{350.f, 470.f}, {400.f, 48.f}}; }
sf::FloatRect BriefingScreen::backButton() const { return {{350.f, 532.f}, {400.f, 44.f}}; }

int BriefingScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (key && key->code == sf::Keyboard::Key::Escape) return ScreenId::CAMPAIGN_MAP;
    if (key && key->code == sf::Keyboard::Key::Enter) return ScreenId::LEVEL_PLAY;
    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left) return ScreenId::NONE;
    sf::Vector2f point = window.mapPixelToCoords(click->position);
    if (startButton().contains(point)) return ScreenId::LEVEL_PLAY;
    if (backButton().contains(point)) return ScreenId::CAMPAIGN_MAP;
    return ScreenId::NONE;
}

void BriefingScreen::draw(sf::RenderWindow& window) {
    theme.drawVaultBackdrop(window);
    Renderer renderer(window, theme.getFont());
    theme.drawPanel(window, {{180.f, 40.f}, {740.f, 610.f}});

    const LevelDef* level = levels.load(session.selectedLevel);
    if (!level) {
        renderer.drawText("LEVEL DATA UNAVAILABLE", {550.f, 250.f}, 24, Palette::VIOLATION, true);
        return;
    }

    theme.drawChip(window, {550.f, 80.f}, "MISSION BRIEF", 10, Palette::BRASS);
    renderer.drawText("MISSION " + level->id, {550.f, 108.f}, 11, Palette::MUTED, true);
    theme.drawGlowTitle(window, level->title, {550.f, 140.f}, 28, Palette::BRASS, true);

    theme.drawCard(window, {{230.f, 176.f}, {640.f, 124.f}}, Palette::VISITED, false);
    theme.drawWrappedText(window, level->mission, {550.f, 238.f}, 14, Palette::TEXT, 590.f,
                          20.f, 5);

    string objective = TreeMissionScreen::objectiveFor(*level);
    if (objective != level->mission) {
        renderer.drawText(theme.elide(objective, 12, 640.f), {550.f, 316.f}, 12,
                          Palette::MUTED, true);
    }

    theme.drawChip(window, {550.f, 344.f}, "CONCEPT · " + friendlyTask(level->task), 10,
                   Palette::VISITED);
    renderer.drawText("Structure  ·  " + level->structure, {550.f, 374.f}, 12,
                      Palette::TEXT, true);

    renderer.drawText("BEST RUN", {470.f, 406.f}, 10, Palette::MUTED, true);
    int best = session.bestStarsFor(level->id);
    for (int i = 0; i < 3; i++) {
        theme.drawStar(window, {550.f + static_cast<float>(i - 1) * 30.f, 406.f}, 8.f,
                       i < best);
    }
    renderer.drawText("3 stars: optimal steps, no penalties, no hints", {550.f, 436.f}, 11,
                      Palette::MUTED, true);

    theme.drawGlossyButton(window, theme.getFont(), startButton(), "Let's go  [Enter]",
                           ButtonKind::PRIMARY, theme.hovered(window, startButton()));
    theme.drawGlossyButton(window, theme.getFont(), backButton(), "Back to map",
                           ButtonKind::GHOST, theme.hovered(window, backButton()));
}
}
