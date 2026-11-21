# Data Structure Project

**Deadline: 20th November 2026 11:59 PM**

**Instructions**

- All submissions must be made through Google Classroom.

- Submit all project files in a single ZIP folder named 25I-XXXX_25I-XXXX_Section.zip. The naming convention must be followed strictly.

- A group of up to **2 members** is allowed.

- Only one group member will submit the project on behalf of the entire group.
- Students are responsible for checking their ZIP files for corruption, viruses, missing files, and submission errors.

- Both group members must contribute to the project and understand the complete implementation.

- All major data structures and algorithms must be implemented from scratch. Built-in implementations of trees, BSTs, AVL trees, and heaps are not allowed.

- Copying code from other groups is strictly prohibited.

- The submitted project must be functional, and executable.

- Both group members must be present for the project demonstration and viva voce.
- Be prepared for a viva or any follow-up evaluation after project submission.
- Late submissions will not be accepted.

**Deliverables**

1\) Complete source code with a README file (run instructions, and game controls). 2) Unit test suite covering all major data structures and algorithms, along with a test report.

3\) Written project report (8–12 pages) including system architecture, algorithmic complexity analysis, benchmarking results (Section 10), GUI screenshots, screenshots of all four AVL rotation types, and demonstrations of heap construction, extraction, heapify, and the final sorted result.

4\) Level data files and a short guide explaining how to add new levels.

## 1. Overview

Vault Heist is a GUI puzzle-strategy game in which the player is a thief breaking into a vault whose rooms form a tree. The tree is the map. Every step through a room raises an alarm meter, so tree shape (height, balance, ordering) directly decides whether the player escapes. The player learns data structures by using them: reading traversals, searching a BST, deleting nodes, repairing imbalance with AVL rotations, and finally organising and sorting treasures with a Max Heap and Heap Sort. Beyond the standard operations, it requires tree reconstruction from traversals, order-statistic augmentation (subtreeSize), an event-driven architecture that separates data structures from the GUI, animated rotations, undo/replay, a complete Heap Sort campaign act, and a benchmarking report.

### 1.1 Project Goals

- Implement a general binary tree, a BST, an AVL tree and a Max Heap from scratch (no library tree or priority-queue containers).

- Build a GUI where every operation is visualised as a step-by-step animation.
- Design the system so the data structures are independent of the GUI and fully unit-testable.
- Measure and explain the performance difference between BST and AVL, and the complexity of Heap Sort.

### 1.2 Scope

Single-player campaign (4 acts, 20 levels), endless mode, animation, undo/replay, scoring, save/load, benchmark report, Max Heap and Heap Sort with synchronised tree/array visualisation.

## 2. Game Design

### 2.1 Premise

The player must steal a target treasure and escape before the guards arrive. The vault is a tree of rooms. Each room holds one treasure with an integer value. The player always starts at the root (the entrance). In Act 4 the final vault is protected by a priority-based security mechanism that requires the player to organise and extract treasures using a Max Heap.

### 2.2 Core Mechanic: The Alarm Meter

- Entering a room (visiting a node) or comparing against a room's key costs 1 alarm step.
- Each level has an alarm capacity equal to ceil(1.5 × optimalSteps) + 2. When the meter is full, the level is failed.

- A tall, lopsided tree makes the optimal path long, so the meter fills quickly. A balanced tree keeps paths short.

This turns tree height into something the player feels instead of only computing. In Act 4 the same meter punishes incorrect swaps, invalid extractions and excess steps during Heap Sort.

### 2.3 Game Flow

Main Menu → Campaign Map (Acts 1–4) → Level Briefing → Level Play → Result Screen (stars, steps, penalties) → Next Level

Main Menu → Endless Mode → Leaderboard (local file)

Main Menu → Sandbox (free-form tree / heap editor with all operations)

The Sandbox is important: it lets students and graders try any operation on any tree or heap and see the animations, independent of the campaign.

### 2.4 Scoring and Stars

| **Result**                                | **Stars** |
|-------------------------------------------|-----------|
| Steps ≤ optimal, no penalties             | 3         |
| Steps ≤ 1.25 × optimal, at most 1 penalty | 2         |
| Level completed within alarm capacity     | 1         |
| Alarm full                                | 0 (retry) |

**Penalties:** wrong traversal click (+1 alarm), wrong rotation choice (+2 alarm and -50 points), using a hint (-1 star cap), incorrect parent-child comparison or invalid swap in Act 4 (+1 alarm). Score = 1000 - 10 × alarmSteps - penalties + bonuses, floored at 0.

## 3. Level Specifications

### 3.1 Act 1: The Blueprint Wing (General Binary Tree)

Nodes carry arbitrary values with no ordering.

| **Level** | **Mission**                                          | **Concept**                                                      | **Player interaction**                                        | **Success condition**                         |
|-----------|------------------------------------------------------|------------------------------------------------------------------|---------------------------------------------------------------|-----------------------------------------------|
| 1.1       | Guards patrol in a given order. Predict their route. | Preorder, inorder, postorder, level order                        | Click rooms in the order the patrol will visit them           | Click sequence equals the traversal           |
| 1.2       | Decode the blueprint                                 | Reconstruction from preorder + inorder (and postorder + inorder) | Drag room tokens into tree slots                              | Built tree equalsthe hidden tree              |
| 1.3       | The alarm spreads from the leaves upward             | Height via postorder, diameter                                   | Enter each room's height before the alarm reachesthe root     | All heights correct; then report the diameter |
| 1.4       | Find where two escape routes meet                    | LCA, path between nodes                                          | Select two rooms, click the meeting room, then trace the path | Correct LCA and path length                   |
| 1.5       | Mirror the vault                                     | Tree inversion, recursion                                        | Swap children by clicking nodes bottom-up (or press Auto)     | Tree equals mirror of original                |

### 3.2 Act 2: The Sorted Wing (BST)

Smaller keys go left, larger go right. No duplicates (duplicates are rejected with a visible message).

| **Level** | **Mission**        | **Concept**                           | **Notes**                                                                                                                               |
|-----------|--------------------|---------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| 2.1       | Search heist       | BST search                            | Navigate left/right at each room; alarm counts comparisons                                                                              |
| 2.2       | Stash loot         | BST insert                            | Player chooses the path; the new room appears at the correct leaf position                                                              |
| 2.3       | Steal and collapse | Delete: leaf, one child, two children | Player must pick which case applies, then watch the animation. Two-child case highlights the inorder successor and shows the value swap |

| 2.4 | Range raid | Range query \[lo, hi\] | GUI greys out skipped subtrees; score rewards pruning |
|-----|------------|------------------------|-------------------------------------------------------|

| 2.5 | Boss: The Lazy Architect | Degeneration | Deliveries arrive in sorted order, the vault becomes a corridor, height reaches n. Player must complete a search under a shrinking alarm capacity. Ends with a cutscene explaining why balancing is needed |
|-----|--------------------------|--------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

### 3.3 Act 3: The Architect's Wing (AVL)

| **Level** | **Mission**                     | **Concept**               | **Notes**                                                                                                                                 |
|-----------|---------------------------------|---------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|
| 3.1       | Read the badges                 | Balance factor            | Every room shows a badge. Green means -1, 0 or 1. Red means violation. Player clicks all unbalanced rooms             |
| 3.2       | Manual repair, single rotations | LL and RR                 | Player picks the pivot and rotation by dragging; wrong choice triggers a penalty                                                          |
| 3.3       | Manual repair, double rotations | LR and RL                 | Player must identify the case first, then perform two rotations                                                                           |
| 3.4       | Auto mode and prediction        | Insert with rebalancing   | The vault repairs itself with animation. Before each insert, the player predicts which rotation (if any) will occur                       |
| 3.5       | Boss: Order-statistic heist     | subtreeSize, kth, rank    | Task: steal the k-th most valuable treasure in O(log n) steps. The boss validates sizes after every rotation, and a wrong size is flagged |
| Endless   | Survival mode                   | Mixed inserts and deletes | Stream of operations. Score is survival time while the tree stays valid. Difficulty ramps up the operation rate                           |

### 3.4 Act 4: The Heap Fortress (Max Heap & Heap Sort)

**Story and Gameplay**

After successfully completing the AVL Balance Chamber, the thief discovers the location of the final treasure vault. However, the vault is protected by an advanced priority-based security mechanism. Unlike the previous vaults, this mechanism does not require searching for a particular key or balancing a search tree. Instead, the player must organise the treasures according to their values using a Max Heap. The vault contains multiple treasures, each represented by a numerical value. The player must construct a valid Max Heap, repeatedly extract the maximum element, restore the heap property, and complete the sorting process before the alarm reaches its limit. The final objective is to successfully execute Heap Sort and unlock the final treasure vault.

**Gameplay Mechanics**

The player interacts with two synchronised representations:

- **Heap Tree View** – Displays the binary tree representation of the Max Heap.
- **Array View** – Displays the underlying array representation, including the active heap and sorted region.

Whenever the player performs a swap, the corresponding elements are highlighted in both

representations. The root contains the maximum element; every parent must be ≥ its existing children; the maximum is extracted by swapping the root with the last element of the active heap; the active heap size is reduced after extraction; heapify restores the Max Heap property; the process continues until the array is completely sorted.

| **Level**                           | **Mission**                       | **Concept**                                                          | **Player interaction / success**                                                                                                   |
|-------------------------------------|-----------------------------------|----------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| 4.1 – Activate the Vault            | Organise the treasure             | Complete binary tree; Max Heap property; bottom-up heap construction | Player selects or performs valid swaps to build a Max Heap. Success when every parent is ≥ its children.        |
| 4.2 – Repair the Security Locks     | Restore the mechani sm            | Heapify; sift-up and sift down                                       | Player repairs a deliberately disturbed heap. Success when the heap property is restored.                                          |
| 4.3 – Extract the Greatest Treasure | Remove the highest-value treasure | Extract-max;root/last swap; reduce heap size; heapify                | Player repeatedly extracts the root and restores the remaining heap. Success when each extracted maximum enters the sorted region. |
| 4.4 – Race Against the Guards       | Beat the alarm                    | Heap Sort steps and complexity                                       | Player completes multiple extraction/heapify iterations under an alarm/operation limit.                                            |
| 4.5 – Final Vault Boss              | Unlock the final vault            | Complete Heap Sort workflow                                          | Player sorts a new array independently. The final door opens only when the array is correctly sorted in ascending order.           |

## 4. System Architecture

### 4.1 Guiding Principle

The data structures know nothing about the GUI. Every operation returns a list of events describing what happened. The GUI plays events back as animation. This gives testability, undo, replay and hints with almost no extra work.

Model (pure data structures) → Event Log (list of events per operation) → Animation Controller (timing, queue, scrubber) → Renderer (draws nodes, edges, badges, array cells) Input (mouse/keyboard) → Level Logic → calls Model operation → new Event Log

### 4.2 Layers and Modules

| **Module**           | **Responsibility**                                                      | **Depends on** |
|----------------------|-------------------------------------------------------------------------|----------------|
| TreeNode, BinaryTree | General tree, traversals, reconstruction, LCA, diameter, mirror         | none           |
| BST                  | Ordered insert, search, delete, successor, predecessor, range, min, max | TreeNode       |

| AVLTree (extends BST) | Height, balance factor, rotations, size, kth, rank                                 | BST                |
|-----------------------|------------------------------------------------------------------------------------|--------------------|
| MaxHeap               | Array-based Max Heap, build, siftUp/siftDown, extractMax, heapSort, isMaxHeap      | none (array)       |
| Validator             | Checks invariants (ordering, heights, balance, sizes, parent links, heap property) | trees, heap        |
| EventLog              | Records, stores, reverses events                                                   | none               |
| LayoutEngine          | Computes (x, y) for every node; also lays out array cells                          | trees, heap        |
| AnimationController   | Plays events with easing, pause, step, speed                                       | EventLog, renderer |
| Renderer              | Draws nodes, edges, badges, highlights, array view                                 | LayoutEngine       |
| LevelManager          | Level definitions, generation, win/lose checks                                     | all model modules  |
| ScoreSystem           | Alarm meter, penalties, stars, leaderboard                                         | LevelManager       |
| SaveManager           | Preorder serialisation with null markers; heap array serialisation                 | trees, heap        |
| UIManager             | Menus, HUD, dialogs, input routing                                                 | everything above   |

### 4.3 Event Model

Every event has a type, the affected node / array indices, and enough data to undo it.

| **Event**                    | **Payload**                             | **Visual effect**                                                            |
|------------------------------|-----------------------------------------|------------------------------------------------------------------------------|
| VISIT                        | nodeId                                  | Node pulses, alarm +1                                                        |
| COMPARE                      |                                         | nodeId, key, result (\<, =, \>) Arrow shows chosen direction                 |
| INSERT                       | nodeId, parentId, side                  | Node grows in at its slot                                                    |
| UNLINK                       | nodeId, parentId, side                  | Node fades and edge retracts                                                 |
| SWAP_VALUE                   | nodeA, nodeB                            | Values slide between nodes (delete two child case)                           |
| ROTATE                       | type (LL, RR, LR, RL), pivotId          | Nodes tween to new positions                                                 |
| UPDATE_HEIGHT / UPDATE_SIZE  | nodeId, old, new                        | Badge / size label flickers                                                  |
| BALANCE_ALERT                | nodeId, balanceFactor                   | Badge turns red                                                              |
| HIGHLIGHT_SUBTREE            | nodeId, mode (visited, skipped, result) | Subtree tinted                                                               |
| HEAP_BUILD                   | array snapshot / start indices          | Highlight active region before construction                                  |
| HEAP_SWAP                    | indexA, indexB, reason                  | Animate the two array cells and corresponding tree nodes swapping            |
| HEAPIFY_START / HEAPIFY_DONE | startIndex, heapSize                    | Highlight active heap region; indicate restoration                           |
| EXTRACT_MAX                  |                                         | value, rootIndex, heapSize Highlight the root as the maximum being extracted |

| MARK_SORTED        | index, value     | Move the extracted value into the visually distinct sorted region |
|--------------------|------------------|-------------------------------------------------------------------|
| HEAP_SIZE_CHANGE   | oldSize, newSize | Update heap-size indicator                                        |
| HEAP_SORT_COMPLETE | final array      | Celebrate completion, unlock vault                                |
| DONE               | summary          | Ends the batch                                                    |

### 4.4 Undo and Replay

Every event stores the inverse data (for example ROTATE stores the pre-rotation links and old heights/sizes; HEAP_SWAP stores the two indices and previous values). Undo applies the inverse events in reverse order. Replay re-runs the stored log through the animator with a scrubber bar. Alternative for simplicity: snapshot the tree / array by serialisation before each player action and restore on undo. Implement snapshots first, then upgrade to inverse events if time allows.

## 5. Data Structure Specification

### 5.1 Node Definitions

TreeNode { id, value, left, right, parent } // Act 1 and 2

BSTNode : TreeNode with key (integer), treasure payload

AVLNode : BSTNode + height (leaf = 1), size (leaf = 1)

MaxHeap : array-based; no explicit node objects required, but tree view is derived from parent/child index arithmetic.

Use unique id values so the GUI can track nodes across rotations and value swaps. Parent pointers are optional but recommended because they simplify successor and the layout of rotated subtrees. If used, the validator must check them.

### 5.2 BinaryTree API (Act 1)

| **Function**                             | **Description**                                                  |
|------------------------------------------|------------------------------------------------------------------|
| preorder, inorder, postorder, levelOrder | Return node id lists (recursive and iterative versions for each) |
| build(pre, in)                           | Reconstruct from preorder + inorder                              |
| build(post, in)                          | Reconstruct from postorder + inorder                             |
| height(node)                             | Edges or nodes (pick one convention and document it)             |
| diameter()                               | Longest path between any two nodes                               |
| lca(a, b)                                | Lowest common ancestor in a general binary tree                  |
| pathTo(node)                             | Root-to-node path                                                |
| mirror()                                 | Swap children recursively                                        |
| isSameTree, isSymmetric                  | Structural comparison (used for win checks)                      |

### 5.3 BST API (Act 2)

| **Function** | **Description**                     |
|--------------|-------------------------------------|
| insert(key)  | Standard insert, rejects duplicates |

| search(key)            | Returns node and emits COMPARE events                              |
|------------------------|--------------------------------------------------------------------|
| delete(key)            | Three cases (leaf, one child, two children)                        |
| findMin, findMax       | Leftmost and rightmost                                             |
| successor, predecessor | Using parent pointers or a top-down search                         |
| rangeQuery(lo, hi)     | Inorder with pruning; emits HIGHLIGHT_SUBTREE for skipped subtrees |
| kthSmallest(k)         | Inorder counter (plain BST version, O(n))                          |
| lcaBST(a, b)           | Uses ordering                                                      |

**Delete cases:** (1) Leaf: unlink, emit UNLINK. (2) One child: bypass the node by linking its parent directly to its child. (3) Two children: find the inorder successor (minimum of the right subtree), emit SWAP_VALUE, then delete the successor. Boss level generator: inserting keys 1, 2, 3, …, n produces a right-leaning chain of height n.

### 5.4 AVL API (Act 3)

| **Function**            | **Description**                               |
|-------------------------|-----------------------------------------------|
| insert(key)             | BST insert, then rebalance while returning up |
| delete(key)             | BST delete, then rebalance while returning up |
| rotateLeft, rotateRight | Primitive rotations, update height and size   |
| rebalance(node)         | Choose LL, RR, LR or RL                       |
| balanceFactor(node)     | height(left) - height(right)                  |
| kth(k)                  | k-th smallest using size                      |
| rank(key)               | Number of keys smaller than or equal to key   |
| countInRange(lo, hi)    | rank(hi) - rank(lo - 1)                       |
| isBalanced()            | Validator, checks every node                  |

### 5.5 MaxHeap and Heap Sort API (Act 4)

The MaxHeap class manages the heap using a zero-based array representation. For a node at index i: parent = (i - 1) // 2, left child = 2i + 1, right child = 2i + 2.

| **Operation**                     | **Description**                                                                         |
|-----------------------------------|-----------------------------------------------------------------------------------------|
| buildMaxHeap(A)                   | Convert an array into a Max Heap using bottom-up heap construction                      |
| siftDown(i, heapSize) / heapify() | Move an element downward until the Max Heap property is restored                        |
| siftUp(i)                         | Move an inserted element upward until the Max Heap property is restored                 |
| extractMax()                      | Remove and return the root; restore the heap property                                   |
| insert(value)                     | Insert a new element while maintaining the heap property                                |
| heapSort(A)                       | Build a Max Heap, repeatedly swap root with last active element, shrink heap, sift down |
| isMaxHeap()                       | Checks whether the array satisfies the Max Heap property                                |

### 5.6 Validator

Run after every operation in debug mode and in all unit tests. It must verify:

- BST ordering (all keys in the left subtree are smaller, all in the right are larger).
- Stored height equals recomputed height; stored size equals recomputed size.
- \|balanceFactor\| ≤ 1 for every AVL node.

- Parent pointers are consistent (if used).

- No cycles and no shared nodes (node count equals traversal count).

- Max Heap property: every parent ≥ its existing children; active heap size and sorted region are consistent.

## 6. GUI Specification

### 6.1 Screens

| **Screen**             | **Contents**                                                                                                                      |
|------------------------|-----------------------------------------------------------------------------------------------------------------------------------|
| Main menu              | Campaign, Endless, Sandbox, Leaderboard, Settings, Quit                                                                           |
| Campaign map           | Four acts with level nodes, stars earned, locked levels                                                                           |
| Level screen           | Tree / heap canvas, HUD (alarm meter, score, objective), toolbar, event log panel                                                 |
| Result screen          | Stars, steps versus optimal, penalties, retry and next                                                                            |
| Sandbox                | Operation panel (insert, delete, search, range, rotate, heap ops), tree type switch (Binary, BST, AVL, MaxHeap), random generator |
| Report view (optional) | Charts of height versus n for BST and AVL; Heap Sort timings                                                                      |

**Shared components.** The following components appear on more than one screen and are therefore specified once here and instantiated per screen:

- **Tree / heap canvas** — the main visualisation area. Acts 1–3 render a binary tree; Act 4 renders the Max Heap tree view synchronised with the array view. Layout is produced by the LayoutEngine and drawn by the Renderer.

- **HUD** — alarm meter, score, objective, steps taken / max steps

- **Event Log panel** — a human-readable list of events emitted by the model for the current operation. The same component is reused on the Level screen and in Act 4; Act 4 populates it with the heap specific events (HEAP_BUILD, HEAP_SWAP, HEAPIFY_START/DONE, EXTRACT_MAX, MARK_SORTED, HEAP_SIZE_CHANGE, HEAP_SORT_COMPLETE).

- **Operation Panel** — the toolbar of actions available for the current tree type. On the Level screen it is level-driven, in the Sandbox it exposes all operations, and in Act 4 it exposes the heap operations, item 5.

- **Toolbar** — Undo, Replay, Reset, Hint, speed slider

### 6.2 Act 4 GUI – The Heap Fortress

The GUI for Act 4 follows the existing fantasy vault theme while introducing a dedicated interface for Max Heap and Heap Sort visualisation. The main gameplay screen contains:

1\. **Level Information Panel** – Current act (Act 4 – The Heap Fortress), level, objective, steps taken

/ max steps.

2\. **Alarm Meter** – Remaining security tolerance; incorrect operations and unnecessary steps increase the alarm.

3\. **Heap Tree Visualisation** – Max Heap as a binary tree; root highlighted as maximum; parent child relationships clear; selected nodes highlighted during comparisons; swaps animated; invalid relationships indicated.

4\. **Array Visualisation** – Underlying array distinguishing active heap elements, extracted maxima, and sorted region.

5\. **Operation Panel** – Build Max Heap, Heapify, Extract Maximum, Heap Sort, Undo, Replay, Reset Level.

6\. **Event Log** – Sequence of operations (comparisons, swaps, extraction, heapify, size changes). 7. **Instruction Panel** – Hints and instructions based on the current level.

**Synchronisation rule.** The Heap Tree View and Array View remain synchronised. When the player swaps two elements in the array, the corresponding nodes in the tree update as well; this is driven by the HEAP_SWAP event. Extracted elements are visually separated from the active heap.

**Accessibility rule.** Colour is never the only signal: the active heap and sorted region are labelled, and icons, outlines or text labels are used in addition to colour, consistent with the colour-code.

**Completion.** After successfully completing Act 4, the completion screen shows the final score, steps, penalties, the sorted array and the achievement badge "Final Vault Unlocked", emitted via HEAP_SORT_COMPLETE.

### 6.3 Interaction Design (general)

- Click a room: select, or answer a traversal question.

- Hover a room: tooltip with key, height, balance factor, size, depth.

- Drag a room token: reconstruction levels. Drag a subtree to a rotation pivot: manual AVL rotation.

- Mouse wheel and drag background: zoom and pan.

- Keyboard: Space (play/pause), Left/Right (step event), Z (undo), H (hint), Esc (menu).
- Colour code: green (balanced), yellow (balance factor ±1 warning), red (violation), blue (current visit), grey (skipped by pruning), gold (target). Never rely on colour alone.

## 7. Level Generation and Data

- Seeded random: every level has a seed so that results are reproducible and gradable.
- Act 1 trees: random shapes with 6 to 15 nodes; for reconstruction levels ensure distinct values.
- Act 2 trees: insert a random permutation; difficulty controlled by tree size and forced skewness.
- Act 3 trees: hand-crafted and generated cases that guarantee each rotation type appears at least twice.

- Act 4 arrays: random, reverse-sorted, already-heap, and duplicate-heavy inputs of controlled size; levels ensure each heap operation is exercised.

- Optimal steps are computed by the model, never typed in by hand.

- Levels are described in a data file (JSON), so adding a level requires no code change.

## 8. Save and Load

- Serialise trees using preorder with null markers, for example 50 30 \# \# 70 60 \# \# \#.
- Serialise heaps as the underlying array plus the current active heap size.

- Save file stores campaign progress, stars, current endless-mode tree/heap and random seed.
- AVL height and size are recomputed on load rather than stored; the validator runs before the loaded game starts.

## 9. Testing Strategy

### 9.1 Unit Tests (model only, no GUI)

| **Area**         | **Cases**                                                                                                                          |
|------------------|------------------------------------------------------------------------------------------------------------------------------------|
| Traversals       | Empty tree, single node, skewed, full tree; recursive equals iterative                                                             |
| Reconstruction   | Round trip: build tree, take traversals, rebuild, compare structure                                                                |
| BST delete       | Leaf, one left child, one right child, two children, root deletion, delete missing key                                             |
| AVL rotations    | One test per case: LL, RR, LR, RL; deletion cases including balance factor 0 on the heavy child                                    |
| Order statistics | kth and rank agree with a sorted array after every operation                                                                       |
| Duplicates       | Rejected consistently                                                                                                              |
| Heap property    | After build, insertion, extraction and heapify, verify every parent its existing children                                          |
| Build Max Heap   | Empty, one element, duplicates, already-heap, reverse-sorted, random arrays                                                        |
| Extract maximum  | Returned value is the maximum; remaining active region is still a valid Max Heap                                                   |
| Heap Sort        | Compare output with a trusted sorted reference for empty, singleton, duplicate, negative, sorted, reverse-sorted and random inputs |

### 9.2 GUI Tests (manual checklist)

- Every level (including 4.1–4.5) can be completed, failed and retried.

- Zoom, pan and fit work on a 200-node tree / large heap without frame drops (target 60 fps).
- Animation never leaves nodes or array cells at stale positions after undo.

- Tree view and array view stay synchronised throughout Heap Sort.

## 10. Benchmark and Analysis Plan

Run benchmarks without the GUI and without event recording (provide a flag to disable events).

| **Experiment**    | **Variables**                                              | **Metrics**                                         |
|-------------------|------------------------------------------------------------|-----------------------------------------------------|
| Height comparison | n = 100, 1,000, 10,000; sorted vs random input; BST vs AVL | Final height                                        |
| Search cost       | Same trees, 10,000 random searches                         | Average comparisons and time                        |
| Rotation count    | AVL insert and delete, random vs sorted                    | Average rotations per operation, maximum per delete |

| Order statistics   | kth on AVL vs inorder scan on BST                               | Time for 1,000 queries                                 |
|--------------------|-----------------------------------------------------------------|--------------------------------------------------------|
| Heap construction  | n = 100, 1,000, 10,000; random and ordered inputs               | Build time; confirm O(n) bottom-up construction        |
| Heap Sort          | Same n; random,sorted, reverse-sorted, duplicate heavy          | Execution time, comparisons, swaps; explain O(n log n) |
| Sorting comparison | Heap Sort versus other sorting algorithms covered in the course | Runtime, swaps/comparisons, auxiliary space            |

