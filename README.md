# Virtual Closet

A C++ / SFML desktop app that simulates picking out an outfit: build a digital
closet, try items on one at a time, keep the ones you want, and let the app
generate a full outfit for you with **"Pick for Me."**

The base closet/stack mechanics were originally built as a data structures
course project. Everything described under **Color-Compatibility Scoring**
below — the color field, the scoring algorithm, weighted selection, and pick
frequency tracking — is original work added afterward, independent of that
course.

---

## What it does

1. **Build your closet** — add clothing items one at a time (name, category,
   optional color, matched case-insensitively). Items are stored in a
   **stack**, mirroring how you'd actually dig through a pile of clothes.
   The app starts with a small set of pre-filled demo items so there's
   something to try immediately.
2. **Try items on** — pop the top item off the stack, decide to **keep** it
   (moves to today's outfit) or **return** it. Returning sends the item to
   the **bottom** of the stack (not the top), so rejecting something you
   don't want doesn't just hand it right back to you on the next try — you
   can cycle through the whole closet without being forced to keep
   anything.
3. **Pick for Me** — once you have at least one item in each of the four
   categories (Tops, Bottoms, Shoes, Accessories), generate a full outfit.
   Instead of picking each piece completely at random, the app scores
   several candidate outfits by color compatibility and is more likely
   (not guaranteed) to choose a well-matched one. The resulting
   compatibility score is shown with the outfit.
4. **Press `F`** at any time to print how often each item has been chosen
   by Pick for Me so far, to the console.

---

## Classes and data structures

**Why a stack for the closet:** trying on clothes and putting them back is
naturally LIFO — you reach for what's on top, and if you don't want it, it
goes back on top. A `std::vector` used as a simple list would work
functionally, but a stack makes that access pattern explicit in the type
itself, not just in how the code happens to use it.

**Insert-at-bottom algorithm:** `std::stack` only exposes `push`, `pop`,
and `top` — there's no way to insert at the bottom directly. Returning a
rejected item to the *top* (the obvious approach) means the very next
`tryOn()` hands you that same item right back, so you can never see the
next item without being forced to keep something you don't want.
`Closet::returnItemToBottom()` solves this with the classic technique: pop
everything into a temporary stack (reversing order), push the rejected
item so it's now alone at the bottom, then pop everything back on top of
it (reversing again, restoring the original order above the new bottom
item). O(n) instead of O(1) 

---

## Color-Compatibility Scoring

The starting version of "Pick for Me" chose one random item per category
with zero regard for whether the colors actually looked good together. This
extension replaces that with a scoring system:

### 1. Pairwise color scoring — `pairScore(colorA, colorB)`
Returns a graded score, not just yes/no:
- **2** — same color, either color is neutral (Black/White/Gray/Denim), or
  the pair is explicitly listed as compatible in a hand-built pairing table.
- **1** — not explicitly listed, but both colors share the same "temperature"
  (warm: red/orange/yellow/brown/beige/maroon/tan — cool: blue/green/purple/
  pink/navy/olive). This is a fallback signal, not a replacement for the
  direct table.
- **0** — different temperature and not listed together.
- Missing/empty color is always scored as a **2** — an item without a typed
  color shouldn't be penalized for something we simply don't know.
- All color matching is **case-insensitive** (`"Blue"`, `"blue"`, and
  `"BLUE"` all score identically). The color a user types is still stored
  and displayed exactly as entered — only internal comparisons normalize
  case.

### 2. Whole-outfit scoring — `OutfitCombo::scoreCombo()`
Combines `pairScore()` across five pairs, with the top–bottom pairing
weighted **2x** since it's the visual core of an outfit:

```
score = 2 * pairScore(top, bottom)
      +     pairScore(top, shoes)
      +     pairScore(bottom, shoes)
      +     pairScore(bottom, accessory)
      +     pairScore(top, accessory)
```

Maximum possible score: **12** (all five pairs scoring a perfect 2, with the
top-bottom pair's 2 counted twice).

### 3. Weighted-random selection
"Pick for Me" doesn't just take the single best-scoring combo every time —
it generates **10 random candidate outfits**, scores each, then picks one
using **weighted random selection** (weight = score + 1, so even a
poorly-matched combo has a small, nonzero chance). This keeps the feature
feeling natural and varied instead of robotically always returning the same
"optimal" outfit, while still making well-matched outfits noticeably more
likely.

---

## limitations

- **The color rules are hand-designed, not learned from data.** They reflect
  one reasonable set of color-theory (neutral colors, warm/cool
  grouping), not a comprehensive model of fashion compatibility. A next step would be training a compatibility model
  on a real outfit dataset. 
- **Only a fixed, modest set of colors is recognized** (~17 colors). A color
  typed in that isn't in the table still works. It just falls back to
  "unknown temperature," so it can still score a 2 (via the neutral rule, if applicable) or a 0, but never gets temperature-based partial credit.
- **Category-specific nuance is simplified.** In real styling, an accessory or shoe in a bold, contrasting color can be an intentional "pop of color"rather than a mismatch. This version treats all pairs with the same scoring logic (aside from the top-bottom weighting) rehter than explicity modelling the distinction.
- **Weighted selection means a "bad" outfit can still occasionally be
  chosen.** This is deliberate (see design rationale above), but worth
  stating plainly: a single run is not a guarantee of a good match, only a
  statistically likely one.

---

## Build & run

Windows, using g++ (MinGW) and a local/extracted SFML 3.x install (path
configured via the `SFML_DIR` environment variable or a `.\SFML\` folder in
the repo root).

```
build_app.cmd
```

## Tests

Each class has an independent unit test file, so they build and run with a plain `g++` command:

```
build_test.cmd              # ClothingItem tests
build_closet_test.cmd       # Closet tests (includes pick-frequency tests)
build_outfit_combo_test.cmd # OutfitCombo tests (includes scoring tests)
```

Color compatibility is currently tested via `color_compatibility_test.cpp`
