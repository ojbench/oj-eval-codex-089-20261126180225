#include "src.hpp"
#include <cassert>
#include <cstdio>
#include <random>
#include <utility>
#include <vector>

int main() {
  std::mt19937 random(2199);
  for (int minimum : {1, 2, 4}) {
    for (int exponent = 0; exponent <= 10; ++exponent) {
      const int ram = minimum * (1 << exponent);
      sjtu::BuddyAllocator allocator(ram, minimum);
      std::vector<bool> used(ram, false);
      std::vector<std::pair<int, int>> live;
      for (int step = 0; step < 10000; ++step) {
        if (!live.empty() && random() % 3 == 0) {
          const int index = random() % live.size();
          const auto block = live[index];
          allocator.free_at(block.first, block.second);
          for (int i = block.first; i < block.first + block.second; ++i)
            used[i] = false;
          live[index] = live.back();
          live.pop_back();
          continue;
        }
        const int size = minimum * (1 << (random() % (exponent + 1)));
        const bool fixed = random() % 2;
        const int address = (random() % (ram / size)) * size;
        int expected = -1;
        for (int start = fixed ? address : 0; start + size <= ram;
             start += size) {
          bool available = true;
          for (int i = start; i < start + size; ++i)
            if (used[i]) available = false;
          if (available) { expected = start; break; }
          if (fixed) break;
        }
        const int actual = fixed ? allocator.malloc_at(address, size)
                                 : allocator.malloc(size);
        assert(actual == expected);
        if (actual != -1) {
          live.emplace_back(actual, size);
          for (int i = actual; i < actual + size; ++i) used[i] = true;
        }
      }
      for (const auto &block : live)
        allocator.free_at(block.first, block.second);
      assert(allocator.malloc(ram) == 0);
      assert(allocator.malloc(minimum) == -1);
      allocator.free_at(0, ram);
      assert(allocator.malloc_at(0, ram) == 0);
    }
  }
  sjtu::BuddyAllocator large(262144, 1);
  for (int i = 0; i < 262144; ++i) assert(large.malloc(1) == i);
  assert(large.malloc(1) == -1);
  for (int i = 0; i < 262144; ++i) large.free_at(i, 1);
  assert(large.malloc(262144) == 0);
  std::puts("All allocator tests passed.");
}
