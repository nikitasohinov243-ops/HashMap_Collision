#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <utility>

template <typename K, typename V, typename Hash = std::hash<K>>
class HashMap{
    private:
    struct Node
    {
        K key;
        V value;
        Node* next;
    };

    std::vector<Node*> buckets_;
    size_t size_;

    size_t index(const K& key) const
    {
        return Hash{}(key) % buckets_.size();
    }
    void clear() noexcept{
        for (size_t i = 0;i < buckets_.size();i++ )
        {
            Node* current = buckets_[i];
            while(current != nullptr)
            {
                Node* next = current->next;//обращаемся к куче, а не массиву buckets_
                delete current;
                current = next;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }
    void rehash(size_t new_capacity){
        std::vector<Node*> new_buckets_(new_capacity, nullptr);
        for(size_t i = 0; i<buckets_.size(); ++i)
        {
            Node* current = buckets_[i];
            while(current != nullptr)
            {
                Node* next = current -> next;
                size_t idx = Hash{}(current -> key) % new_capacity;
                current -> next = new_buckets_[idx];
                new_buckets_[idx] = current;
                current = next;
            }
        }
        buckets_.swap(new_buckets_);
    }
    public:
    HashMap(size_t capacity = 8): buckets_(capacity, nullptr), size_(0){
        if (capacity == 0)
        {
            throw std::invalid_argument("Capacity must be greater than 0");
        }
    }
    HashMap(const HashMap& other):buckets_(other.buckets_.size(), nullptr), size_(0){ //глубокое копирование
        try{
            for(size_t i = 0; i < buckets_.size(); ++i)
            {
                for(const Node* cur = other.buckets_[i]; cur != nullptr; cur = cur->next)
                {
                    Node* NewNode = new Node{cur -> key,cur -> value, buckets_[i]};
                    buckets_[i] = NewNode;
                    ++size_;
                }
            }
        }
        catch(...){clear(); throw;}
    }
    HashMap(HashMap&& other) noexcept: buckets_(std::move(other.buckets_)), size_(other.size_)
    {
        other.buckets_.assign(1, nullptr);
        other.size_ = 0;
    }
    ~HashMap(){ clear(); }
    void insert(const K& key, V value){
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
    const V *find(const K& key) const
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
    V *find(const K& key)
    {
        Node *current = buckets_[index(key)];
        while (current != nullptr)
        {
            if (current->key == key) return &current->value;
            current = current->next;
        }
        return nullptr;
    }
    void swap(HashMap& other) noexcept { //noexcept нужен для того что бы выйграть на копировании (обещаем, что не бросит исключение)
        buckets_.swap(other.buckets_);
        std::swap(size_, other.size_);
    }
    HashMap& operator=(HashMap other) noexcept{
        swap(other);
        return *this;
    }
    bool erase(const K& key){
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
    size_t size_buckets(const K& key) const{
        Node* current = buckets_[index(key)];
        size_t count = 0;
        while (current != nullptr)
        {
            ++count;
            current = current->next;
        }
        return count;
    }

    V& operator[](const K& key){
        V *p = find(key);
        if (p) return *p;

        insert(key, V{});
        return *find(key);
    }

};

int main() {
    HashMap<std::string, int> a;
    for (int i = 0; i < 100; ++i)
        a["k" + std::to_string(i)] = i;

    HashMap<std::string, int> b = a; // копия
    b["k0"] = 999;
    std::cout << a["k0"] << " " << b["k0"] << "\n"; // 0 999 (таблицы независимы)

    HashMap<std::string, int> c = std::move(a);       // перемещение
    std::cout << c.size() << " " << a.size() << "\n"; // 100 0
    a["new"] = 1;                                     // перемещённый объект не сломан
    std::cout << a.size() << "\n";                    // 1

    b = c;                         // копирующее присваивание
    c = std::move(b);              // перемещающее
    c = c;                         // самоприсваивание, ничего не ломается
    std::cout << c.size() << "\n"; // 100
}