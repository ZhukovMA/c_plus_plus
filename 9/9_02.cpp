#include <iostream>
#include <memory>
#include <queue>
#include <vector>

class Tree {
public:
    struct Node {
        int value{};
        std::shared_ptr<Node> left{};
        std::shared_ptr<Node> right{};
        std::weak_ptr<Node> parent{};

        explicit Node(int v) : value(v) {
            std::cout << "in constructor Node " << value << std::endl;
        }

        ~Node() {
            std::cout << "in destructor Node " << value << '\n';
        }
    };

    std::shared_ptr<Node> root{};

    void traverse_v1() const {
        std::cout << "traverse in BFS: ";


        if (!root) {
            std::cout << "empty root\n";
            return;
        }

        std::queue<std::shared_ptr<Node>> q;
        q.push(root);

        while (!q.empty()) {
            auto cur = q.front();
            q.pop();

            std::cout << cur->value << ' ';

            if (cur->left) {
                q.push(cur->left);
            }



            if (cur->right) {
                q.push(cur->right);
            }
        }

        std::cout << '\n';
    }

    void traverse_v2() const {
        std::cout << "traverse in DFS: ";
        if (!root) {
            std::cout << "empty root\n";
            return;
        }

        std::vector<std::shared_ptr<Node>> st;
        st.push_back(root);

        while (!st.empty()) {
            auto cur = st.back();
            st.pop_back();

            std::cout << cur->value << ' ';

            if (cur->right) {
                st.push_back(cur->right);
            }
            if (cur->left) {
                st.push_back(cur->left);
            }
        }

        std::cout << '\n';
    }
};

int main() {
    std::cout << "start building tree\n";

    Tree tree;

    auto attach_left = [](const std::shared_ptr<Tree::Node>& parent, int value) {
        parent->left = std::make_shared<Tree::Node>(value);
        parent->left->parent = parent;
        return parent->left;
    };

    auto attach_right = [](const std::shared_ptr<Tree::Node>& parent, int value) {
        parent->right = std::make_shared<Tree::Node>(value);
        parent->right->parent = parent;
        return parent->right;
    };

    tree.root = std::make_shared<Tree::Node>(1);

    auto mid_left  = attach_left(tree.root, 2);
    auto mid_right = attach_right(tree.root, 3);

    attach_left(mid_left, 4);
    attach_right(mid_left, 5);
    attach_left(mid_right, 6);
    attach_right(mid_right, 7);

    std::cout << "Traverse in tree:\n";
    tree.traverse_v1();
    tree.traverse_v2();

    std::cout << "reset external references\n";
    mid_left.reset();
    mid_right.reset();

    std::cout << "tree destroyed\n";
}