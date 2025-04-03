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
concept ValidHasher = requires(Hash h, Key k) {
  { h(k) } -> std::convertible_to<std::size_t>;
};

template <typename Equal, typename Key>
concept TransparentEqual = requires(Equal eq, Key k, std::string_view s) {
    { eq(k, k) } -> std::convertible_to<bool>;
    { eq(k, s) } -> std::convertible_to<bool>;
};

template <
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<>,
    typename Allocator = std::allocator<std::pair<const Key, Value>>
> requires ValidHasher<Hash, Key> && TransparentEqual<KeyEqual, Key>
class hash_map;
