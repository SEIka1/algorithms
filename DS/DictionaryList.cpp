#include <iostream>
#include <memory>
#include <utility>
#include <cassert>

#if _HAS_CXX17
#include <xpolymorphic_allocator.h>
#endif // _HAS_CXX17

#pragma pack(push, _CRT_PACKING)
#pragma warning(push, 3)
#pragma warning(disable : 4180 4324 4412 4455 4494 4514 4574 \
4582 4583 4587 4588 4619 4623 4625 4626 4643 4648 4702 4793 \
4820 4868 4988 5026 5027 5045 5220 5246 6294  4984 5053)

template <typename _Ty, typename Allocator = std::allocator<_Ty>>
class DictionaryList {
private:
    struct Node {
        Node* prev;
        Node* next;
        _Ty data;

        Node(const _Ty& item) noexcept
            : data{ std::move(item) }, prev(nullptr), next(nullptr) {
        }
    };

    Node* head;
    Node* tail;
    size_t size;

    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    NodeAllocator allocator;

    void swap(DictionaryList& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(size, other.size);
        std::swap(allocator, other.allocator);
    }

public:

    

    using      value_type = _Ty;
    using       size_type = size_t;
    using difference_type = ptrdiff_t;
    using         pointer = value_type*;
    using   const_pointer = const value_type*;
    using       reference = value_type&;
    using const_reference = const value_type&;

    DictionaryList() = default;

    explicit DictionaryList(const Allocator& alloc = Allocator()) noexcept
        : head(nullptr), tail(nullptr), size(0), allocator(alloc) {
    }

    DictionaryList(std::initializer_list<value_type> items)
    {
        for (auto& item : items)
        {
            push_back(item);
        } 
    }

    ~DictionaryList() noexcept {
        clear();
    }

    DictionaryList(DictionaryList&& other) noexcept
        : head(other.head), tail(other.tail), size(other.size), allocator(std::move(other.allocator)) {
        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }

    DictionaryList& operator=(DictionaryList&& other) noexcept {
        if (this != &other) {
            clear();
            swap(other);
        }
        return *this;
    }

    class Unchecked_const_iterator {
    private:
        Node* current;

        friend class DictionaryList;
    public:

        using   difference_type = DictionaryList::difference_type;
        using        value_type = DictionaryList::value_type;
        using           pointer = DictionaryList::const_pointer;
        using         reference = DictionaryList::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit Unchecked_const_iterator(Node* ptr) noexcept : current{ ptr } {}

        _NODISCARD reference operator*() const noexcept {
            assert(current && "Dereferencing a null iterator.");
            return current->data;
        }

        _NODISCARD pointer operator->() const noexcept {
            return std::pointer_traits<pointer>::pointer_to(**this);
        }

        Unchecked_const_iterator& operator++() noexcept {
            assert(current && "Incrementing a null iterator.");
            current = current->next;
            return *this;
        }

        Unchecked_const_iterator operator++(int) noexcept {
            assert(current && "Incrementing(int) a null iterator.");
            Unchecked_const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        Unchecked_const_iterator& operator--() noexcept {
            assert(current && "Decrementing a null iterator.");
            current = current->prev;
            return *this;
        }

        Unchecked_const_iterator operator--(int) noexcept {
            assert(current && "Decrementing(int) a null iterator.");
            Unchecked_const_iterator temp = *this;
            --(*this);
            return temp;
        }

        _NODISCARD bool operator==(const Unchecked_const_iterator& other) const noexcept {
            return current == other.current;
        }

        _NODISCARD bool operator!=(const Unchecked_const_iterator& other) const noexcept {
            return !(*this == other);
        }

    };

    class Unchecked_iterator
        : public Unchecked_const_iterator
    {
    private:
        friend class DictionaryList;

        explicit Unchecked_iterator(Node* ptr) noexcept
            : Unchecked_const_iterator{ ptr } {}

    public:

        using   difference_type = DictionaryList::difference_type;
        using        value_type = DictionaryList::value_type;
        using           pointer = DictionaryList::pointer;
        using         reference = DictionaryList::reference;
        using iterator_category = std::bidirectional_iterator_tag;

        reference operator*() noexcept
        {
            return const_cast<reference>(
                Unchecked_const_iterator::operator*());
        }

        pointer operator->() noexcept {
            return const_cast<pointer>(Unchecked_const_iterator::operator->());
        }

        Unchecked_iterator& operator++() noexcept
        {
            Unchecked_const_iterator::operator++();
            return *this;
        }

        Unchecked_iterator& operator--() noexcept
        {
            Unchecked_const_iterator::operator--();
            return *this;
        }


        Unchecked_iterator operator++(int) noexcept {
            Unchecked_iterator temp = *this;
            Unchecked_const_iterator::operator++();
            return temp;
        }

        Unchecked_iterator operator--(int) noexcept {
            Unchecked_iterator temp = *this;
            Unchecked_const_iterator::operator--();
            return temp;
        }
    };

    Unchecked_const_iterator begin() const noexcept {
        return Unchecked_const_iterator(head);
    }

    Unchecked_const_iterator end() const noexcept {
        return Unchecked_const_iterator(nullptr);
    }

    Unchecked_const_iterator cbegin() const noexcept {
        return Unchecked_const_iterator(head);
    }

    Unchecked_const_iterator cend() const noexcept {
        return Unchecked_const_iterator(nullptr);
    }

    Unchecked_iterator begin() noexcept {
        return Unchecked_iterator(head);
    }

    Unchecked_iterator end() noexcept {
        return Unchecked_iterator(nullptr);
    }

    using iterator = Unchecked_iterator;
    using const_iterator = Unchecked_const_iterator;

    void push_front(value_type item)
    {
        auto newnode = new Node{ std::move(item) };
        if (head)
        {
            head->prev = newnode;
            newnode->next = head;
            head = newnode;
        }
        else
        {
            head = tail = newnode;
        }
    }

    void push_back(value_type item)
    {
        auto newnode = new Node{ std::move(item) };
        if (tail)
        {
            tail->next = newnode;
            newnode->prev = tail;
            tail = newnode;
        }
        else
        {
            head = tail = newnode;
        }
    }

    void insert(const_iterator place, value_type item) {
        Node* newNode = std::allocator_traits<NodeAllocator>::allocate(allocator, 1);
        std::allocator_traits<NodeAllocator>::construct(allocator, newNode, item);

        if (!head) {
            head = tail = newNode;
            ++size;
            return;
        }

        auto ptr = const_cast<Node*>(place.current);
        if (!ptr)
        {
            push_back(std::move(item));
            return;
        }

        newNode->next = ptr;
        newNode->prev = ptr->prev;

        if (ptr->prev)
        {
            ptr->prev->next = newNode;
        } else {
            head = newNode;
        }

        ptr->prev = newNode;
            
        ++size;
    }

    void insert(size_type index, const value_type& value) {
        if (index > size) {
            return;
        }
        auto it = begin();
        std::advance(it, index);
        insert(it, value); 
    }

    const_iterator find_first(const_reference item) const noexcept
    {
        for (auto it = begin(); it != end(); ++it) {
            if (*it == item) {
                return it;
            }
        }
        return const_iterator{ nullptr };
    }

    iterator find_first(const_reference item) noexcept
    {
        auto it = static_cast<const DictionaryList&>(*this)
            .find_first(item);

        return iterator{ const_cast<Node*>(it.current) };
    }

    iterator find(const_reference item) noexcept {
        for (auto it = begin(); it != end(); ++it) {
            if (**it == *item) {
                return it;
            }
        }
        return end();
    }

    const_iterator find(const_reference item) const noexcept {
        for (auto it = begin(); it != end(); ++it) {
            if (**it == *item) {
                return it;
            }
        }
        return end();
    }

    void deleteItem(const_iterator place) noexcept {
        auto ptr = const_cast<Node*>(place.current);
        assert(ptr != nullptr);

        if (ptr->prev) {
            ptr->prev->next = ptr->next;
        }
        else {
            head = ptr->next;
        }

        if (ptr->next) {
            ptr->next->prev = ptr->prev;
        }
        else {
            tail = ptr->prev;
        }

        std::allocator_traits<NodeAllocator>::destroy(allocator, ptr);
        std::allocator_traits<NodeAllocator>::deallocate(allocator, ptr, 1);
        --size;
    }

    void deleteItem(iterator place) noexcept {
        deleteItem(static_cast<const_iterator>(place));
    }

    void clear() {
        while (head) {
            Node* temp = head;
            head = head->next;
            std::allocator_traits<NodeAllocator>::destroy(allocator, temp);
            std::allocator_traits<NodeAllocator>::deallocate(allocator, temp, 1);
        }
        tail = nullptr;
        size = 0;
    }

    void merge(DictionaryList&& other) {
        if (this == &other) {
            return;
        }

        Node* current = other.head;
        while (current) {
            Node* next = current->next;

            if (!head) {
                head = current;
                tail = current;
            }
            else {
                tail->next = current;
                current->prev = tail;
                tail = current;
            }

            current = next;
            ++size;
        }

        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }

    void print() const {
        Node* current = head;
        size_t i = 0;
        std::cout << "{ ";
        while (current) {
            if (i == 0) {
                std::cout << "" << i << " : ";
            }
            else {
                std::cout << "  " << i << " : ";
            }

            if constexpr (std::is_pointer_v<_Ty>) {
                std::cout << *current->data;
            }
            else {
                std::cout << current->data;
            }

            if (current->next == nullptr) {
                std::cout << " }\n";
            }
            else {
                std::cout << "\n";
            }

            current = current->next;
            ++i;
        }
        std::cout << std::endl;
    }

};

int main() {

    using CustomAllocator = std::allocator<int>;
    CustomAllocator customAlloc;

    DictionaryList<int, CustomAllocator> dict1(customAlloc);
    dict1.push_back(5);
    dict1.push_back(6);
    dict1.push_back(7);

    DictionaryList<int, CustomAllocator> dict2(customAlloc);
    dict2.push_back(8);
    dict2.push_back(9);
    dict2.push_back(10);

    std::cout << "Dict1:\n";
    dict1.print();
    std::cout << "Dict2:\n";
    dict2.print();

    dict1.merge(std::move(dict2));

    std::cout << "Merged Dict1:\n";
    dict1.print();

    std::cout << "Dict1 after removing first occurence of 10:\n";

    dict1.deleteItem(dict1.find_first(10));
    dict1.print();

    std::cout << "Dict1 after inserting 1st element (5):\n";
    dict1.insert(1, 5);
    dict1.print();

    std::cout << "Dict3 after pushback (int 5):\n";
    DictionaryList<int*, CustomAllocator> dict3(customAlloc);
    int a = 5;
    dict3.push_back(&a);
    dict3.print();

    std::cout << "Trying to find (int a = 5):\n";
    auto A = dict3.find(&a);
    
    if(A != dict3.end())
        std::cout << "element found:\n{ " << **A << " : " << *A << " }\n";

    return 0;
}


#pragma warning(pop)
#pragma pack(pop)
