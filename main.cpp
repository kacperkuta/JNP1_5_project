#include <cstdlib>
#include <functional>
#include <memory>

#define MAP insertion_ordered_map

template <class K, class V, class Hash = std::hash<K>>
class MAP {
public:
    MAP () : my_size(0), tab(new node*[16]) {};

    MAP (const MAP& other)  {
        my_size = other.size();
        tab = other.get_tab();
    }

    size_t size() noexcept {
        return my_size;
    }

protected:
    typedef struct node {
        K key;
        V val;
        node * next;
        node * link;
        node * back_link;
    } node;

    using tab_ptr = std::shared_ptr<node*[]>;

    tab_ptr get_tab() {
        return tab;
    }

private:

    size_t my_size;
    tab_ptr tab;

};

int main() {

}