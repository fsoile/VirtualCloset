#ifndef COLOR_COMPATIBILITY_H
#define COLOR_COMPATIBILITY_H

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------
// Color compatibility rules.
//
// This is a small, hand-designed rule table (not learned from data) that
// says which colors are considered to "go together". It intentionally
// treats a set of NEUTRAL colors (Black, White, Gray, Denim) as compatible
// with everything, since that mirrors how most people actually think about
// outfit matching in real life.
//
// On top of the direct pairing table, colors are also classified as WARM or
// COOL (basic color theory: red/orange/yellow/brown/maroon/tan lean warm,
// blue/green/purple lean cool). Two colors of the same temperature get
// partial credit even if they aren't explicitly listed as a pair -- this
// turns compatibility from a strict yes/no into a graded 0/1/2 score:
//   2 = same color, one is neutral, or an explicit listed pair
//   1 = not explicitly listed, but same temperature (both warm or both cool)
//   0 = different temperature and not listed as compatible
//
// All lookups are case-insensitive: a color typed as "Blue", "blue", or
// "BLUE" is treated identically. The table itself is stored with lowercase
// keys as the canonical form; normalize() converts any input to that form
// before every comparison. This only affects matching -- the color string
// a user actually typed is stored and displayed exactly as given, never
// overwritten by normalize().
//
// The table is deliberately simple and editable: adding a new color means
// adding one entry to kColorTemperature (and, optionally, pairing entries).
// -----------------------------------------------------------------------

namespace ColorCompatibility {

enum class Temperature { Warm, Cool, Neutral, Unknown };

// Lowercases a color string for comparison purposes only. Case-insensitive
// matching means a user typing "green" gets the same result as "Green".
inline std::string normalize(const std::string& color) {
    std::string result = color;
    std::transform(result.begin(), result.end(), result.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Colors that are treated as compatible with every other color.
// Stored lowercase (the canonical form); see normalize().
inline const std::vector<std::string> kNeutralColors = {
    "black", "white", "gray", "denim"
};

inline bool isNeutral(const std::string& color) {
    const std::string norm = normalize(color);
    for (const std::string& neutral : kNeutralColors) {
        if (norm == neutral) return true;
    }
    return false;
}

// Warm/cool classification, used for the temperature fallback rule.
// Neutrals are excluded here since they're already handled separately.
// Keys stored lowercase (the canonical form); see normalize().
inline const std::unordered_map<std::string, Temperature> kColorTemperature = {
    {"red",    Temperature::Warm},
    {"orange", Temperature::Warm},
    {"yellow", Temperature::Warm},
    {"brown",  Temperature::Warm},
    {"beige",  Temperature::Warm},
    {"maroon", Temperature::Warm},
    {"tan",    Temperature::Warm},
    {"blue",   Temperature::Cool},
    {"green",  Temperature::Cool},
    {"purple", Temperature::Cool},
    {"pink",   Temperature::Cool},
    {"navy",   Temperature::Cool},
    {"olive",  Temperature::Cool},
};

inline Temperature temperatureOf(const std::string& color) {
    if (color.empty()) return Temperature::Unknown;
    if (isNeutral(color)) return Temperature::Neutral;
    auto it = kColorTemperature.find(normalize(color));
    if (it != kColorTemperature.end()) return it->second;
    return Temperature::Unknown; // color not in our table at all
}

// Non-neutral color pairing rules. Only colors that are NOT already neutral
// need entries here, since isCompatible()/pairScore() check neutrality first.
// Keys and values stored lowercase (the canonical form); see normalize().
inline const std::unordered_map<std::string, std::vector<std::string>> kColorPairings = {
    {"blue",   {"white", "gray", "denim", "brown", "beige", "tan"}},
    {"red",    {"black", "white", "denim", "beige"}},
    {"green",  {"black", "white", "denim", "brown", "beige", "olive"}},
    {"brown",  {"blue", "white", "beige", "green", "tan"}},
    {"beige",  {"blue", "black", "brown", "red", "green", "navy"}},
    {"yellow", {"black", "white", "denim", "gray"}},
    {"pink",   {"white", "gray", "denim", "black"}},
    {"purple", {"black", "white", "gray"}},
    {"orange", {"black", "white", "denim"}},
    {"navy",   {"white", "beige", "tan", "gray"}},
    {"olive",  {"white", "black", "tan", "brown"}},
    {"maroon", {"white", "black", "gray", "tan"}},
    {"tan",    {"blue", "navy", "brown", "olive", "maroon", "white"}},
};

// True if colorB appears in colorA's pairing list (one direction only).
inline bool listedPair(const std::string& colorA, const std::string& colorB) {
    auto it = kColorPairings.find(normalize(colorA));
    if (it == kColorPairings.end()) return false;
    const std::string normB = normalize(colorB);
    for (const std::string& c : it->second) {
        if (c == normB) return true;
    }
    return false;
}

// Graded compatibility score for a pair of colors:
//   2 = same color, either is neutral, or explicitly listed as a pair
//       (checked in both directions, since the table isn't fully symmetric)
//   1 = not explicitly listed, but both colors share the same temperature
//   0 = different temperature (or one/both unknown) and not listed together
//
// Missing color info (empty string) is treated as a "2" -- we don't have
// data to penalize with, so it shouldn't drag an otherwise-good combo down.
// All comparisons are case-insensitive (see normalize()).
inline int pairScore(const std::string& colorA, const std::string& colorB) {
    if (colorA.empty() || colorB.empty()) return 2;
    if (normalize(colorA) == normalize(colorB)) return 2;
    if (isNeutral(colorA) || isNeutral(colorB)) return 2;
    if (listedPair(colorA, colorB) || listedPair(colorB, colorA)) return 2;

    const Temperature tempA = temperatureOf(colorA);
    const Temperature tempB = temperatureOf(colorB);
    if (tempA != Temperature::Unknown && tempA == tempB) return 1;

    return 0;
}

// Kept for the standalone compatibility tests written earlier, and for any
// caller that just wants a yes/no answer: true whenever pairScore() > 0.
inline bool isCompatible(const std::string& colorA, const std::string& colorB) {
    return pairScore(colorA, colorB) > 0;
}

} // namespace ColorCompatibility

#endif
