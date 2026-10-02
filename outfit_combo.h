#ifndef OUTFIT_COMBO_H
#define OUTFIT_COMBO_H

#include <vector>
#include <string>
#include "clothing_item.h"
using namespace std;


class OutfitCombo {
    private:
    ClothingItem top; // selected Tops category item
    ClothingItem bottom;// selected Bottoms category item
    ClothingItem shoes; // selected Shoes category item
    ClothingItem accessory; // selected Accessories category item
    bool valid; // true only when all 4 categories were selected
    int compatibilityScore; // total color-compatibility score of this combo (see scoreCombo)

    public:
    OutfitCombo(); // default constructor
    OutfitCombo(const vector<ClothingItem>& outfitList); // specific constructor
    
    // getters
    ClothingItem getTop() const; // get the top item
    ClothingItem getBottom() const; // get the bottom item
    ClothingItem getShoes() const; // get the shoes item
    ClothingItem getAccessory() const; // get the accessory item
    int getCompatibilityScore() const; // get this combo's total color-compatibility score

    //boolean functions
    bool isValid() const; // returns false if any category is missing

    // display functions
    void print() const; // print the outfit combination
    string display() const; // display the outfit combination

    
    static bool canMakeOutfit(const vector<ClothingItem>& outfitList); 
    // returns true if the outfitList has at least one item from each category
    // use before calling the constructor to give the user a message to fix the outfitList if it is not valid

    // Scores one candidate combo of 4 items by color compatibility.
    // Uses ColorCompatibility::pairScore() (0/1/2 per pair) across the pairs
    // that visually matter most:
    //   top-bottom   x2 weight (the visual core of the outfit)
    //   top-shoes    x1
    //   bottom-shoes x1
    //   bottom-accessory x1
    //   top-accessory    x1
    // Max possible score: top-bottom (2*2=4) + four other pairs (1*2 each=8) = 12.
    static int scoreCombo(const ClothingItem& top, const ClothingItem& bottom,
                           const ClothingItem& shoes, const ClothingItem& accessory);

};
#endif

    
