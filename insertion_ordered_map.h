#include <cstdlib>
#include <functional>
#include <memory>

#define MAP insertion_ordered_map

template <class K, class V, class Hash = std::hash<K>>
class MAP {
public:
    MAP () : my_size(0), tab(new node_ptr[16]) {};

    MAP (const MAP& other) {
        my_size = other.my_size;
        tab = other.tab;
    }

    size_t size() noexcept {
        return my_size;
    }

private:

    typedef struct node {
        K key;
        V val;
        node * next;
        node * link;
        node * back_link;
    } node;

    using node_ptr = std::shared_ptr<node>;
    using tab_ptr = std::shared_ptr<node_ptr[]>;

    size_t my_size;
    tab_ptr tab;


    class iterator {
    public:
        iterator(): ptr_(0) {}

        iterator(node* it): ptr_(it) {}

        V operator* () { return ptr_ -> val; }

        iterator& operator++() {
            ptr_ = ptr_ -> link;
            return *this;
        }

        bool operator== ( const iterator& it ) {
            if (ptr_ == it.ptr_)
                return true;

            return false;
        }

        bool operator!= (const iterator& it) {
            return !(*this == it);
        }

        iterator& operator= (const iterator& it) {
            ptr_ = it.ptr_;

            return *this;
        }

        iterator& begin() {
            return begin_;
        }

        iterator& end() {
            return end_;
        }

        static void setBegin(node *begin) {
            begin_ = begin;
        }

        static void setEnd(node *end) {
            end_ = end;
        }

    private:
        node* ptr_;

        static node* begin_;

        static node* end_;
    };
};
