#include <SFML/Graphics.hpp>

#include "closet.h"

#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstdint>

int main() {
    constexpr unsigned int kWindowWidth = 800;
    constexpr unsigned int kWindowHeight = 600;

    sf::RenderWindow window(
        sf::VideoMode({kWindowWidth, kWindowHeight}),
        "Virtual Closet",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setVerticalSyncEnabled(true);

    sf::Font font;
    bool fontLoaded = font.openFromFile("C:\\Windows\\Fonts\\arial.ttf");
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("arial.ttf"); // optional local font fallback
    }
    if (!fontLoaded) {
        std::cerr
            << "Error: Could not load a font.\n"
            << "Tried: C:\\Windows\\Fonts\\arial.ttf and ./arial.ttf\n";
        return 1;
    }

    // Script font for the "Pick for Me!" banner (fallback to the main font).
    sf::Font scriptFont;
    bool scriptLoaded = scriptFont.openFromFile("C:\\Windows\\Fonts\\segoesc.ttf"); // Segoe Script
    if (!scriptLoaded) {
        scriptLoaded = scriptFont.openFromFile("C:\\Windows\\Fonts\\segoepr.ttf"); // Segoe Print
    }
    if (!scriptLoaded) {
        scriptFont = font;
    }

    auto fitTextToRect = [](sf::Text& t, const sf::FloatRect& rect, float padding, unsigned int minSize) {
        unsigned int size = t.getCharacterSize();
        // Shrink until text fits inside rect (including at minSize).
        while (size >= minSize) {
            const sf::FloatRect b = t.getLocalBounds();
            const float w = b.size.x;
            const float h = b.size.y;
            if (w <= rect.size.x - 2.f * padding && h <= rect.size.y - 2.f * padding) break;
            if (size == minSize) break;
            size--;
            t.setCharacterSize(size);
        }
    };

    const float margin = 20.f;
    const float gap = 20.f;

    // Reserve space at the bottom for buttons.
    const float buttonBarHeight = 90.f;
    const float panelHeight = kWindowHeight - 2.f * margin - buttonBarHeight;
    const float leftWidth = 240.f;
    const float centerWidth = 280.f;
    const float rightWidth = kWindowWidth - 2.f * margin - 2.f * gap - leftWidth - centerWidth;

    const sf::Color panelFill(245, 245, 245);
    const sf::Color panelOutline(60, 60, 60);

    sf::RectangleShape closetPanel({leftWidth, panelHeight});
    closetPanel.setPosition({margin, margin});
    closetPanel.setFillColor(panelFill);
    closetPanel.setOutlineColor(panelOutline);
    closetPanel.setOutlineThickness(2.f);

    sf::RectangleShape currentPanel({centerWidth, panelHeight});
    currentPanel.setPosition({margin + leftWidth + gap, margin});
    currentPanel.setFillColor(panelFill);
    currentPanel.setOutlineColor(panelOutline);
    currentPanel.setOutlineThickness(2.f);

    sf::RectangleShape outfitPanel({rightWidth, panelHeight});
    outfitPanel.setPosition({margin + leftWidth + gap + centerWidth + gap, margin});
    outfitPanel.setFillColor(panelFill);
    outfitPanel.setOutlineColor(panelOutline);
    outfitPanel.setOutlineThickness(2.f);

    auto makeLabel = [&](const sf::String& text, const sf::Vector2f& panelPos) {
        sf::Text label(font, text, 20);
        label.setCharacterSize(20);
        label.setFillColor(sf::Color::Black);
        label.setPosition(panelPos + sf::Vector2f(12.f, 10.f));
        return label;
    };

    sf::Text closetLabel = makeLabel("Closet Stack", closetPanel.getPosition());
    sf::Text currentLabel = makeLabel("Currently Trying", currentPanel.getPosition());
    sf::Text outfitLabel = makeLabel("Today's Outfit", outfitPanel.getPosition());

    // Panel content is rendered as "cards" (rectangles + text), not as plain lists.

    struct Button {
        sf::RectangleShape rect;
        sf::Text label;
        sf::Color baseColor;
        sf::Color hoverColor;
    };

    auto darken = [](sf::Color c, int amount) {
        auto clamp = [](int v) { return static_cast<std::uint8_t>(std::max(0, std::min(255, v))); };
        return sf::Color(
            clamp(static_cast<int>(c.r) - amount),
            clamp(static_cast<int>(c.g) - amount),
            clamp(static_cast<int>(c.b) - amount)
        );
    };

    auto makeButton = [&](const sf::String& text, const sf::Vector2f& pos, const sf::Vector2f& size, sf::Color fill, sf::Color textColor) {
        Button b{sf::RectangleShape{}, sf::Text(font, text, 16), fill, darken(fill, 25)};

        b.rect.setPosition(pos);
        b.rect.setSize(size);
        b.rect.setFillColor(b.baseColor);
        b.rect.setOutlineColor(sf::Color(60, 60, 60));
        b.rect.setOutlineThickness(1.f);

        b.label.setFillColor(textColor);

        // Center label.
        const sf::FloatRect bounds = b.label.getLocalBounds();
        b.label.setOrigin(sf::Vector2f{
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });
        b.label.setPosition(sf::Vector2f{pos.x + size.x / 2.f, pos.y + size.y / 2.f});

        return b;
    };

    const sf::Color blue(120, 190, 255);
    const sf::Color orange(255, 200, 120);
    const sf::Color gold(255, 236, 170);
    const sf::Color bannerGold(245, 199, 64);
    const sf::Color closetCard(210, 190, 255); // light purple
    const sf::Color outfitCard(200, 200, 200); // gray
    const sf::Color currentCard(255, 245, 140); // yellow highlight

    const float buttonY = margin + panelHeight + 20.f;
    const float buttonH = 44.f;
    const float buttonGap = 10.f;
    const float buttonW = (kWindowWidth - 2.f * margin - 5.f * buttonGap) / 6.f;

    std::vector<Button> buttons;
    buttons.reserve(6);
    buttons.push_back(makeButton("Add to Closet", {margin + 0.f * (buttonW + buttonGap), buttonY}, {buttonW, buttonH}, blue, sf::Color::White));
    buttons.push_back(makeButton("Try It On", {margin + 1.f * (buttonW + buttonGap), buttonY}, {buttonW, buttonH}, blue, sf::Color::White));
    buttons.push_back(makeButton("Keep It", {margin + 2.f * (buttonW + buttonGap), buttonY}, {buttonW, buttonH}, blue, sf::Color::White));
    buttons.push_back(makeButton("Back to Closet", {margin + 3.f * (buttonW + buttonGap), buttonY}, {buttonW, buttonH}, blue, sf::Color::White));
    buttons.push_back(makeButton("Pick for Me!", {margin + 4.f * (buttonW + buttonGap), buttonY}, {buttonW, buttonH}, orange, sf::Color(40, 40, 40)));
    buttons.push_back(makeButton("Clear Outfit", {margin + 5.f * (buttonW + buttonGap), buttonY}, {buttonW, buttonH}, blue, sf::Color::White));

    Closet closet;

    // Pre-filled demo data: a small closet with multiple items per category
    // and a mix of clearly-compatible and clearly-clashing colors, so the
    // color-compatibility scoring is visible immediately without requiring
    // the user to type anything in first.
    closet.addItem(ClothingItem("White Tee", "Tops", "White"));
    closet.addItem(ClothingItem("Red Flannel", "Tops", "Red"));
    closet.addItem(ClothingItem("Blue Jeans", "Bottoms", "Blue"));
    closet.addItem(ClothingItem("Khaki Pants", "Bottoms", "Beige"));
    closet.addItem(ClothingItem("White Sneakers", "Shoes", "White"));
    closet.addItem(ClothingItem("Brown Boots", "Shoes", "Brown"));
    closet.addItem(ClothingItem("Black Belt", "Accessories", "Black"));
    closet.addItem(ClothingItem("Orange Scarf", "Accessories", "Orange"));

    bool pickResultActive = false;
    OutfitCombo pickResult;
    std::string pickStatusMessage; // shown in the gold banner when outfit can't be made yet

    // Add-to-closet UI state (center panel)
    bool addModeActive = false;
    bool addNameActive = true;
    bool addColorActive = false; // mutually exclusive with addNameActive
    std::string addNameBuffer;
    std::string addColorBuffer; // optional; empty is valid (see ClothingItem's default color)
    size_t selectedCategory = 0; // 0..3
    const std::vector<std::string> categories = {"Tops", "Bottoms", "Shoes", "Accessories"};

    auto makeCardText = [&](unsigned int size, sf::Color color) {
        sf::Text t(font);
        t.setCharacterSize(size);
        t.setFillColor(color);
        return t;
    };

    auto centerTextInRect = [](sf::Text& t, const sf::FloatRect& r) {
        const sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(sf::Vector2f{
            b.position.x + b.size.x / 2.f,
            b.position.y + b.size.y / 2.f
        });
        t.setPosition(sf::Vector2f{
            r.position.x + r.size.x / 2.f,
            r.position.y + r.size.y / 2.f
        });
    };

    auto drawCard = [&](const sf::Vector2f& pos, const sf::Vector2f& size, sf::Color fill, const std::string& title, const std::string& subtitle, bool center = false) {
        sf::RectangleShape card(size);
        card.setPosition(pos);
        card.setFillColor(fill);
        card.setOutlineColor(sf::Color(80, 80, 80));
        card.setOutlineThickness(1.f);
        window.draw(card);

        sf::Text nameText = makeCardText(16, sf::Color::Black);
        nameText.setString(title);
        if (center) {
            centerTextInRect(nameText, card.getGlobalBounds());
            window.draw(nameText);
            return;
        }

        nameText.setPosition(pos + sf::Vector2f(10.f, 8.f));
        window.draw(nameText);

        sf::Text catText = makeCardText(14, sf::Color(50, 50, 50));
        catText.setString(subtitle);
        catText.setPosition(pos + sf::Vector2f(10.f, 28.f));
        window.draw(catText);
    };

    sf::Text tryPrompt = makeCardText(18, sf::Color::Black);
    tryPrompt.setString("Try an item on!");

    sf::Text addPrompt = makeCardText(16, sf::Color::Black);
    addPrompt.setString("Add to Closet");

    // Banner for "Pick for Me!"
    sf::RectangleShape pickBanner;
    pickBanner.setFillColor(bannerGold);
    pickBanner.setOutlineColor(sf::Color(80, 80, 80));
    pickBanner.setOutlineThickness(2.f);

    sf::Text pickBannerText(scriptFont, "", 18);
    pickBannerText.setFillColor(sf::Color(35, 25, 10));

    auto makeSmallButton = [&](const sf::String& text, const sf::Vector2f& pos, const sf::Vector2f& size, sf::Color fill, sf::Color textColor) {
        Button b{sf::RectangleShape{}, sf::Text(font, text, 14), fill, darken(fill, 25)};

        b.rect.setPosition(pos);
        b.rect.setSize(size);
        b.rect.setFillColor(b.baseColor);
        b.rect.setOutlineColor(sf::Color(60, 60, 60));
        b.rect.setOutlineThickness(1.f);

        b.label.setFillColor(textColor);

        const sf::FloatRect bounds = b.label.getLocalBounds();
        b.label.setOrigin(sf::Vector2f{
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });
        b.label.setPosition(sf::Vector2f{pos.x + size.x / 2.f, pos.y + size.y / 2.f});

        // Ensure long labels (e.g. "Accessories") fit inside the button.
        fitTextToRect(b.label, b.rect.getGlobalBounds(), 6.f, 10);
        const sf::FloatRect b2 = b.label.getLocalBounds();
        b.label.setOrigin(sf::Vector2f{
            b2.position.x + b2.size.x / 2.f,
            b2.position.y + b2.size.y / 2.f
        });
        b.label.setPosition(sf::Vector2f{pos.x + size.x / 2.f, pos.y + size.y / 2.f});
        return b;
    };

    // Center-panel "Add to Closet" controls (simple input + category selection)
    const float cpX = currentPanel.getPosition().x;
    const float cpY = currentPanel.getPosition().y;
    const float cpW = currentPanel.getSize().x;

    sf::RectangleShape nameField({cpW - 24.f, 44.f});
    nameField.setPosition({cpX + 12.f, cpY + 80.f});
    nameField.setFillColor(sf::Color::White);
    nameField.setOutlineColor(sf::Color(60, 60, 60));
    nameField.setOutlineThickness(2.f);

    sf::Text nameFieldLabel = makeCardText(14, sf::Color(50, 50, 50));
    nameFieldLabel.setString("Name");
    nameFieldLabel.setPosition(nameField.getPosition() + sf::Vector2f(6.f, -18.f));

    sf::Text nameFieldText = makeCardText(18, sf::Color(30, 30, 30));
    nameFieldText.setPosition(nameField.getPosition() + sf::Vector2f(10.f, 8.f));

    // Color field: same pattern as the name field, positioned just below it.
    // Free-text and optional -- an empty color is valid (ClothingItem
    // defaults color to "", and the compatibility scorer treats missing
    // color as "don't penalize", not as a mismatch).
    sf::RectangleShape colorField({cpW - 24.f, 44.f});
    colorField.setPosition({cpX + 12.f, cpY + 134.f});
    colorField.setFillColor(sf::Color::White);
    colorField.setOutlineColor(sf::Color(60, 60, 60));
    colorField.setOutlineThickness(2.f);

    sf::Text colorFieldLabel = makeCardText(14, sf::Color(50, 50, 50));
    colorFieldLabel.setString("Color (optional)");
    colorFieldLabel.setPosition(colorField.getPosition() + sf::Vector2f(6.f, -18.f));

    sf::Text colorFieldText = makeCardText(18, sf::Color(30, 30, 30));
    colorFieldText.setPosition(colorField.getPosition() + sf::Vector2f(10.f, 8.f));

    sf::Text categoryLabel = makeCardText(14, sf::Color(50, 50, 50));
    categoryLabel.setString("Category");
    categoryLabel.setPosition({cpX + 12.f, cpY + 204.f});

    std::vector<Button> categoryButtons;
    categoryButtons.reserve(4);
    const float catY = cpY + 228.f;
    const float catH = 34.f;
    const float catGap = 8.f;
    const float catW = (cpW - 24.f - 3.f * catGap) / 4.f;
    for (size_t i = 0; i < 4; i++) {
        categoryButtons.push_back(makeSmallButton(
            categories[i],
            {cpX + 12.f + static_cast<float>(i) * (catW + catGap), catY},
            {catW, catH},
            sf::Color(210, 210, 210),
            sf::Color::Black
        ));
    }

    Button confirmAdd = makeSmallButton("Confirm", {cpX + 12.f, cpY + 284.f}, {cpW - 24.f, 40.f}, blue, sf::Color::White);

    auto trim = [](std::string s) {
        const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
        while (!s.empty() && isSpace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
        while (!s.empty() && isSpace(static_cast<unsigned char>(s.back()))) s.pop_back();
        return s;
    };

    auto updatePanels = [&]() {
        // Only affects background tint for the "Pick for Me!" result.
        currentPanel.setFillColor(pickResultActive ? gold : panelFill);
    };

    updatePanels();

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    if (addModeActive) {
                        addModeActive = false;
                        addNameBuffer.clear();
                        addColorBuffer.clear();
                    } else {
                        window.close();
                    }
                } else if (keyPressed->code == sf::Keyboard::Key::F && !addModeActive) {
                    // Prints how many times each item has been chosen by
                    // "Pick for Me" so far -- the measurable output of the
                    // color-compatibility scoring. Guarded by !addModeActive
                    // so typing a name/color containing the letter F doesn't
                    // also trigger this.
                    std::cout << "\n--- Pick Frequency (press F anytime to refresh) ---\n";
                    closet.displayPickFrequency();
                    std::cout << "----------------------------------------------------\n";
                }
            } else if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                if (!addModeActive) continue;
                if (!addNameActive && !addColorActive) continue;

                const char32_t ch = textEntered->unicode;
                std::string& activeBuffer = addNameActive ? addNameBuffer : addColorBuffer;
                if (ch == 8) { // backspace
                    if (!activeBuffer.empty()) activeBuffer.pop_back();
                } else if (ch >= 32 && ch < 127) {
                    activeBuffer.push_back(static_cast<char>(ch));
                }
            } else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button != sf::Mouse::Button::Left) continue;

                const sf::Vector2f mousePos = window.mapPixelToCoords({mousePressed->position.x, mousePressed->position.y});

                if (addModeActive) {
                    if (nameField.getGlobalBounds().contains(mousePos)) {
                        addNameActive = true;
                        addColorActive = false;
                    } else if (colorField.getGlobalBounds().contains(mousePos)) {
                        addNameActive = false;
                        addColorActive = true;
                    }

                    for (size_t c = 0; c < categoryButtons.size(); c++) {
                        if (categoryButtons[c].rect.getGlobalBounds().contains(mousePos)) {
                            selectedCategory = c;
                        }
                    }

                    if (confirmAdd.rect.getGlobalBounds().contains(mousePos)) {
                        const std::string name = trim(addNameBuffer);
                        const std::string color = trim(addColorBuffer); // optional; empty is fine
                        if (!name.empty()) {
                            closet.addItem(ClothingItem(name, categories[selectedCategory], color));
                            addModeActive = false;
                            addNameBuffer.clear();
                            addColorBuffer.clear();
                            pickResultActive = false;
                            updatePanels();
                        }
                    }
                }

                for (size_t i = 0; i < buttons.size(); i++) {
                    if (!buttons[i].rect.getGlobalBounds().contains(mousePos)) continue;

                    if (i == 0) { // Add to Closet
                        addModeActive = true;
                        addNameActive = true;
                        addColorActive = false;
                        addNameBuffer.clear();
                        addColorBuffer.clear();
                        selectedCategory = 0;
                        pickResultActive = false;
                        updatePanels();
                    } else if (i == 1) { // Try It On
                        pickResultActive = false;
                        closet.tryOn();
                        updatePanels();
                    } else if (i == 2) { // Keep It
                        pickResultActive = false;
                        closet.keepItem();
                        updatePanels();
                    } else if (i == 3) { // Back to Closet
                        pickResultActive = false;
                        // Returns to the bottom, not the top, of the closet
                        // stack -- so rejecting an item doesn't immediately
                        // hand it right back to you on the next Try It On.
                        closet.returnItemToBottom();
                        updatePanels();
                    } else if (i == 4) { // Pick for Me!
                        // If we can't make a full outfit yet, show a status message in the banner.
                        if (!closet.canMakeOutfit()) {
                            // Two lines so script font fits the narrow gold banner.
                            pickStatusMessage =
                                "Need at least one top, bottom,\nshoes and accessory";
                            pickResult = OutfitCombo(); // keep result invalid
                        } else {
                            pickStatusMessage.clear();
                            pickResult = closet.pickRandomOutfit();
                        }
                        pickResultActive = true;
                        updatePanels();
                    } else if (i == 5) { // Clear Outfit
                        pickResultActive = false;
                        pickStatusMessage.clear();
                        closet.clearTodaysOutfit();
                        updatePanels();
                    }
                }
            }
        }

        // Hover detection.
        const sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (auto& b : buttons) {
            const bool hovered = b.rect.getGlobalBounds().contains(mousePos);
            b.rect.setFillColor(hovered ? b.hoverColor : b.baseColor);
        }

        if (addModeActive) {
            for (size_t i = 0; i < categoryButtons.size(); i++) {
                const bool hovered = categoryButtons[i].rect.getGlobalBounds().contains(mousePos);
                const bool selected = (i == selectedCategory);
                sf::Color base = selected ? sf::Color(160, 160, 160) : sf::Color(210, 210, 210);
                sf::Color hov = darken(base, 20);
                categoryButtons[i].rect.setFillColor(hovered ? hov : base);
            }

            const bool confirmHovered = confirmAdd.rect.getGlobalBounds().contains(mousePos);
            confirmAdd.rect.setFillColor(confirmHovered ? confirmAdd.hoverColor : confirmAdd.baseColor);

            nameFieldText.setString(addNameBuffer.empty() ? std::string("Type a name...") : addNameBuffer);
            nameFieldText.setFillColor(addNameBuffer.empty() ? sf::Color(140, 140, 140) : sf::Color(30, 30, 30));
            nameField.setOutlineColor(addNameActive ? sf::Color(80, 120, 255) : sf::Color(60, 60, 60));

            colorFieldText.setString(addColorBuffer.empty() ? std::string("e.g. Blue") : addColorBuffer);
            colorFieldText.setFillColor(addColorBuffer.empty() ? sf::Color(140, 140, 140) : sf::Color(30, 30, 30));
            colorField.setOutlineColor(addColorActive ? sf::Color(80, 120, 255) : sf::Color(60, 60, 60));
        }

        window.clear(sf::Color(230, 230, 230));

        window.draw(closetPanel);
        window.draw(currentPanel);
        window.draw(outfitPanel);

        window.draw(closetLabel);
        window.draw(currentLabel);
        window.draw(outfitLabel);

        // ---- Closet Stack cards (light purple) ----
        {
            std::stack<ClothingItem> st = closet.getClosetStack();
            std::vector<ClothingItem> items;
            while (!st.empty()) {
                items.push_back(st.top());
                st.pop();
            }

            const float topY = closetPanel.getPosition().y + 44.f;
            const float x = closetPanel.getPosition().x + 10.f;
            const float w = closetPanel.getSize().x - 20.f;
            const float h = 46.f;
            const float spacing = 10.f;

            int maxCards = static_cast<int>((closetPanel.getSize().y - 54.f) / (h + spacing));
            if (maxCards < 0) maxCards = 0;
            if (static_cast<int>(items.size()) > maxCards) items.resize(static_cast<size_t>(maxCards));

            for (size_t i = 0; i < items.size(); i++) {
                const float y = topY + static_cast<float>(i) * (h + spacing);
                drawCard({x, y}, {w, h}, closetCard, items[i].getName(), items[i].getCategory());
            }
        }

        // ---- Currently Trying card (yellow) / Add-to-closet UI ----
        {
            const float pad = 12.f;
            const sf::Vector2f panelPos = currentPanel.getPosition();
            const sf::Vector2f panelSize = currentPanel.getSize();
            const float cardW = panelSize.x - 2.f * pad;
            const float cardH = 64.f;
            const float y = panelPos.y + 70.f;
            const float x = panelPos.x + pad;

            if (addModeActive) {
                addPrompt.setPosition({panelPos.x + 12.f, panelPos.y + 54.f});
                window.draw(addPrompt);
                window.draw(nameFieldLabel);
                window.draw(nameField);
                window.draw(nameFieldText);
                window.draw(colorFieldLabel);
                window.draw(colorField);
                window.draw(colorFieldText);
                window.draw(categoryLabel);
                for (const auto& cb : categoryButtons) {
                    window.draw(cb.rect);
                    window.draw(cb.label);
                }
                window.draw(confirmAdd.rect);
                window.draw(confirmAdd.label);
            } else if (pickResultActive) {
                // Gold banner: show either an error/status message or the 4-line outfit (script font).
                const float bannerPad = 12.f;
                const float bannerH = 144.f; // enough height for 5 lines (4 items + score) in script font
                pickBanner.setPosition({panelPos.x + bannerPad, panelPos.y + 56.f});
                pickBanner.setSize({panelSize.x - 2.f * bannerPad, bannerH});
                window.draw(pickBanner);

                if (!pickStatusMessage.empty()) {
                    pickBannerText.setString(pickStatusMessage);
                } else if (pickResult.isValid()) {
                    const ClothingItem t = pickResult.getTop();
                    const ClothingItem b = pickResult.getBottom();
                    const ClothingItem s = pickResult.getShoes();
                    const ClothingItem a = pickResult.getAccessory();

                    pickBannerText.setString(
                        "Top: " + t.getName() + "\n"
                        "Bottom: " + b.getName() + "\n"
                        "Shoes: " + s.getName() + "\n"
                        "Accessory: " + a.getName() + "\n"
                        "Score: " + std::to_string(pickResult.getCompatibilityScore()) + "/12"
                    );
                } else {
                    pickBannerText.setString(
                        "Need at least one top, bottom,\nshoes and accessory");
                }

                pickBannerText.setCharacterSize(22);
                fitTextToRect(pickBannerText, pickBanner.getGlobalBounds(), 8.f, 10);
                centerTextInRect(pickBannerText, pickBanner.getGlobalBounds());
                window.draw(pickBannerText);
            } else if (closet.currentlyTryingOn()) {
                const ClothingItem cur = closet.getCurrentItem();
                drawCard({x, y}, {cardW, cardH}, currentCard, cur.getName(), cur.getCategory());
            } else {
                // Prompt text when nothing is being tried on.
                centerTextInRect(tryPrompt, sf::FloatRect(sf::Vector2f{panelPos.x, panelPos.y + 80.f}, sf::Vector2f{panelSize.x, 60.f}));
                window.draw(tryPrompt);
            }
        }

        // ---- Today's Outfit cards (gray) ----
        {
            const std::vector<ClothingItem> outfit = closet.getTodaysOutfit();

            const float topY = outfitPanel.getPosition().y + 44.f;
            const float x = outfitPanel.getPosition().x + 10.f;
            const float w = outfitPanel.getSize().x - 20.f;
            const float h = 46.f;
            const float spacing = 10.f;

            int maxCards = static_cast<int>((outfitPanel.getSize().y - 54.f) / (h + spacing));
            if (maxCards < 0) maxCards = 0;
            const size_t count = std::min(outfit.size(), static_cast<size_t>(maxCards));

            for (size_t i = 0; i < count; i++) {
                const float y = topY + static_cast<float>(i) * (h + spacing);
                drawCard({x, y}, {w, h}, outfitCard, outfit[i].getName(), outfit[i].getCategory());
            }
        }

        for (const auto& b : buttons) {
            window.draw(b.rect);
            window.draw(b.label);
        }

        window.display();
    }

    return 0;
}

