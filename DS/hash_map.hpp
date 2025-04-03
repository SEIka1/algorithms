// implementation of hash_map with C++20

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template <typename Hash, typename Key>
concept ValidHasher = requires(Hash h_, Key k_) {
    { h_(k_) } -> std::convertible_to<std::size_t>;
};

template <typename Equal, typename Key>
concept TransparentEqual = requires(Equal eq_, Key k_, std::string_view s_) {
    { eq_(k_, k_) } -> std::convertible_to<bool>;
    { eq_(k_, s_) } -> std::convertible_to<bool>;
};

template <typename Key, typename Value,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<>,
          typename Allocator = std::allocator<std::pair<const Key, Value>>>
requires ValidHasher<Hash, Key> && TransparentEqual<KeyEqual, Key>
class hash_map {
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

private:
    using Bucket = std::list<value_type, Allocator>;
    std::vector<Bucket> buckets;
    Hash hash_fn;
    KeyEqual key_eq;
    [[no_unique_address]] Allocator alloc;
    float max_load_factor = 0.75f;
    size_type element_count = 0;

    size_type get_bucket(const key_type& key) const noexcept {
        return hash_fn(key) % buckets.size();
    }

    void rehash_if_needed() {
        if (load_factor() > max_load_factor) {
            rehash(buckets.size() * 2);
        }
    }

public:
    template <bool IsConst>
    class Iterator {
        using BucketIterator = std::conditional_t<IsConst,
            typename Bucket::const_iterator,
            typename Bucket::const_iterator>;

        using VectorIterator = std::conditional_t<IsConst,
            typename std::vector<Bucket>::const_iterator,
            typename std::vector<Bucket>::iterator>;

        VectorIterator vec_it;
        VectorIterator vec_end;
        BucketIterator bucket_it;

        void skip_empty_buckets() {
            while (vec_it != vec_end && bucket_it == vec_it->end()) {
                ++vec_it;
                if (vec_it != vec_end) {
                    bucket_it = vec_it->begin();
                }
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<const Key, Value>;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;

        Iterator(VectorIterator begin, VectorIterator end)
            : vec_it(begin), vec_end(end) {
            if (vec_it != vec_end) {
                bucket_it = vec_it->begin();
                skip_empty_buckets();
            }
        }

        Iterator& operator++() {
            ++bucket_it;
            skip_empty_buckets();
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        reference operator*() const { return *bucket_it; }
        pointer operator->() const { return &(*bucket_it); }

        bool operator==(const Iterator& other) const {
            return vec_it == other.vec_it && 
                  (vec_it == vec_end || bucket_it == other.bucket_it);
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    explicit hash_map(size_type bucket_count = 16,
                     const Hash& hash = Hash(),
                     const KeyEqual& equal = KeyEqual(),
                     const Allocator& alloc = Allocator())
        : buckets(bucket_count), hash_fn(hash), key_eq(equal), alloc(alloc) {}

    hash_map(std::initializer_list<value_type> init,
             size_type bucket_count = 16,
             const Hash& hash = Hash(),
             const KeyEqual& equal = KeyEqual(),
             const Allocator& alloc = Allocator())
        : hash_map(bucket_count, hash, equal, alloc) {
        for (const auto& item : init) {
            insert(item.first, item.second);
        }
    }

    hash_map(const hash_map& other)
        : buckets(other.buckets.size()),
          hash_fn(other.hash_fn),
          key_eq(other.key_eq),
          alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)),
          max_load_factor(other.max_load_factor),
          element_count(0) {
        for (const auto& bucket : other.buckets) {
            for (const auto& item : bucket) {
                insert(item.first, item.second);
            }
        }
    }

    hash_map& operator=(hash_map other) noexcept {
        swap(other);
        return *this;
    }

    hash_map(hash_map&& other) noexcept
        : buckets(std::move(other.buckets)),
          hash_fn(std::move(other.hash_fn)),
          key_eq(std::move(other.key_eq)),
          alloc(std::move(other.alloc)),
          max_load_factor(other.max_load_factor),
          element_count(other.element_count) {
        other.element_count = 0;
    }

    ~hash_map() = default;

    void swap(hash_map& other) noexcept {
        using std::swap;
        swap(buckets, other.buckets);
        swap(hash_fn, other.hash_fn);
        swap(key_eq, other.key_eq);
        swap(alloc, other.alloc);
        swap(max_load_factor, other.max_load_factor);
        swap(element_count, other.element_count);
    }

    [[nodiscard]] iterator begin() noexcept {
        return iterator(buckets.begin(), buckets.end());
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator(buckets.begin(), buckets.end());
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return const_iterator(buckets.begin(), buckets.end());
    }

    [[nodiscard]] iterator end() noexcept {
        return iterator(buckets.end(), buckets.end());
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator(buckets.end(), buckets.end());
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return const_iterator(buckets.end(), buckets.end());
    }

    template <typename K, typename V>
    [[nodiscard]] std::pair<iterator, bool> insert(K&& key, V&& value) {
        auto& bucket = buckets[get_bucket(key)];
        
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (key_eq(it->first, key)) {
                return {iterator(buckets.begin() + get_bucket(key), bucket, it}, false};
            }
        }

        if constexpr (std::is_trivially_copyable_v<Value>) {
            bucket.emplace_back(std::forward<K>(key), std::forward<V>(value));
        } else {
            bucket.emplace_back(std::piecewise_construct,
                              std::forward_as_tuple(std::forward<K>(key)),
                              std::forward_as_tuple(std::forward<V>(value)));
        }

        ++element_count;
        rehash_if_needed();
        return {iterator(buckets.begin() + get_bucket(key), bucket, std::prev(bucket.end())), true};
    }

    template <typename K>
    [[nodiscard]] Value* find(K&& key) noexcept {
        auto& bucket = buckets[get_bucket(key)];
        for (auto& item : bucket) {
            if (key_eq(item.first, key)) {
                return &item.second;
            }
        }
        return nullptr;
    }

    template <typename K>
    [[nodiscard]] const Value* find(K&& key) const noexcept {
        return const_cast<hash_map*>(this)->find(std::forward<K>(key));
    }

    template <typename K>
    Value& operator[](K&& key) {
        if (auto* val = find(key)) {
            return *val;
        }
        
        auto& bucket = buckets[get_bucket(key)];
        bucket.emplace_back(std::forward<K>(key), Value());
        ++element_count;
        rehash_if_needed();
        return bucket.back().second;
    }

    template <typename K>
    [[nodiscard]] size_type erase(K&& key) noexcept {
        auto& bucket = buckets[get_bucket(key)];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (key_eq(it->first, key)) {
                bucket.erase(it);
                --element_count;
                return 1;
            }
        }
        return 0;
    }

    void clear() noexcept {
        for (auto& bucket : buckets) {
            bucket.clear();
        }
        element_count = 0;
    }

    [[nodiscard]] size_type size() const noexcept { return element_count; }
    [[nodiscard]] bool empty() const noexcept { return element_count == 0; }
    [[nodiscard]] size_type bucket_count() const noexcept { return buckets.size(); }

    [[nodiscard]] float load_factor() const noexcept {
        return static_cast<float>(element_count) / buckets.size();
    }

    [[nodiscard]] float max_load_factor() const noexcept {
        return max_load_factor;
    }

    void max_load_factor(float ml) {
        max_load_factor = ml;
        rehash_if_needed();
    }

    void rehash(size_type count) {
        std::vector<Bucket> new_buckets(count);
        for (auto& bucket : buckets) {
            for (auto& item : bucket) {
                auto new_bucket_idx = hash_fn(item.first) % count;
                new_buckets[new_bucket_idx].splice(
                    new_buckets[new_bucket_idx].end(), 
                    bucket, 
                    std::find(bucket.begin(), bucket.end(), item)
                );
            }
        }
        buckets = std::move(new_buckets);
    }
};
