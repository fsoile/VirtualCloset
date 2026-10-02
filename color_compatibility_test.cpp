#include "color_compatibility.h"

#include <cassert>
#include <iostream>

using namespace ColorCompatibility;

static void test_isNeutral() {
    assert(isNeutral("Black"));
    assert(isNeutral("White"));
    assert(isNeutral("Gray"));
    assert(isNeutral("Denim"));
    assert(!isNeutral("Blue"));
    assert(!isNeutral("Red"));
    assert(!isNeutral(""));
}

static void test_same_color_is_compatible() {
    assert(isCompatible("Blue", "Blue"));
    assert(isCompatible("Red", "Red"));
}

static void test_neutral_is_compatible_with_anything() {
    assert(isCompatible("Black", "Red"));
    assert(isCompatible("Purple", "White"));
    assert(isCompatible("Denim", "Orange"));
    // two neutrals together should also be compatible
    assert(isCompatible("Black", "White"));
}

static void test_missing_color_is_treated_as_compatible() {
    // No color info -> we don't penalize; matches the design decision that
    // items without a typed-in color shouldn't be scored as incompatible.
    assert(isCompatible("", "Red"));
    assert(isCompatible("Blue", ""));
    assert(isCompatible("", ""));
}

static void test_known_compatible_pairs() {
    assert(isCompatible("Blue", "White"));   // Blue lists White directly
    assert(isCompatible("White", "Blue"));   // reverse lookup direction
    assert(isCompatible("Red", "Black"));
    assert(isCompatible("Brown", "Beige"));
    assert(isCompatible("Beige", "Brown"));  // reverse direction again
}

static void test_known_incompatible_pair() {
    // Red is not listed as pairing with Green, and neither is neutral.
    assert(!isCompatible("Red", "Green"));
    // Purple and Orange: neither list contains the other.
    assert(!isCompatible("Purple", "Orange"));
}

static void test_unknown_color_not_in_table() {
    // A color with no entry in kColorPairings or kColorTemperature at all
    // (e.g. "Turquoise") isn't neutral, has Unknown temperature, and isn't
    // listed anywhere -- so it only scores compatible with itself or with
    // a neutral color.
    assert(!isCompatible("Turquoise", "Red"));
    assert(isCompatible("Turquoise", "Black"));   // neutral rule still applies
    assert(isCompatible("Turquoise", "Turquoise"));
}

static void test_temperatureOf() {
    assert(temperatureOf("Red") == Temperature::Warm);
    assert(temperatureOf("Tan") == Temperature::Warm);
    assert(temperatureOf("Blue") == Temperature::Cool);
    assert(temperatureOf("Navy") == Temperature::Cool);
    assert(temperatureOf("Black") == Temperature::Neutral);
    assert(temperatureOf("Turquoise") == Temperature::Unknown);
    assert(temperatureOf("") == Temperature::Unknown);
}

static void test_pairScore_exact_or_listed_pairs_score_2() {
    assert(pairScore("Blue", "Blue") == 2);       // same color
    assert(pairScore("Black", "Red") == 2);       // neutral
    assert(pairScore("Blue", "White") == 2);      // listed pair
    assert(pairScore("White", "Blue") == 2);      // reverse direction
    assert(pairScore("", "Red") == 2);            // missing info -> don't penalize
}

static void test_pairScore_same_temperature_scores_1() {
    // Red (Warm) and Orange (Warm) are not explicitly listed as a pair,
    // but share the same temperature -> partial credit.
    assert(pairScore("Red", "Orange") == 1);
    // Blue (Cool) and Purple (Cool): not listed together, same temperature.
    assert(pairScore("Blue", "Purple") == 1);
}

static void test_pairScore_different_temperature_and_unlisted_scores_0() {
    // Red (Warm) vs Green (Cool): different temperature, not listed together.
    assert(pairScore("Red", "Green") == 0);
    assert(pairScore("Purple", "Orange") == 0);
}

static void test_new_colors_added() {
    assert(isCompatible("Navy", "Tan"));    // listed pair
    assert(isCompatible("Olive", "White")); // listed pair
    assert(isCompatible("Maroon", "Gray")); // listed pair (Gray is neutral anyway)
    assert(pairScore("Maroon", "Red") == 1); // both Warm, not explicitly listed
}

static void test_case_insensitive_matching() {
    // Same result regardless of how the color was capitalized when typed.
    assert(pairScore("Blue", "White") == pairScore("blue", "white"));
    assert(pairScore("Blue", "White") == pairScore("BLUE", "WHITE"));
    assert(isNeutral("white") && isNeutral("White") && isNeutral("WHITE"));
    assert(temperatureOf("green") == temperatureOf("Green"));
    assert(temperatureOf("GREEN") == Temperature::Cool);

    // The exact real-world case that motivated this fix: lowercase "green"
    // and "blue" typed into the app must score the same as "Green"/"Blue".
    assert(pairScore("green", "blue") == pairScore("Green", "Blue"));
    assert(pairScore("green", "blue") == 1); // both Cool, not explicitly listed
}

int main() {
    test_isNeutral();
    test_same_color_is_compatible();
    test_neutral_is_compatible_with_anything();
    test_missing_color_is_treated_as_compatible();
    test_known_compatible_pairs();
    test_known_incompatible_pair();
    test_unknown_color_not_in_table();
    test_temperatureOf();
    test_pairScore_exact_or_listed_pairs_score_2();
    test_pairScore_same_temperature_scores_1();
    test_pairScore_different_temperature_and_unlisted_scores_0();
    test_new_colors_added();
    test_case_insensitive_matching();

    std::cout << "ColorCompatibility tests passed.\n";
    return 0;
}
