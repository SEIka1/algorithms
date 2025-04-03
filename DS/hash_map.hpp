//implementation of hash map with C++20

#include <concepts>
#include <functional>
#include <list>
#include <vector>
#include <type_traits>
#include <format>
#include <coroutine>
#include <source_location>
#include <stdexcept>
#include <string_view>

template<typename Hash, typename Key>
concept ValidHasher = requires(Hash h_, Key k_) {
  { h_(k_) } -> std::convertible_to<std::size_t>;
};

template <typename Equal, typename Key>
concept TransparentEqual = requires(Equal eq_, Key k_, std::string_view s_) {
    { eq_(k_, k_) } -> std::convertible_to<bool>;
    { eq(k_, s_) } -> std::convertible_to<bool>;
};

template <
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<>,
    typename Allocator = std::allocator<std::pair<const Key, Value>>
>   requires ValidHasher<Hash, Key> && TransparentEqual<KeyEqual, Key>
class hash_map {
private:
    using Bucket = std::list<std::pair<const Key, Value>, Allocator>;
    std::vector<Bucket> buckets;
    Hash hash_fn;
    KeyEqual key_eq;
    [[no_unique_address]] Allocator alloc;
    float max_load_factor = 0.75f;

    void rehash_if_needed() {
        if (load_factor() > max_load_factor) {
            rehash(buckets.size() * 2);
        }
    }

public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using size_type = std::size_t;

    hash_map(size_type bucket_count = 16, 
             const Hash& hash = Hash(),
             const KeyEqual& equal = KeyEqual(),
             const Allocator& alloc = Allocator())
        : buckets(bucket_count), hash_fn(hash), key_eq(equal), alloc(alloc) {}
