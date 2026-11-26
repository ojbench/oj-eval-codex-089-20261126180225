# Buddy allocator

`src.hpp` implements the required interface without STL containers. A complete
binary tree represents the aligned buddy blocks. Each node stores the largest
free buddy block in its range. A fully free node stores its range size, and a
fully occupied node stores zero. These uniform states are propagated to children
only when descending, so allocating or releasing a large block is inexpensive.

For `malloc`, descend into the left child whenever its largest free block can
satisfy the request; otherwise descend right. All valid requested blocks are
tree nodes because sizes and addresses are power-of-two aligned. Thus this
search returns the smallest valid address. `malloc_at` follows the requested
address and succeeds only when its entire block is free. `free_at` restores the
block and merges free buddies while updating ancestors.

With N = ram_size / min_block_size, each operation takes O(log N) time, with
O(N) initialization time and memory. The owned array is released by the
destructor; copying is disabled to avoid accidental shared ownership.

## Local verification

- Supplied public example passed.
- 330,000 deterministic randomized operations matched a byte-array reference
  across minimum block sizes 1, 2, and 4, including one-block memory regions.
- Allocating all 262,144 minimum blocks in order and freeing them restored the
  entire memory region.
- Tests passed under AddressSanitizer and UndefinedBehaviorSanitizer.

Run the regression test from the repository root:

```sh
g++ -std=c++14 -O2 -Wall -Wextra -Werror -I. tests/allocator_test.cpp -o /tmp/allocator_test
/tmp/allocator_test
```
