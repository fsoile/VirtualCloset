#include "outfit_combo.h"
#include "color_compatibility.h"

#include <algorithm>
#include <iostream>
#include <random>

using namespace std;

namespace {
// Returns only the items whose category exactly matches `category`.
// We keep the comparison strict (case + spelling) because `main.cpp` creates items
// using the fixed set: "Tops", "Bottoms", "Shoes", "Accessories".
vector<ClothingItem> filterByCategory(const vector<ClothingItem>& items, const string& category) {
    vector<ClothingItem> result;
    result.reserve(items.size()); // upper bound; avoids repeated reallocations

    for (const ClothingItem& item : items) {
        if (item.getCategory() == category) {
            result.push_back(item);
        }
    }

    return result;
}

// Picks 1 item uniformly at random from a non-empty vector.
// If the vector is empty, returns an "empty" ClothingItem so callers can remain safe.
ClothingItem pickOne(const vector<ClothingItem>& items) {
    if (items.empty()) return ClothingItem();

    // One RNG for the entire program run (no reseeding on every call).
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, items.size() - 1);

    return items[dist(rng)];
}

// How many random candidate outfits to generate before choosing one.
// A larger number gives the weighted pick more/better options to choose
// among, at the cost of a bit more work -- 10 is a reasonable middle ground
// for a closet-sized item list.
constexpr int kNumCandidates = 10;

// One generated candidate combo, paired with its compatibility score.
struct Candidate {
    ClothingItem top, bottom, shoes, accessory;
    int score;
};

// Weighted-random pick among a list of scored candidates.
// Each candidate's weight is (score + 1), so even a 0-scoring combo has a
// small nonzero chance of being chosen (this is a deliberate design choice:
// we want the best combos to be *likely*, not the only possible outcome).
Candidate weightedPick(const vector<Candidate>& candidates) {
    // Build the weight list in the same order as candidates.
    vector<int> weights;
    weights.reserve(candidates.size());
    for (const Candidate& c : candidates) {
        weights.push_back(c.score + 1);
    }

    static std::mt19937 rng(std::random_device{}());
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());

    return candidates[dist(rng)];
}
} // namespace

OutfitCombo::OutfitCombo() {
    valid = false;
    compatibilityScore = 0;
}

OutfitCombo::OutfitCombo(const vector<ClothingItem>& outfitList) {
    valid = false;
    compatibilityScore = 0;

    vector<ClothingItem> tops        = filterByCategory(outfitList, "Tops");
    vector<ClothingItem> bottoms     = filterByCategory(outfitList, "Bottoms");
    vector<ClothingItem> shoesList   = filterByCategory(outfitList, "Shoes");
    vector<ClothingItem> accessories = filterByCategory(outfitList, "Accessories");

    // All 4 categories must have at least one item
    if (tops.empty() || bottoms.empty() || shoesList.empty() || accessories.empty())
        return;   // valid stays false

    // Generate several random candidate combos and score each one by color
    // compatibility, instead of picking a single random item per category
    // with no regard for how the pieces look together.
    vector<Candidate> candidates;
    candidates.reserve(kNumCandidates);

    for (int i = 0; i < kNumCandidates; i++) {
        ClothingItem candTop       = pickOne(tops);
        ClothingItem candBottom    = pickOne(bottoms);
        ClothingItem candShoes     = pickOne(shoesList);
        ClothingItem candAccessory = pickOne(accessories);

        int score = scoreCombo(candTop, candBottom, candShoes, candAccessory);

        candidates.push_back({candTop, candBottom, candShoes, candAccessory, score});
    }

    const Candidate chosen = weightedPick(candidates);

    top       = chosen.top;
    bottom    = chosen.bottom;
    shoes     = chosen.shoes;
    accessory = chosen.accessory;
    compatibilityScore = chosen.score;
    valid     = true;
}

ClothingItem OutfitCombo::getTop() const { return top; }
ClothingItem OutfitCombo::getBottom() const { return bottom; }
ClothingItem OutfitCombo::getShoes() const { return shoes; }
ClothingItem OutfitCombo::getAccessory() const { return accessory; }
int OutfitCombo::getCompatibilityScore() const { return compatibilityScore; }

bool OutfitCombo::isValid() const {
    return valid;
}

void OutfitCombo::print() const {
    cout << display() << endl;
}

string OutfitCombo::display() const {
    if (!valid) return "OutfitCombo: (invalid - missing one or more categories)";

    return "Top: " + top.display()
        + " | Bottom: " + bottom.display()
        + " | Shoes: " + shoes.display()
        + " | Accessory: " + accessory.display()
        + " | Compatibility Score: " + to_string(compatibilityScore) + "/12";
}

bool OutfitCombo::canMakeOutfit(const vector<ClothingItem>& outfitList) {
    // Fast pre-check: confirm all 4 categories exist at least once.
    // This mirrors the same rule enforced by the constructor.
    bool hasTop = false;
    bool hasBottom = false;
    bool hasShoes = false;
    bool hasAccessory = false;

    for (const ClothingItem& item : outfitList) {
        const string cat = item.getCategory();
        if (cat == "Tops") hasTop = true;
        else if (cat == "Bottoms") hasBottom = true;
        else if (cat == "Shoes") hasShoes = true;
        else if (cat == "Accessories") hasAccessory = true;

        if (hasTop && hasBottom && hasShoes && hasAccessory) return true;
    }

    return hasTop && hasBottom && hasShoes && hasAccessory;
}

int OutfitCombo::scoreCombo(const ClothingItem& top, const ClothingItem& bottom,
                             const ClothingItem& shoes, const ClothingItem& accessory) {
    int score = 0;
    score += 2 * ColorCompatibility::pairScore(top.getColor(), bottom.getColor());       // core pairing, weighted x2
    score += ColorCompatibility::pairScore(top.getColor(), shoes.getColor());
    score += ColorCompatibility::pairScore(bottom.getColor(), shoes.getColor());
    score += ColorCompatibility::pairScore(bottom.getColor(), accessory.getColor());
    score += ColorCompatibility::pairScore(top.getColor(), accessory.getColor());
    return score;
}
