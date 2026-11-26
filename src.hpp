#ifndef SJTU_BUDDY_ALLOCATOR_HPP
#define SJTU_BUDDY_ALLOCATOR_HPP

namespace sjtu {

class BuddyAllocator {
public:
  BuddyAllocator(int ram_size, int min_block_size)
      : ram_size_(ram_size),
        largest_(new int[2 * (ram_size / min_block_size)]()) {
    largest_[1] = ram_size_;
  }

  ~BuddyAllocator() { delete[] largest_; }

  BuddyAllocator(const BuddyAllocator &) = delete;
  BuddyAllocator &operator=(const BuddyAllocator &) = delete;

  int malloc(int size) {
    if (largest_[1] < size) return -1;
    return allocate_first(1, ram_size_, 0, size);
  }

  int malloc_at(int addr, int size) {
    return allocate_at(1, ram_size_, 0, addr, size) ? addr : -1;
  }

  void free_at(int addr, int size) {
    release(1, ram_size_, 0, addr, size);
  }

private:
  int ram_size_;
  int *largest_;

  // Uniform nodes are lazy: their descendants may hold stale values.
  void split(int node, int length) {
    if (largest_[node] == length) {
      largest_[node * 2] = largest_[node * 2 + 1] = length / 2;
    } else if (largest_[node] == 0) {
      largest_[node * 2] = largest_[node * 2 + 1] = 0;
    }
  }

  void pull(int node, int length) {
    const int left = largest_[node * 2];
    const int right = largest_[node * 2 + 1];
    if (left == length / 2 && right == length / 2) {
      largest_[node] = length;
    } else {
      largest_[node] = left > right ? left : right;
    }
  }

  int allocate_first(int node, int length, int begin, int size) {
    if (length == size) {
      largest_[node] = 0;
      return begin;
    }
    split(node, length);
    const int half = length / 2;
    int result;
    if (largest_[node * 2] >= size) {
      result = allocate_first(node * 2, half, begin, size);
    } else {
      result = allocate_first(node * 2 + 1, half, begin + half, size);
    }
    pull(node, length);
    return result;
  }

  bool allocate_at(int node, int length, int begin, int addr, int size) {
    if (largest_[node] < size) return false;
    if (length == size) {
      largest_[node] = 0;
      return true;
    }
    split(node, length);
    const int half = length / 2;
    const bool right = addr >= begin + half;
    const bool success = allocate_at(node * 2 + right, half,
                                    begin + (right ? half : 0), addr, size);
    pull(node, length);
    return success;
  }

  void release(int node, int length, int begin, int addr, int size) {
    if (length == size) {
      largest_[node] = length;
      return;
    }
    split(node, length);
    const int half = length / 2;
    const bool right = addr >= begin + half;
    release(node * 2 + right, half, begin + (right ? half : 0), addr, size);
    pull(node, length);
  }
};

} // namespace sjtu

#endif
