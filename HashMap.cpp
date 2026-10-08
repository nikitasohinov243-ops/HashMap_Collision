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
    HashMap(size_t capacity = 8): buckets_(capacity, nullptr), size_(0){}
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
    };

    void insert(const std::string& key, int value){
        
    };
    int* find(const std::string& key){};
};