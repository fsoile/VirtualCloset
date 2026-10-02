#ifndef CLOSET_H
#define CLOSET_H

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include "clothing_item.h"
#include "outfit_combo.h"

class Closet {
    private:
    std::stack<ClothingItem> closetStack; // stack of clothing items
    std::vector<ClothingItem> todaysOutfit; // vector of clothing items for today's outfit
    ClothingItem currentItem; // current clothing item
    bool isTryingOn; 

    // Tracks how many times each item (keyed by "name (category)", via
    // display(), so two different items can't collide under the same name)
    // has been chosen as part of a "Pick for Me" outfit. This is the data
    // behind reporting things like "which items get picked most often".
    std::unordered_map<std::string, int> pickFrequency;
    
    public:
    Closet(); // default constructor
    //funtions for the stack
    void addItem(const ClothingItem& item); // (push)add a clothing item to the closet
    void keepItem(); // move the current item to the todaysOutfit vector
    bool tryOn(); // pop the top item; returns true if an item was available
    void returnItem(); // push the current item back to the TOP of the closetStack
    void returnItemToBottom(); // push the current item to the BOTTOM of the closetStack
    // (uses the classic "insert at bottom of a stack" technique: pop
    // everything into a temporary stack, push the item, pop everything back.
    // This lets you cycle through every item in the closet without being
    // forced to keep one you don't want -- returnItem() alone would put a
    // rejected item right back on top, where the very next tryOn() would
    // hand it right back to you.)

    // The random outfit picker
    // NOTE: no longer const -- generating a pick now records frequency data,
    // which is a real change to the Closet's state, not just an internal
    // cache, so marking it const would misrepresent what the function does.
    OutfitCombo pickRandomOutfit(); // pick a (weighted-random, color-scored) outfit from today's outfit
    void clearTodaysOutfit(); // clear the todaysOutfit list

    // display functions
    void displayTodaysOutfit() const; // display all items in today's outfit
    void displayClosetStack() const; // display all items in the closetStack
    void displayPickFrequency() const; // display how often each item has been chosen by Pick for Me

    // getters
    ClothingItem getCurrentItem() const; // returns the item that is currently being tried on
    std::vector<ClothingItem> getTodaysOutfit() const; // returns the list of items in today's outfit
    std::stack<ClothingItem> getClosetStack() const; // returns a copy of the closetStack
    std::unordered_map<std::string, int> getPickFrequency() const; // returns a copy of the pick-frequency map

    // bool functions
    bool currentlyTryingOn() const; // returns true if the user is currently trying on an item
    bool isEmpty() const; // returns true if the closetStack is empty
    bool canMakeOutfit() const; //  |returns true if today's outfit has at least one item from each category
};
#endif 