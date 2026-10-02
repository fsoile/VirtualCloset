#include "closet.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;


// default constructor
// initialize the closetStack to an empty stack
// initialize the todaysOutfit to an empty vector
// initialize the currentItem to an empty ClothingItem
// initialize the isTryingOn to false
Closet::Closet() : isTryingOn(false) {
    static bool seeded = false;
    if (!seeded) {
        seeded = true;
        srand(static_cast<unsigned>(time(nullptr))); // seed once per program run
    }
}

// add a clothing item to the closetStack
void Closet::addItem(const ClothingItem& item) {
    closetStack.push(item);
}

// try on the top clothing item from the closetStack
bool Closet::tryOn() {
    if (closetStack.empty()) {            // check if there are no items left to try on
        isTryingOn = false;               // ensure the state reflects that nothing is being tried on
        currentItem = ClothingItem();     // reset the current item to an "empty" ClothingItem
        return false;                     // report failure (no item was available)
    }

    currentItem = closetStack.top();      // copy the item on the top of the stack into currentItem
    closetStack.pop();                    // remove that item from the stack since it's now being tried on
    isTryingOn = true;                    // update state to show the user is currently trying on an item
    return true;                          // report success (an item is now being tried on)
}

// keep the current item in the todaysOutfit vector
// Does nothing if the user is not trying on an item
void Closet::keepItem() {
    if(!isTryingOn) return; // if the user is not trying on an item, do nothing
    todaysOutfit.push_back(currentItem); // add the current item to the todaysOutfit vector 
    isTryingOn = false; // update the state to show that the user is not trying on an item 
}

// return the current item to the TOP of the closetStack
// Does nothing if the user is not trying on an item
void Closet::returnItem() {
    if(!isTryingOn) return; // if the user is not trying on an item, do nothing
    closetStack.push(currentItem); // add the current item to the closetStack
    isTryingOn = false; // update the state to show that the user is not trying on an item
}

// return the current item to the BOTTOM of the closetStack instead of the top.
// Classic "insert at bottom of a stack" technique: since std::stack only
// exposes push/pop/top, there's no direct way to reach the bottom. Instead:
//   1. Pop everything off closetStack into a temporary stack (this reverses
//      the order: what was on top ends up on the bottom of temp, and vice versa).
//   2. Push currentItem onto the now-empty closetStack -- it's the only item
//      there, so it's automatically at the bottom.
//   3. Pop everything back off temp onto closetStack (reverses again,
//      restoring the original relative order on top of currentItem).
// Net effect: currentItem ends up at the very bottom, and everything else
// keeps its original order above it. This is O(n) instead of O(1), but for
// a closet-sized stack that's a fine trade-off for the behavior it enables.
void Closet::returnItemToBottom() {
    if (!isTryingOn) return;

    std::stack<ClothingItem> temp;
    while (!closetStack.empty()) {
        temp.push(closetStack.top());
        closetStack.pop();
    }

    closetStack.push(currentItem); // now the only item -> automatically the bottom

    while (!temp.empty()) {
        closetStack.push(temp.top());
        temp.pop();
    }

    isTryingOn = false;
}

// pick a full outfit (one from each category) from the todaysOutfit vector,
// using OutfitCombo's weighted color-compatibility scoring, and record which
// items were chosen in pickFrequency.
OutfitCombo Closet::pickRandomOutfit() {
    // If we don't have at least one of each category, return an invalid OutfitCombo.
    // (OutfitCombo's constructor also enforces this, but checking here makes the intent explicit.)
    if (!OutfitCombo::canMakeOutfit(todaysOutfit)) return OutfitCombo();

    // OutfitCombo's constructor handles filtering by category, candidate
    // generation, scoring, and the weighted-random selection.
    OutfitCombo combo(todaysOutfit);

    if (combo.isValid()) {
        // Record each chosen item using display() as the key, so two
        // different items that happen to share a name (but differ in
        // category or color) aren't conflated in the frequency counts.
        pickFrequency[combo.getTop().display()]++;
        pickFrequency[combo.getBottom().display()]++;
        pickFrequency[combo.getShoes().display()]++;
        pickFrequency[combo.getAccessory().display()]++;
    }

    return combo;
}

// clear the todaysOutfit list
void Closet::clearTodaysOutfit() {
    if (todaysOutfit.empty()) {
        cout << "Today's outfit is already empty." << endl;
    } else {
        todaysOutfit.clear(); // clear the todaysOutfit vector
        cout << "Today's outfit has been cleared." << endl;
        cout << "You are now wearing nothing." << endl;
    }

    // Reset "wearing/trying on" state as well.
    isTryingOn = false;
    currentItem = ClothingItem();
}

// display all items in today's outfit
// prints the items in today's outfit
// in the order they were added
void Closet::displayTodaysOutfit() const {
    if(todaysOutfit.empty()) {
        cout << "No items in today's outfit yet." << endl;
        return;
    }
    for(size_t i = 0; i < todaysOutfit.size(); i++) {
        cout << " " << i + 1 << ". " << todaysOutfit[i].display() << endl;
    }    
}

// display all items in the closetStack
// prints the items in the closetStack
// in the order they were added (top to bottom)
// uses a temporary stack to reverse the order of the items
// for the orignal not to be changed
void Closet::displayClosetStack() const {
    if(closetStack.empty()) {
        cout << "No items in the closet yet." << endl;
        return;
    }
    stack<ClothingItem> tempStack = closetStack; // create a temporary stack to reverse the order of the items
    int position = 1;
    while(!tempStack.empty()) {
        cout << " " << position++ << ". " << tempStack.top().display() << endl;
        tempStack.pop();
    }
}

// display how many times each item has been chosen by "Pick for Me" so far.
// This is the concrete, measurable output of the color-compatibility
// scoring: items with more compatible colors get selected more often.
void Closet::displayPickFrequency() const {
    if (pickFrequency.empty()) {
        cout << "No outfits have been picked yet." << endl;
        return;
    }
    for (const auto& entry : pickFrequency) {
        cout << " " << entry.first << ": " << entry.second
             << (entry.second == 1 ? " pick" : " picks") << endl;
    }
}

// return the current item that is being tried on
ClothingItem Closet::getCurrentItem() const {
    return currentItem;
}

// boolean function to check if the user is currently trying on an item
bool Closet::currentlyTryingOn() const {
    return isTryingOn;
}

// boolean function to check if the closetStack is empty
bool Closet::isEmpty() const {
    return closetStack.empty();
}

// returns true only if today's outfit contains at least one item from each required category
bool Closet::canMakeOutfit() const {
    return OutfitCombo::canMakeOutfit(todaysOutfit);
}

//vector function to return the todaysOutfit
std::vector<ClothingItem> Closet::getTodaysOutfit() const {
    return todaysOutfit;
}

//stack function to return a copy of the closetStack
std::stack<ClothingItem> Closet::getClosetStack() const {
    return closetStack;
}

// returns a copy of the pick-frequency map
std::unordered_map<std::string, int> Closet::getPickFrequency() const {
    return pickFrequency;
}
//end of the Closet class
