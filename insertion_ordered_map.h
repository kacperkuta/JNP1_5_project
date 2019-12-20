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

        node (const node_ptr& other, node_ptr previous)
                : hash(other -> hash)
                , key(other -> key)
                , val(other -> val)
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

        explicit node (size_t hash) : hash(hash) {}

    } node;

    MAP ()
            : my_size(0)
            , tab(new node_ptr[16])
            , given_reference(false)
            , mod(16)
    {
        node_ptr n(new node(0));
        n -> link = n;
        n -> back_link = n;
        end_= iterator(n);
    }

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
        end_ = other.end();
    }

    size_t size() noexcept {
        return my_size;
    }

    bool empty() const noexcept {
        return my_size == 0;
    }

    void erase(K const &k) {
        node_ptr n = findNode(k);

        if (n == nullptr) {
            throw "lookup_error";
        } else {
            checkSize();
            n->back_link->link = n->link;
            n->link->back_link = n->back_link;

            size_t kHash = Hash{}(k);
            node_ptr n2 = tab[kHash];
            if (n2 == n) {
                tab[kHash] = n->next;
            } else {
                while (n2->next && n2->next != n) {
                    n2 = n2->next;
                }
                n2->next = n2->next->next;
            }
            n.reset();
            --my_size;
            given_reference = false;
        }
    }

    size_t countNodesNotInMap(insertion_ordered_map const &other) {
        size_t sum = 0;

        //uzywam twojego at by policzyc o ile mam powiekszyc tablice
        for (auto it = other.begin(), end = other.end(); it != end; ++it) {
            size_t kHash = Hash(it -> second) % mod;
            node_ptr n = tab[kHash];
            while (n != nullptr) {
                if (n -> key == it -> first) {
                    ++ sum;
                }
            }
        }

        return sum;
    }

    void merge(insertion_ordered_map const &other) {
        size_t s = other.my_size;
        if (s > 0) {
            //Sprawdzam rozmiar potrzebny do wydluzenia tablicy
            if (countNodesNotInMap(*other) + my_size >= mod*3/4) {
                createResizedMap((countNodesNotInMap(*other) + my_size) / mod + 1 ,1);
            }

            node_ptr previous = end().getPtr();
            //dodaje te wartosci z kluczami z others ktore nie sa w this
            for (auto it = other.begin(), end = other.end(); it != end; ++it) {
                size_t kHash = Hash(it -> second) % mod;
                node_ptr n = tab[kHash];
                while (n != nullptr) {
                    if (n -> key == it -> first) {
                        node_ptr new_node(it.getPtr(), previous);
                        previous -> link = new_node;
                        addNode(new_node, new_node -> hash, tab);
                        previous = new_node;
                    }
                }
            }
            previous -> link = node(previous, previous);
            previous -> link -> link = nullptr;
            end_ = iterator(previous -> link);
        }
    }


    void insert(const K& k, const V& v) {

        checkSize();

        if (tab.use_count() > 1) {
            //std::cout << "test\n";
            tab_ptr t(new node_ptr[mod]);
            hashedMapCopy(t, mod, false);
            tab = t;
        }

        node_ptr n = findNode(k);

        if (n != nullptr) {
            n -> link -> back_link = n -> back_link;
            n->back_link->link = n->link;
            n->back_link = end().getPtr()->back_link;
            n -> link = end().getPtr();
            end().getPtr() -> back_link -> link = n;
            end().getPtr() -> back_link = n;
        } else {
            my_size++;
            node_ptr n2(new node(k, v, nullptr, end().getPtr(),
                         end().getPtr() -> back_link,
                         Hash{}(k)%mod));
            n2 -> link = end().getPtr();

            n2 -> back_link = end().getPtr() -> back_link;
            n2 -> back_link -> link = n2;
            n2 -> link -> back_link = n2;
            addNode(n2, n2 -> hash, tab);
        }
        assert(contains(k));
        given_reference = false;
    }

    V& at(K const& k) {
        node_ptr n = findNode(k);
        if (n == nullptr) {
            throw "lookup_error";
        } else {
            if (tab.use_count() > 1) {
                //std::cout << "test\n";
                tab_ptr t(new node_ptr[mod]);
                hashedMapCopy(t, mod, false);
                tab = t;
            }
            given_reference = true;
            return n -> val;
        }
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

        iterator(const iterator& it): ptr_(it.ptr_) {}

        explicit iterator(node_ptr& node) : ptr_(node) {};

        const std::pair<K&, V&> operator* () {
            return std::pair<K&, V&>(ptr_ -> key, ptr_ -> val);
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

        const node_ptr& getPtr() const {
            return ptr_;
        }

    private:

        node_ptr ptr_;

    };

    iterator& begin() {
        begin_ = iterator(end_.getPtr() -> link);
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
    iterator end_;
    iterator begin_;


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

    void createResizedMap(unsigned p, unsigned q) {
        mod *= p;
        mod /= q;
        tab_ptr tab_n(new node_ptr[mod]);
        hashedMapCopy(tab_n, mod, true);

        tab.reset();
        tab = tab_n;
    }

    void checkSize() {
        if (my_size >= mod*3/4) {
            createResizedMap(2, 1);
        } else if (mod >= 32 && my_size <= mod/10) {
            createResizedMap(1, 2);
        }
    }

    //Requires table of size at least equal to mod. new_tab is filled in with
    //the nodes from actual tab. Linkage is saved. First and last element are
    //correctly set, so begin() and end() methods works properly for the new_tab
    //after function calling.
    void hashedMapCopy(tab_ptr& new_tab, size_t mod, bool rehash) {
        node_ptr previous(new node(0));
        iterator end_it = iterator(previous);
        for (iterator it = begin(); it != end(); ++it) {
            node_ptr new_node(new node(it.getPtr(), previous));
            if (rehash) {
                new_node -> hash = Hash{}(new_node -> key)%mod;
            }
            previous -> link = new_node;
            new_node -> back_link = previous;
            addNode(new_node, new_node -> hash, new_tab);
            previous = new_node;
        }
        end_ = end_it;
        end_.getPtr() -> back_link = previous;
        previous -> link = end_.getPtr();
    }

    node_ptr findNode(const K& k) const noexcept {
        size_t kHashed = Hash{}(k)%mod;
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