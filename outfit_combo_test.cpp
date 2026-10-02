#include "outfit_combo.h"

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

static std::vector<ClothingItem> makeOneOfEachCategory() {
    // NOTE: Categories must match exactly what OutfitCombo expects.
    // (Current project uses: "Tops", "Bottoms", "Shoes", "Accessories")
    return {
        ClothingItem("tee", "Tops"),
        ClothingItem("jeans", "Bottoms"),
        ClothingItem("sneakers", "Shoes"),
        ClothingItem("watch", "Accessories"),
    };
}

static void test_default_constructor_invalid() {
    OutfitCombo o;
    assert(!o.isValid());

    // For an invalid combo, display() should return the invalid message.
    assert(contains(o.display(), "invalid"));

    // print() should print display() + newline (even if invalid).
    const std::string out = captureCout([&]() { o.print(); });
    assert(contains(out, o.display()));
    assert(!out.empty() && out.back() == '\n');
}

static void test_canMakeOutfit_true_only_when_all_categories_present() {
    {
        std::vector<ClothingItem> empty;
        assert(!OutfitCombo::canMakeOutfit(empty));
    }

    {
        // Missing Accessories
        std::vector<ClothingItem> items = {
            ClothingItem("tee", "Tops"),
            ClothingItem("jeans", "Bottoms"),
            ClothingItem("sneakers", "Shoes"),
        };
        assert(!OutfitCombo::canMakeOutfit(items));
    }

    {
        // All 4 categories present
        std::vector<ClothingItem> items = makeOneOfEachCategory();
        assert(OutfitCombo::canMakeOutfit(items));
    }
}

static void test_specific_constructor_invalid_when_missing_category() {
    std::vector<ClothingItem> items = {
        ClothingItem("tee", "Tops"),
        ClothingItem("jeans", "Bottoms"),
        ClothingItem("sneakers", "Shoes"),
        // no Accessories
    };

    OutfitCombo o(items);
    assert(!o.isValid());
    assert(contains(o.display(), "invalid"));
}

static void test_specific_constructor_valid_and_getters() {
    // Make it deterministic by ensuring there is exactly 1 item per category.
    const std::vector<ClothingItem> items = makeOneOfEachCategory();
    OutfitCombo o(items);
    assert(o.isValid());

    const ClothingItem t = o.getTop();
    const ClothingItem b = o.getBottom();
    const ClothingItem s = o.getShoes();
    const ClothingItem a = o.getAccessory();

    assert(t.getCategory() == "Tops");
    assert(b.getCategory() == "Bottoms");
    assert(s.getCategory() == "Shoes");
    assert(a.getCategory() == "Accessories");

    // Because there is 1 per category, the names should match exactly as well.
    assert(t.getName() == "tee");
    assert(b.getName() == "jeans");
    assert(s.getName() == "sneakers");
    assert(a.getName() == "watch");
}

static void test_display_contains_all_parts_when_valid() {
    const std::vector<ClothingItem> items = makeOneOfEachCategory();
    OutfitCombo o(items);
    assert(o.isValid());

    const std::string d = o.display();

    // Should mention each label and each item.
    assert(contains(d, "Top:"));
    assert(contains(d, "Bottom:"));
    assert(contains(d, "Shoes:"));
    assert(contains(d, "Accessory:"));

    assert(contains(d, "tee"));
    assert(contains(d, "jeans"));
    assert(contains(d, "sneakers"));
    assert(contains(d, "watch"));
}

static void test_scoreCombo_all_neutral_scores_max() {
    // Every item neutral -> every pair scores 2 -> top-bottom (2*2=4) +
    // four other pairs (1*2 each = 8) = 12, the maximum possible score.
    ClothingItem top("shirt", "Tops", "White");
    ClothingItem bottom("jeans", "Bottoms", "Black");
    ClothingItem shoes("sneakers", "Shoes", "Gray");
    ClothingItem accessory("belt", "Accessories", "Denim");

    assert(OutfitCombo::scoreCombo(top, bottom, shoes, accessory) == 12);
}

static void test_scoreCombo_no_colors_scores_max() {
    // No color info anywhere -> pairScore treats missing info as a 2 for
    // every pair, so an uncolored combo should never look "bad" -> max (12).
    ClothingItem top("shirt", "Tops");
    ClothingItem bottom("jeans", "Bottoms");
    ClothingItem shoes("sneakers", "Shoes");
    ClothingItem accessory("belt", "Accessories");

    assert(OutfitCombo::scoreCombo(top, bottom, shoes, accessory) == 12);
}

static void test_scoreCombo_clashing_colors_scores_low() {
    // Red top + Green bottom: different temperature, not listed -> 0.
    // Everything else also deliberately mismatched to keep the score low.
    ClothingItem top("shirt", "Tops", "Red");
    ClothingItem bottom("jeans", "Bottoms", "Green");
    ClothingItem shoes("sneakers", "Shoes", "Purple");
    ClothingItem accessory("belt", "Accessories", "Orange");

    const int score = OutfitCombo::scoreCombo(top, bottom, shoes, accessory);
    // top-bottom (Red/Green) = 0 pairs, weighted x2 = 0
    // top-shoes (Red/Purple) = different temp, unlisted = 0
    // bottom-shoes (Green/Purple) = both Cool = 1
    // bottom-accessory (Green/Orange) = different temp = 0
    // top-accessory (Red/Orange) = both Warm = 1
    assert(score == 2);
}

static void test_scoreCombo_topBottom_weighted_double() {
    // Isolate the weighting rule: the top-bottom pair should count for
    // twice as much as any other single pair. To test this cleanly, compare
    // two combos that are identical except for the top-bottom pair's score,
    // holding every other pair's contribution constant.

    ClothingItem shoes("sneakers", "Shoes", "Black");       // neutral -> every pair with it scores 2
    ClothingItem accessory("belt", "Accessories", "Black"); // neutral -> every pair with it scores 2

    // Combo A: top-bottom is a perfect match (Blue/White, listed pair -> 2)
    ClothingItem topA("shirt", "Tops", "Blue");
    ClothingItem bottomA("jeans", "Bottoms", "White");

    // Combo B: identical except top-bottom is a known clash (Red/Green -> 0)
    ClothingItem topB("shirt", "Tops", "Red");
    ClothingItem bottomB("jeans", "Bottoms", "Green");

    const int scoreA = OutfitCombo::scoreCombo(topA, bottomA, shoes, accessory);
    const int scoreB = OutfitCombo::scoreCombo(topB, bottomB, shoes, accessory);

    // Every non-top-bottom pair scores 2 in both combos (shoes/accessory are
    // neutral, so top-shoes, bottom-shoes, bottom-accessory, top-accessory
    // are all 2 regardless of top/bottom color). That's 4 pairs x 2 = 8,
    // identical in both combos. The only difference is the top-bottom pair,
    // weighted x2: (2*2)=4 for combo A vs (2*0)=0 for combo B.
    assert(scoreA - scoreB == 4);
    assert(scoreA == 12); // 8 (other pairs) + 4 (top-bottom, weighted)
    assert(scoreB == 8);  // 8 (other pairs) + 0 (top-bottom, weighted)
}

int main() {
    test_default_constructor_invalid();
    test_canMakeOutfit_true_only_when_all_categories_present();
    test_specific_constructor_invalid_when_missing_category();
    test_specific_constructor_valid_and_getters();
    test_display_contains_all_parts_when_valid();
    test_scoreCombo_all_neutral_scores_max();
    test_scoreCombo_no_colors_scores_max();
    test_scoreCombo_clashing_colors_scores_low();
    test_scoreCombo_topBottom_weighted_double();

    std::cout << "OutfitCombo tests passed.\n";
    return 0;
}

