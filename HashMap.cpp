#include <iostream>
#include <string>
#include <vector>
#include <functional>


class HashMap{
    private:
    struct Node
    {
        std::string key;
        int value;
        Node* next;
    };

    std::vector<Node*> buckets_;
    size_t size_;

    size_t index(const std::string& key) const
    {
        return std::hash<std::string>{}(key) % buckets_.size();
    }
    public:
    HashMap(size_t capacity = 8): buckets_(capacity, nullptr), size_(0){
        if (capacity == 0)
        {
            throw std::invalid_argument("Capacity must be greater than 0");
        }
    }
    ~HashMap()
    {
        for (size_t i = 0;i < buckets_.size();i++ )
        {
            Node* current = buckets_[i];
            while(current != nullptr)
            {
                Node* next = current->next;//обращаемся к куче, а не массиву buckets_
                delete current;
                current = next;
            }
        }
    }

    void insert(const std::string& key, int value){
        size_t idx = index(key);//индекс новой пары

        Node* current = buckets_[idx];//новая пара в узле
        while(current != nullptr) //проходимся по узлу
        {
            if (current->key == key)//ключи совпали -> обновляем значение
            {
                current->value = value;
                return;
            }
            current = current->next; // иначе переходим дальше
        }
        // Если мы здесь значит ключа не было в цепочке
        Node* NewNode = new Node{key, value, buckets_[idx]};//если не встретился, то ставим в начало узла новую пару 
        buckets_[idx] = NewNode;//иницилизация новой первой пары узла 
        // теперь корзина указывает на новый первый узел
        ++size_;
        if(static_cast<double>(size_)/buckets_.size() > 0.75) rehash(buckets_.size()*2);
    }
    const int *find(const std::string &key) const
    {
        const Node *current = buckets_[index(key)];
        while (current != nullptr)
        {
            if (current->key == key)
                return &current->value;
            current = current->next;
        }
        return nullptr;
    }
    int *find(const std::string &key)
    {
        Node *current = buckets_[index(key)];
        while (current != nullptr)
        {
            if (current->key == key) return &current->value;
            current = current->next;
        }
        return nullptr;
    }
    bool erase(const std::string& key){
        size_t idx = index(key);
        Node* current = buckets_[idx];
        Node* prev = nullptr;
        
        while (current != nullptr)
        {
            if(current->key == key)
            {
                if(prev == nullptr)
                {
                    buckets_[idx] = current->next;//вырезаем голову
                }
                else
                {
                    prev->next = current->next;//вырезаем промежуток
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
    size_t size() const{
        return size_;
    }
    size_t capacity() const
    {
        return buckets_.size();
    }
    size_t size_buckets(const std::string& key) const{
        Node* current = buckets_[index(key)];
        size_t count = 0;
        while (current != nullptr)
        {
            ++count;
            current = current->next;
        }
        return count;
    }
    void rehash(size_t new_capacity){
        std::vector<Node*> new_buckets_(new_capacity, nullptr);
        for(size_t i = 0; i<buckets_.size(); ++i)
        {
            Node* current = buckets_[i];
            while(current != nullptr)
            {
                Node* next = current -> next;
                size_t idx = std::hash<std::string>{}(current -> key) % new_capacity;
                current -> next = new_buckets_[idx];
                new_buckets_[idx] = current;
                current = next;
            }
        }
        buckets_.swap(new_buckets_);
    }

    int& operator[](const std::string& key){
        int *p = find(key);
        if (p) return *p;

        insert(key, 0);
        return *find(key);
    }
};

int main() {
    HashMap m;
    m["a"] = 1;
    m["a"] += 10;
    std::cout << m["a"] << "\n";      // 11
    std::cout << m.size() << "\n";    // 1
    std::cout << m["zzz"] << "\n";    // 0
    std::cout << m.size() << "\n";    // 2, чтение вставило ключ
    return 0;
}