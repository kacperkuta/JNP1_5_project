#ifndef INSERTION_ORDERED_MAP_H
#define INSERTION_ORDERED_MAP_H

#include <cstdlib>
#include <functional>
#include <memory>

#define MAP insertion_ordered_map

template <class K, class V, class Hash = std::hash<K>>
class MAP {
public:
    struct node;

    using node_ptr = std::shared_ptr<node>;

    typedef struct node {
        size_t hash;
        K key;
        V val;
        node_ptr next;
        node_ptr link;
        node_ptr back_link;

        node (const node& other, node_ptr previous)
                : hash(other.hash)
                , key(other.key)
                , val(other.val)
                , next(nullptr)
                , link(nullptr)
                , back_link(previous)
        {}

        node (K key, V val, node_ptr next, node_ptr link,
              node_ptr back_link, size_t hash)
                : key(key)
                , val(val)
                , next(next)
                , link(link)
                , back_link(back_link)
                , hash(hash)
        {}

    } node;

    MAP ()
            : my_size(0)
            , tab(new node_ptr[16])
            , given_reference(false)
            , mod(16)
    {}

    MAP (const MAP& other) : mod(other.mod) {
        my_size = other.my_size;
        given_reference = false;
        if (!other.given_reference) {
            tab = other.tab;
        } else {
            tab = new node_ptr[other.mod];
            hashedMapCopy(tab, other.mod, false);
        }
    }

    MAP (MAP&& other) noexcept
            : my_size(other.my_size)
            , tab(other.tab)
            , given_reference(other.given_reference)
            , mod(other.mod)
    {
        other.tab = nullptr;
        other.my_size = 0;
    }

    size_t size() noexcept {
        return my_size;
    }

    bool empty() const noexcept {
        return my_size == 0;
    }

    void insert(const K& k, const V& v) {
        checkSize();
        if (tab.use_count() > 1) {
            tab_ptr t = new node_ptr[mod];
            hashedMapCopy(t, mod, false);
            tab = t;
        }
        node_ptr n = findNode(k);
        if (n != nullptr) {
            n -> link -> back_link = n -> back_link;
            n -> back_link -> link = n -> link;
            n -> link = end();
            n -> back_link = (*end()) -> back_link;
            (*(end())) -> back_link -> link = n;
            (*(end())) -> back_link = n;
        } else {
            n = new node(k, v, nullptr, *(end()),
                         (*(end())) -> back_link, Hash(k)%mod);
            (*(end())) -> back_link -> link = n;
            (*(end())) -> back_link = n;
            addNode(n, n -> hash, tab);
        }
    }

    V& at(K const& k) {
        size_t kHash = Hash(k) % mod;
        node_ptr n = tab[kHash];
        while (n != nullptr) {
            if (n -> key == k) {
                given_reference = true;
                return *(n.get());
            }
        }
        throw "lookup_error";
    }

    V const& at(K const& k) const {
        return at(k);
    }

    void clear() {
        tab_ptr tab = new node_ptr[16];
        my_size = 0;
        mod = 16;
        given_reference = false;
    }

    bool contains(const K& k) const noexcept {
        return findNode(k) != nullptr;
    }

    class iterator {
    public:
        iterator(): ptr_(0) {}

        //ten konstruktor kopiujący przyjmuje jako argument node_ptr, a nie iterator
        //Czy to na pewno zadziała? Bo begin() i end() zwracają obiekt klasy iterator
        iterator(const iterator& it): ptr_(it.ptr_) {}

        const std::pair<K&, V&> operator* () {
            return std::make_pair(ptr_ -> key, ptr_ -> val);
        }

        iterator& operator++() {
            ptr_ = ptr_ -> link;
            return *this;
        }

        bool operator==(const iterator& it) {
            return ptr_ == it.ptr_;
        }

        bool operator!=(const iterator& it) {
            return !(*this == it);
        }

    private:
        node_ptr ptr_;
    };

    iterator& begin() {
        return begin_;
    }

    iterator& end() {
        return end_;
    }

private:

    using tab_ptr = std::shared_ptr<node_ptr[]>;

    size_t my_size;
    //default value of mod is 16.
    // It increases twice when my_size is greater than 75% of mod.
    // Decreases twice when my_size is lower than 10% of mod.
    size_t mod;
    tab_ptr tab;
    bool given_reference;
    iterator begin_;
    iterator end_;


    void addNode(node_ptr& node, size_t hash, tab_ptr& new_tab) {
        node_ptr n = new_tab[hash];
        if (n == nullptr) {
            new_tab[hash] = node;
        } else {
            while (n -> next != nullptr) {
                n = n -> next;
            }
            n -> next = node;
        }
        node -> next = nullptr;
    }

    //simpleCopyOfMap requires table of size at least equal to other.mod
    //this function seems to be redundant
    /*  void simpleMapCopy(const MAP& other, tab_ptr& new_tab) {
          node_ptr previous = nullptr;
          for (iterator it = iterator::begin(); it != iterator::end(); it++) {
              node_ptr new_node(*it, previous);
              if (previous != nullptr)
                  previous -> link = new_node;
              else
                  iterator::setBegin(new_node);
              addNode(new_node, new_node -> hash, new_tab);
              previous = new_node;
          }
          previous -> link = node(previous, previous);
          previous -> link -> link = nullptr;
          iterator::setEnd(previous -> link);
      }
  */
    void createResizedMap(unsigned p, unsigned q) {
        mod *= p;
        mod /= q;
        tab_ptr tab_n = new node_ptr [mod];
        //simpleMapCopy(*this, tab_n);
        hashedMapCopy(tab_n, mod, true);

        tab.reset();
        tab = tab_n;
    }

    void checkSize() {
        if (my_size >= mod*3/4) {
            createResizedMap(2, 1);
        } else if (my_size <= mod/10) {
            createResizedMap(1, 2);
        }
    }

    //Requires table of size at least equal to mod. new_tab is filled in with
    //the nodes from actual tab. Linkage is saved. First and last element are
    //correctly set, so begin() and end() methods works properly for the new_tab
    //after function calling.
    void hashedMapCopy(tab_ptr& new_tab, size_t mod, bool rehash) {
        node_ptr previous = nullptr;
        for (iterator it = iterator::begin(); it != iterator::end(); it++) {
            node_ptr new_node = new node(*it, previous);
            if (rehash) {
                new_node -> hash = Hash(new_node -> key)%mod;
            }
            if (previous != nullptr)
                previous -> next = new_node;
            else
                iterator::setBegin(new_node);
            addNode(new_node, new_node -> hash, new_tab);
            previous = new_node;
        }
        previous -> link = nullptr;
        iterator::setEnd(nullptr);
    }

    node_ptr findNode(const K& k) const noexcept {
        size_t kHashed = Hash(k)%mod;
        node_ptr n = tab[kHashed];
        while (n != nullptr) {
            if (n -> key == k) {
                return n;
            } else {
                n = n -> next;
            }
        }
        return nullptr;
    }

};

#endif //INSERTION_ORDERED_MAP_H