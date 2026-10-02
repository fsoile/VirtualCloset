#include "clothing_item.h"
using namespace std;

// default constructor
// set the name, category, and color to empty strings
ClothingItem::ClothingItem() : name(""), category(""), color("") {}

// specific constructor
// set the name, category, and color to the given values
// color defaults to "" if not provided (see header), so existing 2-argument
// calls like ClothingItem("shirt", "tops") still compile unchanged.
// for example, ClothingItem("shirt", "tops", "blue") sets name="shirt",
// category="tops", color="blue"
ClothingItem::ClothingItem(const std::string& name, const std::string& category, const std::string& color)
    : name(name), category(category), color(color) {}

// get the name of the clothing item, return the name
string ClothingItem::getName() const {
    return name;
}

// get the category of the clothing item, return the category
string ClothingItem::getCategory() const {
    return category;
}

// get the color of the clothing item, return the color
string ClothingItem::getColor() const {
    return color;
}

// display the clothing item, return a string describing it.
// Examples:
//   name only:                     "shirt"
//   name + category:               "shirt (tops)"
//   name + category + color:       "shirt (tops, blue)"
//   name + color, no category:     "shirt (blue)"
//   nothing at all:                "Empty ClothingItem"
string ClothingItem::display() const {
    if (name.empty() && category.empty() && color.empty()) return "Empty ClothingItem";

    // No name to anchor the string to: fall back to whatever detail we do have.
    if (name.empty()) {
        if (!category.empty() && !color.empty()) return category + ", " + color;
        if (!category.empty()) return category;
        return color; // only color was given
    }

    // We have a name. Build up the "(...)" detail section from whatever
    // category/color values are present, joined by ", ".
    if (category.empty() && color.empty()) return name;

    string details = category;
    if (!details.empty() && !color.empty()) details += ", ";
    details += color;

    return name + " (" + details + ")";
}
