#include "closet.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static std::string captureCout(const std::function<void()>& fn) {
    std::ostringstream oss;
    auto* oldBuf = std::cout.rdbuf(oss.rdbuf());
    fn();
    std::cout.rdbuf(oldBuf);
    return oss.str();
}

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

static void test_default_constructor_and_initial_state() {
    Closet c;
    assert(c.isEmpty());
    assert(!c.canMakeOutfit());
    assert(!c.currentlyTryingOn());
    assert(c.getCurrentItem().display() == "Empty ClothingItem");

    // display functions when empty
    const std::string closetOut = captureCout([&]() { c.displayClosetStack(); });
    assert(contains(closetOut, "No items in the closet yet."));

    const std::string outfitOut = captureCout([&]() { c.displayTodaysOutfit(); });
    assert(contains(outfitOut, "No items in today's outfit yet."));
}

static void test_addItem_tryOn_keepItem_flow() {
    Closet c;
    ClothingItem shirt("shirt", "Tops");
    ClothingItem jeans("jeans", "Bottoms");

    c.addItem(shirt);
    c.addItem(jeans); // top of stack
    assert(!c.isEmpty());

    // tryOn pops top (jeans)
    assert(c.tryOn());
    assert(c.currentlyTryingOn());
    assert(c.getCurrentItem().display() == "jeans (Bottoms)");

    // keepItem moves current to outfit and ends trying-on
    c.keepItem();
    assert(!c.currentlyTryingOn());
    assert(!c.canMakeOutfit()); // only bottoms so far; can't form a full outfit yet

    // outfit should show jeans
    const std::string out = captureCout([&]() { c.displayTodaysOutfit(); });
    assert(contains(out, "1. jeans (Bottoms)"));
}

static void test_returnItem_puts_back_on_stack() {
    Closet c;
    ClothingItem hat("hat", "Accessories");
    c.addItem(hat);

    assert(c.tryOn());
    assert(c.getCurrentItem().display() == "hat (Accessories)");

    c.returnItem();
    assert(!c.currentlyTryingOn());

    // Now stack should contain hat again
    auto s = c.getClosetStack();
    assert(!s.empty());
    assert(s.top().display() == "hat (Accessories)");
}

static void test_tryOn_when_empty_resets_state() {
    Closet c;
    assert(!c.tryOn());
    assert(!c.currentlyTryingOn());
    assert(c.getCurrentItem().display() == "Empty ClothingItem");
}

static void test_pickRandomOutfit_empty_and_non_empty() {
    Closet c;

    // empty outfit -> invalid OutfitCombo
    assert(!c.pickRandomOutfit().isValid());

    // Add a complete set: 1 item in each category.
    c.addItem(ClothingItem("tee", "Tops"));
    c.addItem(ClothingItem("jeans", "Bottoms"));
    c.addItem(ClothingItem("sneakers", "Shoes"));
    c.addItem(ClothingItem("watch", "Accessories"));

    // Move all 4 into today's outfit.
    for (int i = 0; i < 4; i++) {
        assert(c.tryOn());
        c.keepItem();
    }

    assert(c.canMakeOutfit());

    const OutfitCombo picked = c.pickRandomOutfit();
    assert(picked.isValid());
}

static void test_clearTodaysOutfit_resets_state_and_messages() {
    Closet c;

    // already empty message
    const std::string msg1 = captureCout([&]() { c.clearTodaysOutfit(); });
    assert(contains(msg1, "Today's outfit is already empty."));

    // add one outfit item
    c.addItem(ClothingItem("shirt", "Tops"));
    assert(c.tryOn());
    c.keepItem();
    assert(!c.canMakeOutfit()); // only tops; not enough for a full outfit

    const std::string msg2 = captureCout([&]() { c.clearTodaysOutfit(); });
    assert(contains(msg2, "Today's outfit has been cleared."));
    assert(contains(msg2, "You are now wearing nothing."));

    assert(!c.canMakeOutfit());
    assert(!c.currentlyTryingOn());
    assert(c.getCurrentItem().display() == "Empty ClothingItem");
}

static void test_displayClosetStack_ordering() {
    Closet c;
    c.addItem(ClothingItem("shirt", "Tops"));
    c.addItem(ClothingItem("jeans", "Bottoms")); // top

    const std::string out = captureCout([&]() { c.displayClosetStack(); });
    // Should print top-to-bottom: jeans first, then shirt
    const size_t jeansPos = out.find("jeans (Bottoms)");
    const size_t shirtPos = out.find("shirt (Tops)");
    assert(jeansPos != std::string::npos);
    assert(shirtPos != std::string::npos);
    assert(jeansPos < shirtPos);
}

static void test_pickFrequency_starts_empty() {
    Closet c;
    assert(c.getPickFrequency().empty());

    const std::string out = captureCout([&]() { c.displayPickFrequency(); });
    assert(contains(out, "No outfits have been picked yet."));
}

static void test_pickFrequency_records_chosen_items() {
    Closet c;
    // Exactly one item per category -> the OutfitCombo constructor has no
    // choice at all, so every pick must choose these exact 4 items. This
    // makes the frequency counts fully deterministic and easy to verify.
    c.addItem(ClothingItem("tee", "Tops", "White"));
    c.addItem(ClothingItem("jeans", "Bottoms", "Blue"));
    c.addItem(ClothingItem("sneakers", "Shoes", "Black"));
    c.addItem(ClothingItem("watch", "Accessories", "Gray"));
    for (int i = 0; i < 4; i++) {
        assert(c.tryOn());
        c.keepItem();
    }

    // Pick 3 times; with only one possible combo, all 4 items should each
    // be recorded exactly 3 times.
    for (int i = 0; i < 3; i++) {
        const OutfitCombo combo = c.pickRandomOutfit();
        assert(combo.isValid());
    }

    const auto freq = c.getPickFrequency();
    assert(freq.at("tee (Tops, White)") == 3);
    assert(freq.at("jeans (Bottoms, Blue)") == 3);
    assert(freq.at("sneakers (Shoes, Black)") == 3);
    assert(freq.at("watch (Accessories, Gray)") == 3);

    const std::string out = captureCout([&]() { c.displayPickFrequency(); });
    assert(contains(out, "3 picks"));
}

static void test_pickFrequency_not_recorded_on_invalid_pick() {
    Closet c;
    // Missing categories -> pickRandomOutfit should return invalid and NOT
    // record anything in pickFrequency.
    c.addItem(ClothingItem("tee", "Tops", "White"));
    assert(c.tryOn());
    c.keepItem();

    const OutfitCombo combo = c.pickRandomOutfit();
    assert(!combo.isValid());
    assert(c.getPickFrequency().empty());
}

int main() {
    test_default_constructor_and_initial_state();
    test_addItem_tryOn_keepItem_flow();
    test_returnItem_puts_back_on_stack();
    test_tryOn_when_empty_resets_state();
    test_pickRandomOutfit_empty_and_non_empty();
    test_clearTodaysOutfit_resets_state_and_messages();
    test_displayClosetStack_ordering();
    test_pickFrequency_starts_empty();
    test_pickFrequency_records_chosen_items();
    test_pickFrequency_not_recorded_on_invalid_pick();

    std::cout << "Closet tests passed.\n";
    return 0;
}

