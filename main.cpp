#include <iostream>
#include <memory>
#include <unordered_map>
#include <random>
#include <string>
#include <sstream>
#include <type_traits>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>

std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string uuid;
    for (int i = 0; i < 8; ++i) {
        uuid += "0123456789ABCDEF"[dis(gen)];
    }
    return uuid;
}

template <typename T>
struct TreeNode {
    std::string id; 
    T value;      
    std::shared_ptr<TreeNode<T>> left;
    std::shared_ptr<TreeNode<T>> right;

    TreeNode(T val) : id(generateUUID()), value(val), left(nullptr), right(nullptr) {}
};

template <typename T>
class BinaryTree {
private:
    std::shared_ptr<TreeNode<T>> root;
    std::unordered_map<std::string, std::shared_ptr<TreeNode<T>>> nodeMap; 
    std::mutex fileMutex;

    std::shared_ptr<TreeNode<T>> addNode(std::shared_ptr<TreeNode<T>> node, T value) {
        if (!node) {
            auto newNode = std::make_shared<TreeNode<T>>(value);
            nodeMap[newNode->id] = newNode;
            return newNode;
        }

        if (value <= node->value) {
            node->left = addNode(node->left, value);
        } else {
            node->right = addNode(node->right, value);
        }
        return node;
    }

    std::shared_ptr<TreeNode<T>> removeNode(std::shared_ptr<TreeNode<T>> node, const std::string& id) {
        if (!node) return nullptr;

        if (node->id == id) {
            nodeMap.erase(id);

            if (!node->left) return node->right;
            if (!node->right) return node->left;

            auto minNode = getMinNode(node->right);
            node->value = minNode->value;
            node->id = minNode->id;
            nodeMap[node->id] = node;
            node->right = removeNode(node->right, minNode->id);
        } else if (id < node->id) {
            node->left = removeNode(node->left, id);
        } else {
            node->right = removeNode(node->right, id);
        }
        return node;
    }

    std::shared_ptr<TreeNode<T>> getMinNode(std::shared_ptr<TreeNode<T>> node) {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }

    void findByValueHelper(std::shared_ptr<TreeNode<T>> node, const T& value, std::vector<std::pair<std::string, T>>& results) {
        if (!node) return;

        findByValueHelper(node->left, value, results);
        if (node->value == value) {
            results.emplace_back(node->id, node->value);
        }
        findByValueHelper(node->right, value, results);
    }

public:
    void add(T value) {
        root = addNode(root, value);
    }

    void remove(const std::string& id) {
        root = removeNode(root, id);
    }

    std::shared_ptr<TreeNode<T>> find(const std::string& id) {
        if (nodeMap.find(id) != nodeMap.end()) {
            return nodeMap[id];
        }
        return nullptr;
    }

    void update(const std::string& id, T newValue) {
        auto node = find(id);
        if (node) {
            node->value = newValue;
        } else {
            std::cout << "Node with ID " << id << " not found!\n";
        }
    }

    std::vector<std::pair<std::string, T>> findByValue(const T& value) {
        std::vector<std::pair<std::string, T>> results;
        findByValueHelper(root, value, results);
        return results;
    }

    void printTree(std::shared_ptr<TreeNode<T>> node) {
        if (!node) return;
        printTree(node->left);
        std::cout << "ID: " << node->id << ", Value: " << node->value << "\n";
        printTree(node->right);
    }

    void print() {
        printTree(root);
    }

    std::shared_ptr<TreeNode<T>> getRoot() {
        return root;
    }
};

class TreeManager {
private:
    std::unordered_map<std::string, std::shared_ptr<BinaryTree<std::string>>> trees;

public:
    void createTree(const std::string& name) {
        if (trees.find(name) == trees.end()) {
            trees[name] = std::make_shared<BinaryTree<std::string>>();
            std::cout << "Tree created with name: " << name << "\n";
        } else {
            std::cout << "Tree with name " << name << " already exists!\n";
        }
    }

    std::shared_ptr<BinaryTree<std::string>> getTree(const std::string& name) {
        if (trees.find(name) != trees.end()) {
            return trees[name];
        } else {
            std::cout << "No tree found with name " << name << "\n";
            return nullptr;
        }
    }

    void removeTree(const std::string& name) {
        if (trees.find(name) != trees.end()) {
            trees.erase(name);
            std::cout << "Tree " << name << " removed\n";
        } else {
            std::cout << "No tree found with name " << name << "\n";
        }
    }
};

int main() {
    TreeManager manager;

    manager.createTree("FruitTree");
    manager.createTree("VeggieTree");

    auto fruitTree = manager.getTree("FruitTree");
    if (fruitTree) {
        fruitTree->add("apple");
        fruitTree->add("banana");
        fruitTree->add("cherry");
        fruitTree->print();
    }

    auto veggieTree = manager.getTree("VeggieTree");
    if (veggieTree) {
        veggieTree->add("carrot");
        veggieTree->add("broccoli");
        veggieTree->print();
    }

    manager.removeTree("FruitTree");
    manager.removeTree("UnknownTree");

    return 0;
}