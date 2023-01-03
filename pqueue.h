#ifndef PQUEUE_H_
#define PQUEUE_H_

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

template <typename T, typename C = std::less<T> >
class PQueue {
public:
    // Constructor
    PQueue() {}
    // Return number of items in priority queue
    size_t Size();
    // Return top of priority queue
    T& Top();
    // Remove top of priority queue
    void Pop();
    // Insert item and sort priority queue
    void Push(const T &item);

private:
    std::vector<T> items;
    size_t cur_size = 0;
    C cmp;

    // Helper methods for indices
    size_t Root() {
        return 0;
    }
    size_t Parent(size_t n) {
        return (n - 1) / 2;
    }
    size_t LeftChild(size_t n) {
        return 2 * n + 1;
    }
    size_t RightChild(size_t n) {
        return 2 * n + 2;
    }

    // Helper methods for node testing
    bool HasParent(size_t n) {
        return n != Root();
    }
    bool IsNode(size_t n) {
        return n < cur_size;
    }

    // Helper methods for restructuring
    void PercolateUp(size_t n);
    void PercolateDown(size_t n);

    // Node comparison
    bool CompareNodes(size_t i, size_t j);
};

// To be completed below
// Return number of items in priority queue
template <typename T, typename C>
size_t PQueue<T,C>::Size() {
    return cur_size;
}
// Return top of priority queue
template <typename T, typename C>
T& PQueue<T,C>::Top() {
    if (!Size())
        throw std::underflow_error("Empty priority queue!");
    return items[Root()];
}
// Remove top of priority queue
template <typename T, typename C>
void PQueue<T,C>::Pop() {
    if (!Size())
        throw std::underflow_error("Empty priority queue!");
    items[Root()] = std::move(items[--cur_size]);
    items.pop_back();
    PercolateDown(Root());
}
// Insert item and sort priority queue
template <typename T, typename C>
void PQueue<T,C>::Push(const T &item) {
    // Insert at the end
    items.push_back(item);
    cur_size++;
    // Percolate up
    PercolateUp(cur_size-1);
}

// Helper methods for restructuring
template <typename T, typename C>
void PQueue<T,C>::PercolateUp(size_t n) {
    while (HasParent(n) && CompareNodes(n, Parent(n))) {
        std::swap(items[Parent(n)], items[n]);
        n = Parent(n);
    }
}
template <typename T, typename C>
void PQueue<T,C>::PercolateDown(size_t n) {
    // While node has at least one child (if one, necessarily on the left)
    while (IsNode(LeftChild(n))) {
        // Consider left child by default
        size_t child = LeftChild(n);
        // If right child exists and smaller 
        // than left child, then consider right child
        if (IsNode(RightChild(n)) && CompareNodes(RightChild(n), LeftChild(n)))
            child = RightChild(n);
        // Exchange the smallest child with node 
        // to restore heap-order if necessary
        if (CompareNodes(child, n))
            std::swap(items[child], items[n]);
        else
            break;
        // Do it again, one level down
        n = child;
    }
}

// Node comparison
template <typename T, typename C>
bool PQueue<T,C>::CompareNodes(size_t i, size_t j) {
    return cmp(items[i], items[j]);
}


#endif  // PQUEUE_H_
