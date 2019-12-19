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
        K key;
        V val;
        node_ptr next;
        node_ptr link;
        node_ptr back_link;

        node (const node& other, node_ptr previous) {
            key = other.key;
            val = other.val;
            back_link = previous;
        }

        node (K key, V val, node_ptr next, node_ptr link, node_ptr back_link)
                : key(key)
                , val(val)
                , next(next)
                , link(link)
                , back_link(back_link)
        {}


    } node;
    
    MAP ()
        : my_size(0)
        , tab(new node_ptr[16])
        , given_reference(false)
        , mod(16)
    {}

    MAP (const MAP& other) : mod(other.mod) {
        if (!other.given_reference) {
            my_size = other.my_size;
            tab = other.tab;
        }
        given_reference = false;
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

    V& at(K const& k) {
        size_t kHash = Hash(k) % mod;
        node_ptr n = tab[kHash];
        while (n != nullptr) {
            if (n -> key == k)
                return *(n.get());
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

    class iterator {
    public:
        iterator(): ptr_(0) {}

        //ten konstruktor kopiujący przyjmuje jako argument node_ptr, a nie iterator
        //Czy to na pewno zadziała? Bo begin() i end() zwracają obiekt klasy iterator
        explicit iterator(node_ptr it): ptr_(it) {}

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

        // w begin i end jeżeli oznaczymy konstruktor jako explicit to zwrócenie
        //begin i end chyba nie zadziałają - mają one zwracać obiekt klasy iterator,
        //a nie node_ptr
        iterator& begin() {
            return begin_;
        }

        iterator& end() {
            return end_;
        }

        static void setBegin(node_ptr begin) {
            begin_ = begin;
        }

        static void setEnd(node_ptr end) {
            end_ = end;
        }

    private:
        node_ptr ptr_;

        static node_ptr begin_;

        static node_ptr end_;
    };

private:
    
    using tab_ptr = std::shared_ptr<node_ptr[]>;

    size_t my_size;
    //default value of mod is 16.
    // It increases when my_size is greater than 75% of mod.
    // Decreases when my_size is lower than 10% of mod.
    size_t mod;
    tab_ptr tab;
    bool given_reference;


    //simpleCopyOfMap requires table of size at least equal to other.mod
    void simpleMapCopy(const MAP& other , tab_ptr new_tab) {

    }

    //Requires table of size at least equal to mod. new_tab is filled in with
    //the nodes from actual tab. Linkage is saved. First and last element are
    //correctly set, so begin() and end() methods works properly for the new_tab
    //after function calling.
    void hashedMapCopy(tab_ptr new_tab, size_t mod) {

    }

};

#endif //INSERTION_ORDERED_MAP_H