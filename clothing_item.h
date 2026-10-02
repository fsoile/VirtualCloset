#ifndef CLOTHING_ITEM_H
#define CLOTHING_ITEM_H

#include <string>

class ClothingItem {
    private:
    std::string name; // name of the clothing item
    std::string category; // category e.g shoes, pants, shirts, etc.
    std::string color; // color of the item, e.g. "Blue", "Black" (defaults to "" if not given)
    
    public:
    ClothingItem(const std::string& name, const std::string& category, const std::string& color = ""); // specific constructor (color optional, defaults to "")
    ClothingItem(); // default constructor
    std::string getName() const; // get the name of the clothing item
    std::string getCategory() const; // get the category of the clothing item
    std::string getColor() const; // get the color of the clothing item
    std::string display() const; // display the clothing item, return a string with the name, category, and color (when present)
};
#endif

