#pragma once
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cstddef>

template <typename K, typename V, typename Hash = std::hash<K>>
class HashMap {
private:
    struct Node
    {
        std::pair<const K, V> kv;   // kv.first = ключ, kv.second = значение
        Node* next;
    };

    std::vector<Node*> buckets_;
    size_t size_;

    size_t index(const K& key) const
    {
        return Hash{}(key) % buckets_.size();
    }

    void clear() noexcept {
        for (size_t i = 0; i < buckets_.size(); i++)
        {
            Node* current = buckets_[i];
            while (current != nullptr)
            {
                Node* next = current->next; // обращаемся к куче, а не массиву buckets_
                delete current;
                current = next;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    void rehash(size_t new_capacity) {
        std::vector<Node*> new_buckets_(new_capacity, nullptr);
        for (size_t i = 0; i < buckets_.size(); ++i)
        {
            Node* current = buckets_[i];
            while (current != nullptr)
            {
                Node* next = current->next;
                size_t idx = Hash{}(current->kv.first) % new_capacity;
                current->next = new_buckets_[idx];
                new_buckets_[idx] = current;
                current = next;
            }
        }
        buckets_.swap(new_buckets_);
    }

public:
    HashMap(size_t capacity = 8) : buckets_(capacity, nullptr), size_(0) {
        if (capacity == 0)
        {
            throw std::invalid_argument("Capacity must be greater than 0");
        }
    }

    HashMap(const HashMap& other) : buckets_(other.buckets_.size(), nullptr), size_(0) { // глубокое копирование
        try {
            for (size_t i = 0; i < buckets_.size(); ++i)
            {
                for (const Node* cur = other.buckets_[i]; cur != nullptr; cur = cur->next)
                {
                    Node* NewNode = new Node{cur->kv, buckets_[i]};
                    buckets_[i] = NewNode;
                    ++size_;
                }
            }
        }
        catch (...) { clear(); throw; }
    }

    HashMap(HashMap&& other) noexcept : buckets_(std::move(other.buckets_)), size_(other.size_) // перемещение
    {
        other.buckets_.assign(1, nullptr);
        other.size_ = 0;
    }

    ~HashMap() { clear(); }

    void insert(const K& key, V value) {
        size_t idx = index(key); // индекс новой пары

        Node* current = buckets_[idx];
        while (current != nullptr) // проходимся по цепочке
        {
            if (current->kv.first == key) // ключи совпали -> обновляем значение
            {
                current->kv.second = value;
                return;
            }
            current = current->next;
        }
        // Если мы здесь, значит ключа не было в цепочке: ставим новую пару в начало
        Node* NewNode = new Node{{key, value}, buckets_[idx]};
        buckets_[idx] = NewNode;
        ++size_;
        if (static_cast<double>(size_) / buckets_.size() > 0.75) rehash(buckets_.size() * 2);
    }

    const V* find(const K& key) const
    {
        const Node* current = buckets_[index(key)];
        while (current != nullptr)
        {
            if (current->kv.first == key)
                return &current->kv.second;
            current = current->next;
        }
        return nullptr;
    }

    V* find(const K& key)
    {
        Node* current = buckets_[index(key)];
        while (current != nullptr)
        {
            if (current->kv.first == key) return &current->kv.second;
            current = current->next;
        }
        return nullptr;
    }

    void swap(HashMap& other) noexcept {
        buckets_.swap(other.buckets_);
        std::swap(size_, other.size_);
    }

    HashMap& operator=(HashMap other) noexcept {
        swap(other);
        return *this;
    }

    bool erase(const K& key) {
        size_t idx = index(key);
        Node* current = buckets_[idx];
        Node* prev = nullptr;

        while (current != nullptr)
        {
            if (current->kv.first == key)
            {
                if (prev == nullptr)
                {
                    buckets_[idx] = current->next; // вырезаем голову
                }
                else
                {
                    prev->next = current->next;    // вырезаем из середины/хвоста
                }
                delete current;
                --size_;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return buckets_.size();
    }

    size_t size_buckets(const K& key) const {
        const Node* current = buckets_[index(key)];
        size_t count = 0;
        while (current != nullptr)
        {
            ++count;
            current = current->next;
        }
        return count;
    }

    V& operator[](const K& key) {
        V* p = find(key);
        if (p) return *p;

        insert(key, V{});
        return *find(key);
    }

    class iterator {
    private:
        HashMap* map_;
        size_t bucket_;
        Node* node_;

    public:
        using value_type = std::pair<const K, V>;
        using reference = value_type &;
        using pointer = value_type *;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        
        iterator(HashMap* map, size_t bucket, Node* node):map_(map), bucket_(bucket), node_(node) {}
        reference operator*() const{
            return node_->kv;
        };
        pointer operator->() const{
            return &node_->kv;
        };
        iterator& operator++(){
            if(node_->next != nullptr){
                node_ = node_->next;
                return *this;
            }
            ++bucket_;
            while (bucket_ < map_->buckets_.size() && map_->buckets_[bucket_] == nullptr){++bucket_;}

            if(bucket_<map_->buckets_.size()) node_ = map_->buckets_[bucket_];
            else node_=nullptr;

            return *this;
        };
        iterator operator++(int){
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const{
            if (node_ == other.node_){return true;}
            return false;
        };
        bool operator!=(const iterator& other)const{
            if (node_ != other.node_){return true;}
            return false;
        };
    };

    iterator begin() {
            for (size_t i = 0; i < buckets_.size(); ++i){
                if (buckets_[i] != nullptr){
                    return iterator(this, i, buckets_[i]);
                }
            }
            return end();
        }
    iterator end() { return iterator (this, buckets_.size(), nullptr);}
};