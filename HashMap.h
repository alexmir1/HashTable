#pragma once
#include <functional>
#include <list>
#include <memory>
#include <vector>

template<typename KeyType, typename ValueType, typename Hash = std::hash<KeyType> >
class HashMap {
    Hash hasher;
    size_t table_size = 11;
    size_t count = 0;
    using Element = std::pair<const KeyType, ValueType>;
    std::vector<std::list<Element>> table;
    std::list<size_t> used_buckets;
    std::vector<std::unique_ptr<std::list<size_t>::iterator>> used_buckets_iter;

    void resize(size_t new_size) {
        std::vector<std::list<Element>> new_table(new_size);
        std::vector<std::unique_ptr<std::list<size_t>::iterator>> new_used_buckets_iter(new_size);
        std::list<size_t> new_used_buckets;
        for (const auto & x : *this) {
            size_t hash = hasher(x.first) % new_size;
            new_table[hash].push_back(x);
            if (!new_used_buckets_iter[hash]) {
                new_used_buckets.push_front(hash);
                new_used_buckets_iter[hash] = std::make_unique<std::list<size_t>::iterator>(new_used_buckets.begin());
            }
        }
        swap(used_buckets, new_used_buckets);
        swap(used_buckets_iter, new_used_buckets_iter);
        swap(table, new_table);
        table_size = new_size;
    }

    template<typename T, typename U>
    T static abstract_find(const KeyType& key, U & map) {
        size_t hash = map.hasher(key) % map.table_size;
        auto bucketIndex = map.table[hash].begin();
        while (bucketIndex != map.table[hash].end() && !(bucketIndex->first == key))
            ++bucketIndex;
        if (bucketIndex != map.table[hash].end()) {
            return {*map.used_buckets_iter[hash], bucketIndex, &map.table, &map.used_buckets};
        } else {
            return {map.used_buckets.end(), {}, &map.table, &map.used_buckets};
        }
    }

public:

    HashMap() : HashMap(Hash()) {
    }

    HashMap(const Hash& hasher) :
        hasher(hasher),
        table(table_size),
        used_buckets_iter(table_size)
    {}

    template<typename Iter>
    HashMap(Iter begin, const Iter & end, const Hash& hasher = Hash()) : HashMap(hasher) {
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }
    
    HashMap(std::initializer_list<Element> list, const Hash& hasher = Hash()) : HashMap(hasher) {
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
    
    void insert(const Element& elem) {
        if (count == table_size)
            resize(2 * table_size);
        if (find(elem.first) == end()) {
            ++count;
            size_t hash = hasher(elem.first) % table_size;
            table[hash].push_back(elem);
            if (!used_buckets_iter[hash]) {
                used_buckets.push_front(hash);
                used_buckets_iter[hash] = std::make_unique<std::list<size_t>::iterator>(used_buckets.begin());
            }
        }
    }
    
    void erase(const KeyType & key) {
        size_t hash = hasher(key) % table_size;
        auto bucketIndex = table[hash].begin();
        while (bucketIndex != table[hash].end() && bucketIndex->first != key)
            ++bucketIndex;
        if (bucketIndex != table[hash].end()) {
            --count;
            table[hash].erase(bucketIndex);
            if (table[hash].empty()) {
                used_buckets.erase(*used_buckets_iter[hash]);
                used_buckets_iter[hash] = {};
            }
        }
    }
    
    class iterator {
        friend class const_iterator;
        std::list<size_t>::const_iterator list_iter;
        typename std::list<Element>::iterator index;
        std::vector<std::list<Element>>* table;
        std::list<size_t>* used_buckets;
    public:
        
        iterator() {}
    
        iterator(std::list<size_t>::const_iterator list_iter,
            typename std::list<Element>::iterator index,
            std::vector<std::list<Element>>* table,
            std::list<size_t>* used_buckets) :
            list_iter(list_iter), index(index), table(table), used_buckets(used_buckets) {}
        
        iterator& operator++() {
            ++index;
            if (index == (*table)[*list_iter].end()) {
                ++list_iter;
                if (list_iter == used_buckets->end())
                    index = {};
                else
                    index = (*table)[*list_iter].begin();
            }
            return *this;
        }
        
        iterator operator++(int) {
            auto previous = *this;
            ++*this;
            return previous;
        }
        
        Element& operator*() {
            return *index;
        }
        
        typename std::list<Element>::iterator& operator->() {
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
        typename std::list<Element>::const_iterator index;
        const std::vector<std::list<Element>>* table;
        const std::list<size_t>* used_buckets;
    public:

        const_iterator() {}

        const_iterator(std::list<size_t>::const_iterator list_iter,
            typename std::list<Element>::const_iterator index,
            const std::vector<std::list<Element>>* table,
            const std::list<size_t>* used_buckets) :
            list_iter(list_iter), index(index), table(table), used_buckets(used_buckets) {}

        const_iterator& operator++() {
            ++index;
            if (index == (*table)[*list_iter].end()) {
                ++list_iter;
                if (list_iter == used_buckets->end())
                    index = {};
                else
                    index = (*table)[*list_iter].begin();
            }
            return *this;
        }

        const_iterator operator++(int) {
            auto previous = *this;
            ++*this;
            return previous;
        }

        const Element& operator*() const {
            return *index;
        }

        const typename std::list<Element>::const_iterator& operator->() const {
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
            return {used_buckets.begin(), table[used_buckets.front()].begin(), &table, &used_buckets};
        else
            return end();
    }
    
    iterator end() {
        return {used_buckets.end(), {}, &table, &used_buckets};
    }
    
    const_iterator begin() const {
        if (used_buckets.size())
            return {used_buckets.begin(), table[used_buckets.front()].begin(), &table, &used_buckets};
        else
            return end();
    }
    
    const_iterator end() const {
        return {used_buckets.end(), {}, &table, &used_buckets};
    }
    
    iterator find(const KeyType& key) {
        return abstract_find<iterator, HashMap>(key, *this);
    }
    
    const_iterator find(const KeyType& key) const {
        return abstract_find<const_iterator, const HashMap>(key, *this);
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
};
