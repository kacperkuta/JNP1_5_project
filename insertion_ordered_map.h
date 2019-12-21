/*
 * insertion_ordered_map
 * by Kacper Kuta and Franciszek Bieleń
 * created on 18 dec 2019
 */

#ifndef INSERTION_ORDERED_MAP_H
#define INSERTION_ORDERED_MAP_H

#include <cstdlib>
#include <functional>
#include <memory>

#define MAP insertion_ordered_map

class lookup_error : std::exception {
    const char* what() const noexcept {
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
        std::pair<K, V> mapping;
        node_ptr next;
        node_ptr link;
        node_ptr back_link;

        node(const node_ptr& other, node_ptr previous)
                : hash(other->hash)
                , mapping(std::pair<K, V>(other->mapping))
                , next(nullptr)
                , link(nullptr)
                , back_link(previous)
        {}

        node(K key, V val, node_ptr next, node_ptr link,
              node_ptr back_link, size_t hash)
                : hash(hash)
                , mapping(std::pair<K, V>(key, val))
                , next(next)
                , link(link)
                , back_link(back_link)
        {}

        explicit node(size_t hash)
                : hash(hash)
                , next(nullptr)
                , link(nullptr)
                , back_link(nullptr)
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
            ptr_ = ptr_->link;
            return *this;
        }

        std::pair<K, V>* operator->() {
            return &(ptr_->mapping);
        }

        const std::pair<const K, const V> operator* () {
            return ptr_->mapping;
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

        node_ptr ptr_;

    };

private:
    /*
     * Wyjsciowa wartosc mod: 16, zwiekszana jest ona gdy my_size przekroczy
     * 75 procent wartosci mod, zmniejszana gdy my_size przekroczy 10 procent
     * wartosci mod. Mod to liczba używanych hashów.
     */
    size_t my_size;
    size_t mod;
    tab_ptr tab;
    bool given_reference;
    iterator end_;
    iterator begin_;
    node_ptr dummy;

    size_t basic_mod = 16;
    unsigned mult = 2;
    unsigned div = 3;

    void addNode(node_ptr& node, size_t hash, tab_ptr& new_tab) noexcept {
        node_ptr n = new_tab[hash];
        if (n == nullptr) {
            new_tab[hash] = node;
        } else {
            while (n->next != nullptr) {
                n = n->next;
            }
            n->next = node;
        }
        node->next = nullptr;
    }

    /*
     * Powieksza/pomniejsza tab w zaleznosci od p/q.
     * Przenosi elementy do nowej tablicy.
     * Silnie odporna na wyjątki.
     */
    void createResizedMap(unsigned p, unsigned q) {
        guard g(this, mod);
        mod *= p;
        mod /= q;

        tab_ptr tab_n(new node_ptr[mod]);
        hashedMapMove(tab_n);
        tab = tab_n;
        g.success();
    }

    /*
     * Sprawdza czy nalezy powiekszyc/pomniejszyc tab.
     * W razie potrzeby robi to.
     * Silnie odporna na wyjątki.
     */
    void checkSize() {
        if (tab.use_count() > 1) {
            guard g(this, mod);
            if (my_size >= mod*3/4)
                mod *= mult * mod > mult * my_size ?
                        mult : mult * my_size / mod + 1;
            else if (mod > basic_mod && my_size <= mod/10)
                mod /= 3;
            tab_ptr t(new node_ptr[mod]);
            hashedMapCopy(begin(), end(), t, mod, false);
            g.success();
            tab = t;
            given_reference = false;
        } else if (my_size >= mod*3/4) {
            createResizedMap(mult, 1);
            given_reference = false;
        } else if (mod > basic_mod && my_size <= mod/10) {
            createResizedMap(1, div);
            given_reference = false;
        }
    }

    /*
     * Zwraca liczbę elementów, które są w other ale nie sa w this
     */
    size_t countNodesNotInMap(insertion_ordered_map const &other) {
        size_t sum = 0;
        for (auto it = other.begin(), end = other.end(); it != end; ++it) {
            if (findNode((*it).first) == nullptr)
                sum++;
        }
        return sum;
    }

    void hashedMapMove(tab_ptr& new_tab) noexcept {

        node_ptr n = begin().getPtr();
        while (n != end().getPtr()) {
            n->next = nullptr;
            n = n->link;
        }

        n = begin().getPtr();
        while (n != end().getPtr()) {
            n->hash = Hash{}(n->mapping.first) % mod;
            addNode(n, n->hash, new_tab);
            n = n->link;
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
                new_node->hash = Hash{}(new_node->mapping.first) % mod;
            }
            previous->link = new_node;
            new_node->back_link = previous;
            addNode(new_node, new_node->hash, new_tab);
            previous = new_node;
        }
        g.success();

        deleteMap();

        end_ = iterator(end_node);
        end_.getPtr()->back_link = previous;
        begin_ = iterator(end_.getPtr()->link);
        dummy = end_node;
        previous->link = end_.getPtr();
    }

    node_ptr findNode(const K& k) const noexcept {
        size_t kHashed = Hash{}(k)%mod;
        node_ptr n = tab[kHashed];
        while (n != nullptr) {
            if (n->mapping.first == k) {
                return n;
            } else {
                n = n->next;
            }
        }
        return nullptr;
    }

    void deleteMap() {
        if (tab == nullptr)
            return;
        if (tab.use_count() == 1) {
            node_ptr n = dummy->link, next = n->link;
            while (n != dummy) {
                deletePointer(n);
                n = next;
                if (n)
                    next = n->link;
            }
            deletePointer(dummy);
        }
    }

    void deletePointer(node_ptr n) {
        n->link = nullptr;
        n->back_link = nullptr;
        n->next = nullptr;
    }

    friend void swap(MAP& first, MAP& second) {
        using std::swap;
        swap(first.my_size, second.my_size);
        swap(first.tab, second.tab);
        swap(first.begin_, second.begin_);
        swap(first.end_, second.end_);
        swap(first.dummy, second.dummy);
        swap(first.mod, second.mod);
        swap(first.given_reference, second.given_reference);
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
                    next = to_delete->link;
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

    MAP()
            : my_size(0)
            , mod(16)
            , tab(new node_ptr[16])
            , given_reference(false)
    {
        node_ptr n(new node(0));
        n->link = n;
        n->back_link = n;
        end_= iterator(n);
        begin_ = iterator(end_.getPtr()->link);
        dummy = n;
    }

    MAP(const MAP& other)
            : my_size(other.my_size)
            , mod(other.mod)
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

    MAP(MAP&& other) noexcept
            : my_size(other.my_size)
            , mod(other.mod)
            , tab(other.tab)
            , given_reference(other.given_reference)
            , end_(other.end_)
            , begin_(other.begin_)
            , dummy(other.dummy)
    {
        other.tab = nullptr;
        other.my_size = 0;
    }

    ~MAP() {
        deleteMap();
    }

    insertion_ordered_map& operator=(insertion_ordered_map other) {
        swap(*this, other);
        return *this;
    }

    /*
     * Zwraca liczbę par klucz-wartość w słowniku
     */
    size_t size() const noexcept {
        return my_size;
    }

    /*
     * Zwraca true, gdy słownik jest pusty, a false w przeciwnym przypadku.
     */
    bool empty() const noexcept {
        return my_size == 0;
    }

    /*
     * Usuwa wartość znajdującą się pod podanym kluczem k. Jeśli taki klucz
     * nie istnieje, to podnosi wyjątek lookup_error.
     */
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

    /*
     * Scalanie słowników. Dodaje kopie wszystkich elementów podanego słownika other
     * do bieżącego słownika (this). Wartości pod kluczami już obecnymi w bieżącym
     * słowniku nie są nadpisywane. Klucze ze słownika other pojawiają się w porządku
     * iteracji na końcu, zachowując kolejność względem siebie.
     * Silnie odporna na wyjątki.
     */
    void merge(insertion_ordered_map const &other) {
        size_t s = other.my_size;
        if (s > 0) {
            MAP copy = *this;
            copy.my_size += countNodesNotInMap(other);
            copy.checkSize();
            for (auto it = other.begin(), end = other.end(); it != end; ++it) {
                node_ptr n = findNode((*it).first);
                copy.insert((*it).first, (*it).second);
                if (n == nullptr)
                    copy.my_size--;
            }
            swap(*this, copy);
        }
    }

    /*
    * Jeśli klucz k nie jest przechowywany w słowniku, to wstawia wartość v
    * pod kluczem k i zwraca true. Jeśli klucz k już jest w słowniku, to
    * wartość pod nim przypisana nie zmienia się, ale klucz zostaje
    * przesunięty na koniec porządku iteracji, a metoda zwraca false.
     * Silnie odporna na wyjątki.
    */
    void insert(const K& k, const V& v) {
        node_ptr n2(new node(k, v, nullptr, nullptr, nullptr, 0));
        checkSize();
        node_ptr n = findNode(k);

        if (n != nullptr) {
            n->link->back_link = n->back_link;
            n->back_link->link = n->link;
            n->back_link = end().getPtr()->back_link;
            n->link = end().getPtr();
            end().getPtr()->back_link->link = n;
            end().getPtr()->back_link = n;
        } else {
            my_size++;
            n2->hash = Hash{}(k)%mod;
            n2->link = end().getPtr();
            n2->back_link = end().getPtr()->back_link;
            n2->back_link->link = n2;
            n2->link->back_link = n2;
            addNode(n2, n2->hash, tab);
        }
        begin_ = iterator(end().getPtr()->link);
        given_reference = false;
    }

    /*
     *  Zwraca const referencję na wartość przechowywaną W słowniku pod podanym
     *  kluczem k. Jeśli taki klucz nie istnieje w słowniku, to podnosi
     *  wyjątek lookup_error.
     */
    V const& at(K const& k) const {
        node_ptr n = findNode(k);
        if (n == nullptr) {
            throw lookup_error();
        } else {
            return n->val;
        }
    }

    /*
     *  Zwraca referencję na wartość przechowywaną W słowniku pod podanym
     *  kluczem k. Jeśli taki klucz nie istnieje w słowniku, to podnosi
     *  wyjątek lookup_error.
     */
    V& at(K const& k) {
        node_ptr n = findNode(k);
        if (n == nullptr) {
            throw lookup_error();
        } else {
            checkSize();
            given_reference = true;
            return n->mapping.second;
        }
    }

    /*
     * Zwraca referencję na wartość znajdującą się w słowniku pod podanym
     * kluczem k. Wywołanie tego operatora z kluczem nieobecnym w słowniku
     * powoduje dodanie pod tym kluczem domyślnej wartości typu V.
     * Silnie odporna na wyjątki.
     */
    V& operator[](const K& k) {
        node_ptr n = findNode(k);
        if (n == nullptr) {
            insert(k, V());
        } else {
            checkSize();
        }
        n = findNode(k);
        given_reference = true;
        return n->mapping.second;
    }

    /*
     *  Usuwa wszystkie elementy ze słownika.
     *  Silnie odporna na wyjątki.
     */
    void clear() {
        tab_ptr t(new node_ptr[16]);
        node_ptr n(new node(0));
        deleteMap();

        tab = t;
        my_size = 0;
        mod = 16;
        given_reference = false;
        n->link = n;
        n->back_link = n;
        dummy = n;
        end_= iterator(n);
        begin_ = iterator(end_.getPtr()->link);
    }

    /*
     * Sprawdzenie klucza. Zwraca wartość boolowską mówiącą, czy podany
     * klucz k jest w słowniku
     */
    bool contains(const K& k) const noexcept {
        return findNode(k) != nullptr;
    }

    /*
     * Funkcja przekazuje iterator wskazujący na pierwszy element
     * w porządku iteracji.
     */
    const iterator& begin() const noexcept {
        return begin_;
    }

    /*
     * Funkcja przekazuje iterator wskazujący na atrapę dummy na końcu
     * porządku iteracji.
     */
    const iterator& end() const noexcept {
        return end_;
    }
};

#endif //INSERTION_ORDERED_MAP_H