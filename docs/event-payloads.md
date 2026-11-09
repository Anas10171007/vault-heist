# Event payloads (contract)
Codes (EventType, Side, RotationType, HighlightMode, HeapReason) are int constants inside structs, not enums; the Event fields are plain ints.
Fields in order: type, idA, idB, side, key, oldValue, newValue, extra. Unused fields stay 0 and side stays NONE. Tree events use node ids; heap events use array indices. Id 0 means no node. Each VISIT and each COMPARE costs 1 alarm step.
| Type | idA | idB | side | key | oldValue | newValue | extra |
| --- | --- | --- | --- | --- | --- | --- | --- |
| VISIT | node | | | | | | |
| COMPARE | node | | | searched key | | | -1, 0, 1 (searched key <, =, > node key) |
| INSERT | new node | parent (0 for root) | LEFT or RIGHT | inserted key | | | |
| UNLINK | node | parent (0 for root) | LEFT or RIGHT | key of node | | | |
| SWAP_VALUE | node A | node B | | | value A had | value B had | |
| ROTATE | pivot (unbalanced node) | node that becomes subtree root | | | | | RotationType |
| UPDATE_HEIGHT | node | | | | old | new | |
| UPDATE_SIZE | node | | | | old | new | |
| BALANCE_ALERT | node | | | | | | balance factor |
| HIGHLIGHT_SUBTREE | subtree root | | | | | | HighlightMode |
| HEAP_BUILD | start index | heap size | | | | | |
| HEAP_SWAP | index A | index B | | | value at A before | value at B before | HeapReason |
| HEAPIFY_START | start index | heap size | | | | | |
| HEAPIFY_DONE | start index | heap size | | | | | |
| EXTRACT_MAX | root index | heap size before | | extracted value | | | |
| MARK_SORTED | index | | | value | | | |
| HEAP_SIZE_CHANGE | | | | | old size | new size | |
| HEAP_SORT_COMPLETE | array length | | | | | | |
| DONE | | | | | | | |