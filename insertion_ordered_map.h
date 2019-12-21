#ifndef INSERTION_ORDERED_MAP_H
#define INSERTION_ORDERED_MAP_H

#include <cstdlib>
#include <functional>
#include <memory>

#define MAP insertion_ordered_map

class lookup_error : std::exception {
    char const* what() const noexcept {
        return "lookup_error";
    }
};

template <class K, class V, class Hash = std::hash<K>>
class MAP {

private:

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
                , val(other -> val) // tu powinien byc jakiś kopiujący
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

        explicit node (size_t hash)
                : hash(hash)
                , link(nullptr)
                , back_link(nullptr)
                , next(nullptr)
        {}

    } node;

    using tab_ptr = std::shared_ptr<node_ptr[]>;

public:

    class iterator {

    public:

        iterator(): ptr_(0) {}

        iterator(const iterator& it): ptr_(it.ptr_) {}

        explicit iterator(node_ptr& node) : ptr_(node) {};


        const iterator& operator++() {
            ptr_ = ptr_ -> link;
            return *this;
        }

        std::pair<K, V> *operator->() {
            values = std::make_pair(ptr_ -> key, ptr_ -> val);
            return &values;
        }

        bool operator==(const iterator& it) const noexcept {
            return ptr_ == it.ptr_;
        }

        bool operator!=(const iterator& it) const noexcept {
            return !(*this == it);
        }

        const node_ptr& getPtr() const {
            return ptr_;
        }

    private:

        std::pair<K, V> values;

        node_ptr ptr_;

    };

private:

    size_t my_size;
    //default value of mod is 16.
    // It increases twice when my_size is greater than 75% of mod.
    // Decreases twice when my_size is lower than 10% of mod.
    size_t mod;
    tab_ptr tab;
    bool given_reference;
    iterator end_;
    iterator begin_;
    node_ptr dummy;

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
        guard g(this, mod);
        mod *= p;
        mod /= q;

        tab_ptr tab_n(new node_ptr[mod]);
        hashedMapMove(tab_n);
        tab = tab_n;
        g.success();
    }

    void checkSize() {
        if (tab.use_count() > 1) {
            guard g(this, mod);
            if (my_size >= mod*3/4)
                mod *= 2;
            else if (mod > 16 && my_size <= mod/10)
                mod /= 2;
            tab_ptr t(new node_ptr[mod]);
            hashedMapCopy(begin(), end(), t, mod, false);
            g.success();
            tab = t;
            given_reference = false;
        } else if (my_size >= mod*3/4) {
            createResizedMap(2, 1);
            given_reference = false;
        } else if (mod > 16 && my_size <= mod/10) {
            createResizedMap(1, 2);
            given_reference = false;
        }
    }

    void hashedMapMove(tab_ptr& new_tab) noexcept {

        node_ptr n = begin().getPtr();
        while (n != end().getPtr()) {
            n -> next = nullptr;
            n = n -> link;
        }

        n = begin().getPtr();
        while (n != end().getPtr()) {
            n -> hash = Hash{}(n->key) % mod;
            addNode(n, n->hash, new_tab);
            n = n -> link;
        }
    }

    void hashedMapCopy(const iterator& beg, const iterator& end,
                       tab_ptr& new_tab, size_t mod, bool rehash) {

        node_ptr previous(new node(0));
        node_ptr end_node = previous;

        guard g(end_node);
        for (iterator it = beg; it != end; ++it) {
            node_ptr new_node(new node(it.getPtr(), previous));
            if (rehash) {
                new_node -> hash = Hash{}(new_node -> key)%mod;
            }

            previous -> link = new_node;
            new_node -> back_link = previous;
            addNode(new_node, new_node -> hash, new_tab);
            previous = new_node;
        }
        g.success();

        deleteMap();

        end_ = iterator(end_node);
        end_.getPtr() -> back_link = previous;
        begin_ = iterator(end_.getPtr() -> link);
        dummy = end_node;
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

    void deleteMap() {
        if (tab == nullptr)
            return;
        if (tab.use_count() == 1) {
            node_ptr n = dummy -> link, next = n -> link;
            while (n != dummy) {
                deletePointer(n);
                n = next;
                if (n)
                    next = n -> link;
            }
            deletePointer(dummy);
        }
    }

    void deletePointer(node_ptr n) {
        n -> link = nullptr;
        n -> back_link = nullptr;
        n -> next = nullptr;
    }

    typedef struct guard {
        size_t old_mod;
        node_ptr to_delete;
        MAP* m;
        bool suc;

        explicit guard(MAP* m, size_t mod)
                : old_mod(mod)
                , to_delete(nullptr)
                , m(m)
                , suc(false)
        {}

        explicit guard(node_ptr to_delete)
                : old_mod(0)
                , to_delete(to_delete)
                , m(nullptr)
                , suc(false)
        {}

        void success() {
            suc = true;
        }

        ~guard() {
            if (!suc) {
                node_ptr next = nullptr;
                if (to_delete != nullptr)
                    next = to_delete->next;
                while (to_delete != nullptr) {
                    to_delete->link = nullptr;
                    to_delete->back_link = nullptr;
                    to_delete->next = nullptr;
                    to_delete = next;
                    if (to_delete != nullptr)
                        next = to_delete->link;
                }
                if (old_mod != 0)
                    m->mod = old_mod;
            }
        }
    } guard;

public:

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
        begin_ = iterator(end_.getPtr() -> link);
        dummy = n;
    }

    MAP (const MAP& other)
            : mod(other.mod)
            , my_size(other.my_size)
            , given_reference(false)
    {
        if (!other.given_reference) {
            tab = other.tab;
            end_ = other.end_;
            begin_ = other.begin_;
            dummy = other.dummy;
        } else {
            tab_ptr t(new node_ptr[other.mod]);
            tab = nullptr;
            hashedMapCopy(other.begin(), other.end(), t, other.mod, true);
            tab = t;
        }
    }

    MAP (MAP&& other) noexcept
            : my_size(other.my_size)
            , tab(other.tab)
            , given_reference(other.given_reference)
            , mod(other.mod)
            , dummy(other.dummy)
    {
        other.tab = nullptr;
        other.my_size = 0;
        end_ = other.end_;
        begin_ = other.begin_;

    }

    ~MAP () {
        deleteMap();
    }

    size_t size() const noexcept {
        return my_size;
    }

    bool empty() const noexcept {
        return my_size == 0;
    }

    void erase(K const &k) {
        node_ptr n = findNode(k);

        if (n == nullptr) {
            throw lookup_error();
        } else {
            checkSize();
            n = findNode(k);
            n->back_link->link = n->link;
            n->link->back_link = n->back_link;

            size_t kHash = Hash{}(k) % mod;
            node_ptr n2 = tab[kHash];

            if (n2 == n) {
                tab[kHash] = n->next;
            } else {
                while (n2->next && n2->next != n) {
                    n2 = n2->next;
                }
                n2->next = n2->next->next;
            }
            deletePointer(n);
            --my_size;
            begin_ = iterator(end_.getPtr()->link);
            given_reference = false;
        }
    }

    size_t countNodesNotInMap(insertion_ordered_map const &other) {
        size_t sum = 0;

        //uzywam twojego at by policzyc o ile mam powiekszyc tablice
        for (auto it = other.begin(), end = other.end(); it != end; ++it) {
            if (findNode(it->first) == nullptr)
                sum++;
        }

        return sum;
    }

    void merge(insertion_ordered_map const &other) {
        size_t s = other.my_size;
        if (s > 0) {
            if (countNodesNotInMap(other) + my_size >= mod*3/4) {
                // wydłużasz tutaj tablicę dokładnie do potrzebnego rozmiaru.
                //Proponuję nie zliczać elementów, których nie ma, tylko wziąć
                //np 3 razy max rozmiaru słowników. Popatrz, że słownik rozszerzamy juz przy 75% zapełnienia!
                //(po to, żeby zapewnić możliwie dużą unikalność hashy)

                //tu nie zwieszkam o rowna ilosc tylko o okolo 2 razy. I tez zeby nie przesadzac ze zwiekszaniem bo np mozna potem dostac cos w stylu ze beda ci mergowac caly czas tablice z takimi samymi kluczami to ci sie tablica mocno powiekszy po nic
                createResizedMap(((countNodesNotInMap(other) + my_size) / mod) * 2 , 1);
            }

            //dodaje te wartosci z kluczami z others ktore nie sa w this

            //wykorzystaj funkcję findNode(K& k). Ona zwraca node_ptr z węzłem o kluczu k.
            //Jak nie znajdzie to nullptr. Wstawiać możesz za pomocą inserta po prostu.
            //nowy sposób na link i backlink opisałem na messengerze, także zobacz też na to, żeby działało poprawnie
            for (auto it = other.begin(), end = other.end(); it != end; ++it) {
                node_ptr n = findNode(it->first);

                if (n == nullptr) {
                    insert(it->first, it->second);
                }
            }
        }
    }

    void insert(const K& k, const V& v) {

        node_ptr n2(new node(k, v, nullptr, nullptr, nullptr, 0));
        checkSize();
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
            n2 -> hash = Hash{}(k)%mod;
            n2 -> link = end().getPtr();
            n2 -> back_link = end().getPtr() -> back_link;
            n2 -> back_link -> link = n2;
            n2 -> link -> back_link = n2;
            addNode(n2, n2 -> hash, tab);
        }
        begin_ = iterator(end().getPtr() -> link);
        assert(contains(k));
        given_reference = false;
    }

    V const& at(K const& k) const noexcept {
        node_ptr n = findNode(k);
        if (n == nullptr) {
            throw lookup_error();
        } else {
            return n -> val;
        }
    }

    V& at(K const& k) {
        node_ptr n = findNode(k);
        if (n == nullptr) {
            throw lookup_error();
        } else {
            checkSize();
            given_reference = true;
            return n -> val;
        }
    }

    V& operator[](const K& k) {
        node_ptr n = findNode(k);
        if (n == nullptr) {
            insert(k, V());
        } else {
            checkSize();
        }
        n = findNode(k);
        given_reference = true;
        return n->val;
    }

    void clear() {
        tab_ptr t(new node_ptr[16]);
        node_ptr n(new node(0));
        deleteMap();

        tab = t;
        my_size = 0;
        mod = 16;
        given_reference = false;
        n -> link = n;
        n -> back_link = n;
        dummy = n;
        end_= iterator(n);
        begin_ = iterator(end_.getPtr() -> link);
    }

    bool contains(const K& k) const noexcept {
        return findNode(k) != nullptr;
    }

    const iterator& begin() const {
        return begin_;
    }

    const iterator& end() const {
        return end_;
    }
};

#endif //INSERTION_ORDERED_MAP_H