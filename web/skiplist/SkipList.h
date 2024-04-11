//
// Created by 37496 on 2024/3/5.
//

#ifndef WEBSERVER_SKIPLIST_H
#define WEBSERVER_SKIPLIST_H

#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <iostream>

template<typename K, typename V>
class Node
{
public:
    Node() = default;
    Node(K k, V v, int);
    ~Node();

    K getKey() const;
    V getValue() const;
    void setValue(V);

public:
    Node<K,V> **forward;    //前向指针数组，对于不同的层，可能指向不同的节点
    int node_level;         //层数
private:
    K key;
    V val;
};

template<typename K, typename V>
Node<K, V>::Node(K k, V v, int level) :key(k), val(v), node_level(level)
{
    this->forward = new Node<K, V>*[level+1];                   //   forward 大小为 level + 1
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));    // ~ forward 初始化为0（NULL）
}

template<typename K, typename V>
Node<K, V>::~Node()
{
    delete []forward;
}

template<typename K, typename V>
K Node<K, V>::getKey() const {
    return key;
}

template<typename K, typename V>
V Node<K, V>::getValue() const {
    return val;
}

template<typename K, typename V>
void Node<K, V>::setValue(V v) {
    this->val = v;
}

template<typename K, typename V>
class SkipList
{
public:
    explicit SkipList(int);
    ~SkipList();

    int getRandomLevel();

    Node<K, V>* createNode(K, V, int);

    int insertElement(K, V);
    bool searchElement(K);
    void deleteElement(K);

    void dumpFile();
    void loadFile();

    void displayList();

    void clear(Node<K, V>*);

    int size();

private:
    void getKeyValFromString(const std::string& str, std::string* key, std::string* value);
    bool isValidString(const std::string& str);

private:
    int maxLevel;               //跳表层数上限
    int skipListLevel;          //当前跳表的最高层
    Node<K, V>* header;

    std::ofstream fileWriter;
    std::ifstream fileReader;

    int elementCount;
    std::mutex mtx;
};

template<typename K, typename V>
SkipList<K, V>::SkipList(int _maxLevel) :maxLevel(_maxLevel), skipListLevel(0), elementCount(0)
{
    K k;
    V v;
    header = new Node<K, V>(k, v, maxLevel);
}

template<typename K, typename V>
SkipList<K, V>::~SkipList() {
    if(fileWriter.is_open()) {
        fileWriter.close();
    }
    if(fileReader.is_open()) {
        fileReader.close();
    }
    delete header;
}

template<typename K, typename V>
int SkipList<K, V>::getRandomLevel() {
    int k=1;
    while(rand()%2) {
        k++;
    }
    k = (k < maxLevel) ? k : maxLevel;
    return k;
}

template<typename K, typename V>
Node<K, V> *SkipList<K, V>::createNode(const K k, const V v, int level) {
    auto* node = new Node<K, V>(k, v, level);
    return node;
}

template<typename K, typename V>
int SkipList<K, V>::insertElement(const K key, const V val) {
    mtx.lock();
    Node<K, V>* current = this->header;

    Node<K, V>* update[this->maxLevel+1];
    memset(update, 0, sizeof(Node<K, V>*)*(maxLevel+1));

    for(int i=skipListLevel; i>=0; i--) {
        while(current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        update[i]=current;
    }

    current = current->forward[0];

    if(current != nullptr && current->getKey() == key) {
#ifndef USE_COUT
        std::cout << "Key: " << key << ", exists" << std::endl;
#endif
        mtx.unlock();
        return 1;
    }

    if(current == nullptr || current->getKey() != key) {
        int randomLevel = getRandomLevel();
        if(randomLevel > skipListLevel) {
            for(int i=skipListLevel+1; i<randomLevel+1; i++) {
                update[i] = header;
            }
            skipListLevel = randomLevel;
        }

        Node<K, V>* insertNode = createNode(key, val, randomLevel);

        for(int i=0; i<=randomLevel; i++) {
            insertNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = insertNode;
        }

#ifndef USE_COUT
        std::cout << "Successfully insert key: " << key << ", value: " << val << std::endl;
#endif
        elementCount++;
    }
    mtx.unlock();
    return 0;
}



#endif //WEBSERVER_SKIPLIST_H
