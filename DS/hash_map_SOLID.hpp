#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <vector>
#include <list>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <initializer_list>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <string_view>

namespace hash_map_impl {

    template <typename Hash, typename Key>
    concept ValidHasher = requires(Hash h, Key k) {
        { h(k) } -> std::convertible_to<std::size_t>;
    };

    template <typename Equal, typename Key>
    concept KeyComparator = requires(Equal eq, Key k1, Key k2) {
        { eq(k1, k2) } -> std::convertible_to<bool>;
    };

    template <typename Key, typename Value,
        typename Hash = std::hash<Key>,
        typename KeyEqual = std::equal_to<>,
        typename Allocator = std::allocator<std::pair<const Key, Value>>>
    class hash_map_core {
    protected:
        using Bucket = std::list<std::pair<const Key, Value>, Allocator>;
        std::vector<Bucket> buckets;
        Hash hash_fn;
        KeyEqual key_eq;
        [[no_unique_address]] Allocator alloc;
        float max_load_factor_ = 0.75f;
        size_t element_count = 0;

        size_t get_bucket(const Key& key) const noexcept {
            return hash_fn(key) % buckets.size();
        }

        void rehash_if_needed() {
            if (load_factor() > max_load_factor_) {
                rehash(buckets.size() * 2);
            }
        }

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

        template <bool IsConst>
        class Iterator {
            using BucketIterator = std::conditional_t<IsConst,
                typename Bucket::const_iterator,
                typename Bucket::iterator>;

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

            Iterator(VectorIterator vec, Bucket& b, BucketIterator bit)
                : vec_it(vec), vec_end(vec + 1), bucket_it(bit) {
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

        explicit hash_map_core(size_type bucket_count = 16,
            const Hash& hash = Hash(),
            const KeyEqual& equal = KeyEqual(),
            const Allocator& alloc = Allocator())
            : buckets(bucket_count), hash_fn(hash), key_eq(equal), alloc(alloc) {
        }

        hash_map_core(std::initializer_list<value_type> init,
            size_type bucket_count = 16,
            const Hash& hash = Hash(),
            const KeyEqual& equal = KeyEqual(),
            const Allocator& alloc = Allocator())
            : hash_map_core(bucket_count, hash, equal, alloc) {
            for (const auto& pair : init) {
                insert(pair.first, pair.second);
            }
        }

        hash_map_core(const hash_map_core& other)
            : buckets(other.buckets.size()),
            hash_fn(other.hash_fn),
            key_eq(other.key_eq),
            alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)),
            max_load_factor_(other.max_load_factor_),
            element_count(0) {
            for (const auto& bucket : other.buckets) {
                for (const auto& pair : bucket) {
                    insert(pair.first, pair.second);
                }
            }
        }

        hash_map_core(hash_map_core&& other) noexcept
            : buckets(std::move(other.buckets)),
            hash_fn(std::move(other.hash_fn)),
            key_eq(std::move(other.key_eq)),
            alloc(std::move(other.alloc)),
            max_load_factor_(other.max_load_factor_),
            element_count(other.element_count) {
            other.element_count = 0;
        }

        hash_map_core& operator=(hash_map_core other) noexcept {
            swap(other);
            return *this;
        }

        void swap(hash_map_core& other) noexcept {
            using std::swap;
            swap(buckets, other.buckets);
            swap(hash_fn, other.hash_fn);
            swap(key_eq, other.key_eq);
            swap(alloc, other.alloc);
            swap(max_load_factor_, other.max_load_factor_);
            swap(element_count, other.element_count);
        }

        template <typename K, typename V>
        std::pair<Iterator<false>, bool> insert(K&& key, V&& value) {
            auto bucket_idx = get_bucket(key);
            auto& bucket = buckets[bucket_idx];

            for (auto it = bucket.begin(); it != bucket.end(); ++it) {
                if (key_eq(it->first, key)) {
                    return { Iterator<false>(buckets.begin() + bucket_idx, bucket, it), false };
                }
            }

            bucket.emplace_back(std::forward<K>(key), std::forward<V>(value));
            ++element_count;
            rehash_if_needed();
            return { Iterator<false>(buckets.begin() + bucket_idx, bucket, std::prev(bucket.end())), true };
        }

        template <typename K>
        Value* find(const K& key) noexcept {
            auto bucket_idx = get_bucket(key);
            auto& bucket = buckets[bucket_idx];

            for (auto& item : bucket) {
                if (key_eq(item.first, key)) {
                    return &item.second;
                }
            }
            return nullptr;
        }

        template <typename K>
        const Value* find(const K& key) const noexcept {
            return const_cast<hash_map_core*>(this)->find(key);
        }

        template <typename K>
        Value& operator[](K&& key) {
            if (auto* val = find(key)) {
                return *val;
            }

            auto bucket_idx = get_bucket(key);
            auto& bucket = buckets[bucket_idx];
            bucket.emplace_back(std::forward<K>(key), Value());
            ++element_count;
            rehash_if_needed();
            return bucket.back().second;
        }

        template <typename K>
        size_type erase(const K& key) noexcept {
            auto bucket_idx = get_bucket(key);
            auto& bucket = buckets[bucket_idx];

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

        void rehash(size_type count) {
            std::vector<Bucket> new_buckets(count);
            for (auto& bucket : buckets) {
                while (!bucket.empty()) {
                    auto it = bucket.begin();
                    auto new_bucket_idx = hash_fn(it->first) % count;
                    new_buckets[new_bucket_idx].splice(
                        new_buckets[new_bucket_idx].end(),
                        bucket,
                        it
                    );
                }
            }
            buckets = std::move(new_buckets);
        }

        size_type size() const noexcept { return element_count; }
        bool empty() const noexcept { return element_count == 0; }
        size_type bucket_count() const noexcept { return buckets.size(); }
        size_type bucket_size(size_type n) const { return buckets[n].size(); }
        float load_factor() const noexcept { return static_cast<float>(element_count) / buckets.size(); }
        float max_load_factor() const noexcept { return max_load_factor_; }
        void max_load_factor(float ml) { max_load_factor_ = ml; rehash_if_needed(); }

        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;

        iterator begin() noexcept { return iterator(buckets.begin(), buckets.end()); }
        const_iterator begin() const noexcept { return const_iterator(buckets.begin(), buckets.end()); }
        const_iterator cbegin() const noexcept { return const_iterator(buckets.begin(), buckets.end()); }
        iterator end() noexcept { return iterator(buckets.end(), buckets.end()); }
        const_iterator end() const noexcept { return const_iterator(buckets.end(), buckets.end()); }
        const_iterator cend() const noexcept { return const_iterator(buckets.end(), buckets.end()); }
    };

} // namespace hash_map_impl

template <typename Key, typename Value,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<>,
    typename Allocator = std::allocator<std::pair<const Key, Value>>>
    requires hash_map_impl::ValidHasher<Hash, Key>&&
hash_map_impl::KeyComparator<KeyEqual, Key>
class hash_map : public hash_map_impl::hash_map_core<Key, Value, Hash, KeyEqual, Allocator> {
    using Base = hash_map_impl::hash_map_core<Key, Value, Hash, KeyEqual, Allocator>;

public:
    using Base::Base;
    using Base::operator=;
    using Base::swap;

    void print(std::ostream& os = std::cout) const {
        os << "Hash Map (size: " << this->size()
            << ", buckets: " << this->bucket_count()
            << ", load factor: " << this->load_factor() << ")\n";

        for (size_t i = 0; i < this->bucket_count(); ++i) {
            os << "  [" << i << "] ";
            if (this->bucket_size(i) == 0) {
                os << "empty";
            }
            else {
                bool first = true;
                for (const auto& item : this->buckets[i]) {
                    if (!first) os << " -> ";
                    os << "{" << item.first << ": " << item.second << "}";
                    first = false;
                }
            }
            os << "\n";
        }
    }

    void print_collisions(std::ostream& os = std::cout) const {
        std::unordered_map<size_t, std::vector<typename Base::value_type>> hash_groups;

        for (const auto& bucket : this->buckets) {
            for (const auto& item : bucket) {
                size_t h = this->hash_fn(item.first);
                hash_groups[h].push_back(item);
            }
        }

        for (const auto& [hash_val, items] : hash_groups) {
            if (items.size() > 1) {
                os << "Hash " << hash_val << " (" << items.size() << " items):\n";
                for (const auto& item : items) {
                    os << "  {" << item.first << ": " << item.second << "}\n";
                }
            }
        }
    }

    void print_by_hash(size_t hash_value, std::ostream& os = std::cout) const {
        size_t bucket_idx = hash_value % this->bucket_count();
        const auto& bucket = this->buckets[bucket_idx];

        os << "Elements with hash " << hash_value << " (bucket " << bucket_idx << "):\n";
        bool found = false;

        for (const auto& item : bucket) {
            if (this->hash_fn(item.first) == hash_value) {
                os << "  {" << item.first << ": " << item.second << "}\n";
                found = true;
            }
        }

        if (!found) {
            os << "  No elements with this hash found\n";
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const hash_map& map) {
        map.print(os);
        return os;
    }
};

#endif // HASH_MAP_H
