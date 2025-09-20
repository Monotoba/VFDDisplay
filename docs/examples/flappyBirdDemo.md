---

# **Flappy Bird–Style Demo Specification for VFDDisplay Library**

**Target Display:**

* 4 rows × 20 characters per row
* Each character is a 5×7 pixel matrix (VFD character cell)

**Objective:**
Simulate a simplified side-scrolling game where a “bird” moves vertically through gaps in obstacles. The player controls the bird’s vertical position to avoid collisions with obstacles. The game should demonstrate the VFDDisplay library’s graphics and timing capabilities.

---

## **1. Game Entities**

### 1.1 Bird

* **Representation:**

  * A single 5×7 character cell.
  * Can be displayed as a predefined sprite or a custom character.
* **Movement:**

  * Moves vertically only (up/down).
  * Responds to user input to move upward (“flap”). Gravity pulls downward automatically each frame.
* **Constraints:**

  * Cannot move outside the top or bottom of the display (row 0 or row 3).

### 1.2 Obstacles (Pipes)

* **Representation:**

  * Vertical columns of blocks occupying 1–3 rows with a gap.
  * Obstacles appear at the far right column and scroll left each frame.
  * Can be represented using custom characters or standard full-block characters.
* **Spacing:**

  * Horizontal distance between obstacles: configurable (e.g., 4–6 columns).
  * Gap height: 1–2 rows.

### 1.3 Background / Ground

* **Representation:**

  * Optionally a static row at the bottom (e.g., row 3) as the “ground.”
  * Could use a simple repeating pattern character.

---

## **2. Game Mechanics**

### 2.1 Gravity

* Each frame, the bird moves downward by 1 row unless the user flaps.
* Flap input moves the bird upward by 1 row (or configurable number of rows).

### 2.2 Collision Detection

* The game ends when the bird overlaps an obstacle’s occupied cell.
* Collision with the ground or ceiling also ends the game (if the ceiling is considered solid).

### 2.3 Scoring

* Each time the bird passes an obstacle without collision, the score increments by 1.
* Score should be displayed at the top of the screen or in a dedicated cell.

### 2.4 Game Speed

* Display scrolls horizontally at a fixed frame rate (e.g., 5–10 updates per second).
* Speed may gradually increase to demonstrate dynamic timing control in the library.

---

## **3. Input**

* **User Action:** Single input button to flap (move the bird upward).
* **Debouncing:** Input should be sampled once per frame to avoid multiple flaps per frame.

---

## **4. Display Considerations**

* **Character Mapping:**

  * Bird sprite must fit within a single 5×7 character.
  * Obstacles must be visually distinct from the bird.
* **Scrolling:**

  * Smooth horizontal scrolling achieved by shifting character columns across the VFD.
* **Frame Timing:**

  * Frame updates should maintain consistent speed (configurable via a timer).
* **Optional Visual Effects:**

  * Blinking bird on flap
  * Flashing obstacle on collision

---

## **5. Game States**

1. **Start Screen**

   * “Press Button to Start” message
   * Optional simple animation to show bird or scrolling effect

2. **Playing**

   * Bird moves vertically under gravity
   * Obstacles scroll left
   * Score updates in real-time

3. **Game Over**

   * Display “Game Over” message
   * Show final score
   * Option to restart game

---

## **6. Configurability / Parameters**

* Bird flap strength (rows per flap)
* Gravity strength (rows per frame)
* Scroll speed (frames per column shift)
* Obstacle gap size
* Distance between obstacles

---

## **7. Demonstration Goals**

* Showcase VFDDisplay capabilities:

  * Custom characters and sprites
  * Smooth horizontal scrolling
  * Dynamic updates to multiple rows
* Demonstrate timing and input handling
* Provide a recognizable, interactive demo without complex graphics

---
