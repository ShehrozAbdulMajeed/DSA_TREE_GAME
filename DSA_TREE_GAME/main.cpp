#include"AVL.h"
#include"MaxHeap.h"


int main() {
AVL* tree = new AVL;
cout << tree->remove(2);
tree->insert(20,10);
tree->insert(10,20);
tree->insert(30,30);
cout << tree->insert(10, 33);
tree->insert(5,40);
tree->insert(15,50);
tree->insert(25, 60);
tree->insert(40, 70);
cout << tree->kth(1)->key << " ";
cout << tree->kth(4)->key << " ";
cout << tree->kth(7)->key << " ";
cout << tree->rank(17) << " ";
cout << tree->countInRange(30,15) << " ";

}