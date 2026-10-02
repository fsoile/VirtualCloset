#include "clothing_item.h"

#include <cassert>
#include <iostream>

static void test_default_constructor() {
    ClothingItem item;
    assert(item.getName().empty());
    assert(item.getCategory().empty());
    assert(item.display() == "Empty ClothingItem");
}

static void test_specific_constructor_normal() {
    ClothingItem item("shirt", "tops");
    assert(item.getName() == "shirt");
    assert(item.getCategory() == "tops");
    assert(item.display() == "shirt (tops)");
}

static void test_specific_constructor_empty_name() {
    ClothingItem item("", "tops");
    assert(item.getName().empty());
    assert(item.getCategory() == "tops");
    assert(item.display() == "tops");
}

static void test_specific_constructor_empty_category() {
    ClothingItem item("shirt", "");
    assert(item.getName() == "shirt");
    assert(item.getCategory().empty());
    assert(item.display() == "shirt");
}

static void test_color_defaults_to_empty_when_not_given() {
    // Old-style 2-argument call must still compile and behave as before.
    ClothingItem item("shirt", "tops");
    assert(item.getColor().empty());
    assert(item.display() == "shirt (tops)");
}

static void test_specific_constructor_with_color() {
    ClothingItem item("jeans", "Bottoms", "Blue");
    assert(item.getName() == "jeans");
    assert(item.getCategory() == "Bottoms");
    assert(item.getColor() == "Blue");
    assert(item.display() == "jeans (Bottoms, Blue)");
}

static void test_display_name_and_color_no_category() {
    ClothingItem item("scarf", "", "Red");
    assert(item.display() == "scarf (Red)");
}

static void test_display_no_name_but_category_and_color() {
    // Edge case flagged during design: no name, but category/color present.
    ClothingItem item("", "Bottoms", "Blue");
    assert(item.display() == "Bottoms, Blue");
}

static void test_display_no_name_color_only() {
    ClothingItem item("", "", "Blue");
    assert(item.display() == "Blue");
}

int main() {
    test_default_constructor();
    test_specific_constructor_normal();
    test_specific_constructor_empty_name();
    test_specific_constructor_empty_category();
    test_color_defaults_to_empty_when_not_given();
    test_specific_constructor_with_color();
    test_display_name_and_color_no_category();
    test_display_no_name_but_category_and_color();
    test_display_no_name_color_only();

    std::cout << "ClothingItem tests passed.\n";
    return 0;
}

