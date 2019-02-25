#pragma once
#include <functional>
#include <list>
#include <memory>

template<typename KeyType, typename ValueType, typename Hash = std::hash<KeyType> >
class HashMap {
    Hash hasher;
    size_t table_size = 11;
    size_t count = 0;
    std::list<std::pair<const KeyType, ValueType>>* table = nullptr;
    std::list<size_t> used_buckets;
    std::unique_ptr<std::list<size_t>::iterator>* used_buckets_iter = nullptr;
    void resize(size_t new_size) {
        auto new_table = new std::list<std::pair<const KeyType, ValueType>>[new_size];
        auto new_used_buckets_iter = new std::unique_ptr<std::list<size_t>::iterator>[new_size];
        std::list<size_t> new_used_buckets;
        for (const auto & x : *this) {
            size_t hash = hasher(x.first) % new_size;
            new_table[hash].push_back(x);
            if (!new_used_buckets_iter[hash]) {
                new_used_buckets.push_front(hash);
                new_used_buckets_iter[hash].reset(new std::list<size_t>::iterator(new_used_buckets.begin()));
            }
        }
        swap(used_buckets, new_used_buckets);
        swap(used_buckets_iter, new_used_buckets_iter);
        swap(table, new_table);
        delete[] new_table;
        delete[] new_used_buckets_iter;
        table_size = new_size;
    }
public:
    HashMap() : HashMap(Hash()) {
    }
    HashMap(const Hash& hasher) :
        hasher(hasher),
        table(new std::list<std::pair<const KeyType, ValueType>>[table_size]),
        used_buckets_iter(new std::unique_ptr<std::list<size_t>::iterator>[table_size])
    {}
    template<typename Iter>
    HashMap(Iter begin, const Iter & end, const Hash& hasher = Hash()) : HashMap(hasher) {
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }
    HashMap(const std::initializer_list<std::pair<const KeyType, ValueType>> & list, const Hash& hasher = Hash()) : HashMap(hasher) {
        for (const auto& elem : list)
            insert(elem);
    }
    HashMap(const HashMap& another) : HashMap(another.hasher) {
        for (const auto & x : another) {
            insert(x);
        }
    }
    HashMap& operator=(const HashMap& another) {
        if (this != &another) {
            clear();
            for (const auto & x : another) {
                insert(x);
            }
        }
        return *this;
    }
    size_t size() const {
        return count;
    }
    bool empty() const {
        return count == 0;
    }
    const Hash & hash_function() const {
        return hasher;
    }
    void insert(const std::pair<const KeyType, ValueType>& elem) {
        if (count == table_size)
            resize(2 * table_size);
        if (find(elem.first) == end()) {
            ++count;
            size_t hash = hasher(elem.first) % table_size;
            table[hash].push_back(elem);
            if (!used_buckets_iter[hash]) {
                used_buckets.push_front(hash);
                used_buckets_iter[hash].reset(new std::list<size_t>::iterator(used_buckets.begin()));
            }
        }
    }
    void erase(const KeyType & key) {
        size_t hash = hasher(key) % table_size;
        auto j = table[hash].begin();
        while (j != table[hash].end() && j->first != key)
            ++j;
        if (j != table[hash].end()) {
            --count;
            table[hash].erase(j);
            if (!table[hash].size()) {
                used_buckets.erase(*used_buckets_iter[hash]);
                used_buckets_iter[hash] = {};
            }
        }
    }
    class iterator {
        friend class const_iterator;
        std::list<size_t>::iterator list_iter;
        typename std::list<std::pair<const KeyType, ValueType>>::iterator index;
        std::list<std::pair<const KeyType, ValueType>>* table;
        std::list<size_t>* used_buckets;
    public:
        iterator() {}
        iterator(std::list<size_t>::iterator list_iter, typename std::list<std::pair<const KeyType, ValueType>>::iterator index, std::list<std::pair<const KeyType, ValueType>>* table, std::list<size_t>* used_buckets) :
            list_iter(list_iter), index(index), table(table), used_buckets(used_buckets) {}
        iterator& operator++() {
            ++index;
            if (index == table[*list_iter].end()) {
                ++list_iter;
                if (list_iter == used_buckets->end())
                    index = {};
                else
                    index = table[*list_iter].begin();
            }
            return *this;
        }
        iterator operator++(int) {
            auto pr = *this;
            ++index;
            if (index == table[*list_iter].end()) {
                ++list_iter;
                if (list_iter == used_buckets->end())
                    index = {};
                else
                    index = table[*list_iter].begin();
            }
            return pr;
        }
        std::pair<const KeyType, ValueType>& operator*() {
            return *index;
        }
        typename std::list<std::pair<const KeyType, ValueType>>::iterator& operator->() {
            return index;
        }
        template<typename iter>
        bool operator==(const iter& b) const {
            return list_iter == b.list_iter && index == b.index;
        }
        template<typename iter>
        bool operator!=(const iter& b) const {
            return list_iter != b.list_iter || index != b.index;
        }
    };
    class const_iterator {
        friend class iterator;
        std::list<size_t>::const_iterator list_iter;
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator index;
        std::list<std::pair<const KeyType, ValueType>>* table = nullptr;
        const std::list<size_t>* used_buckets;
    public:
        const_iterator() {}
        const_iterator(std::list<size_t>::const_iterator list_iter, typename std::list<std::pair<const KeyType, ValueType>>::const_iterator index, std::list<std::pair<const KeyType, ValueType>>* table, const std::list<size_t>* used_buckets) :
            list_iter(list_iter), index(index), table(table), used_buckets(used_buckets) {}
        const_iterator& operator++() {
            ++index;
            if (index == table[*list_iter].end()) {
                ++list_iter;
                if (list_iter == used_buckets->end())
                    index = {};
                else
                    index = table[*list_iter].begin();
            }
            return *this;
        }
        const_iterator operator++(int) {
            auto pr = *this;
            ++index;
            if (index == table[*list_iter].end()) {
                ++list_iter;
                if (list_iter == used_buckets->end())
                    index = {};
                else
                    index = table[*list_iter].begin();
            }
            return pr;
        }
        const std::pair<const KeyType, ValueType>& operator*() const {
            return *index;
        }
        const typename std::list<std::pair<const KeyType, ValueType>>::const_iterator& operator->() const {
            return index;
        }
        template<typename iter>
        bool operator==(const iter& b) const {
            return list_iter == b.list_iter && index == b.index;
        }
        template<typename iter>
        bool operator!=(const iter& b) const {
            return list_iter != b.list_iter || index != b.index;
        }
    };
    iterator begin() {
        if (used_buckets.size())
            return {used_buckets.begin(), table[used_buckets.front()].begin(), table, &used_buckets};
        else
            return end();
    }
    iterator end() {
        return {used_buckets.end(), {}, table, &used_buckets};
    }
    const_iterator begin() const {
        if (used_buckets.size())
            return {used_buckets.begin(), table[used_buckets.front()].begin(), table, &used_buckets};
        else
            return end();
    }
    const_iterator end() const {
        return {used_buckets.end(), {}, table, &used_buckets};
    }
    iterator find(const KeyType& key) {
        size_t hash = hasher(key) % table_size;
        auto j = table[hash].begin();
        while (j != table[hash].end() && !(j->first == key))
            ++j;
        if (j != table[hash].end()) {
            return {*used_buckets_iter[hash], j, table, &used_buckets};
        } else {
            return end();
        }
    }
    const_iterator find(const KeyType& key) const {
        size_t hash = hasher(key) % table_size;
        auto j = table[hash].begin();
        while (j != table[hash].end() && j->first != key)
            ++j;
        if (j != table[hash].end()) {
            return {*used_buckets_iter[hash], j, table, &used_buckets};
        } else {
            return end();
        }
    }
    ValueType& operator[](const KeyType& key) {
        auto x = find(key);
        if (x == end()) {
            insert({key, ValueType()});
            x = find(key);
        }
        return x->second;
    }
    const ValueType& at(const KeyType& key) const {
        auto x = find(key);
        if (x == end()) {
            throw std::out_of_range("trolling happened");
        }
        return x->second;
    }
    void clear() {
        count = 0;
        for (size_t elem : used_buckets) {
            table[elem].clear();
            used_buckets_iter[elem] = {};
        }
        used_buckets.clear();
    }
    ~HashMap() {
        delete[] table;
        delete[] used_buckets_iter;
    }
};