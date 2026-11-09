#include"BinaryTree.h"

int main() {
EventLog* log = new EventLog;
	List* pre = new List(5, 30);
	pre->next = new List(2, 35);
	pre->next->next = new List(1, 40);
	pre->next->next->next = new List(8, 90);
	pre->next->next->next->next = new List(11, 60);

	List* in = new List(2, 2);
	in->next = new List(1, 1);
	in->next->next = new List(5, 5);
	in->next->next->next = new List(4, 1);
	in->next->next->next->next = new List(3, 2);

	List* po = new List(2, 2);
	po->next = new List(1, 1);
	po->next->next = new List(3, 2);
	po->next->next->next = new List(4, 1);
	po->next->next->next->next = new List(5, 5);
	List* temp = pre;
	BinaryTree* tree = new BinaryTree;
	tree->setTree(tree->postBuild(po, in));
	tree->traverse(tree->getTree());
	pre = temp;
	cout << '\n';
	cout << tree->height(tree->getTree());
	cout << '\n' << tree->diameter() << '\n';
	TreeNode* t1 = tree->lca(8, 11);
	cout << tree->isSymmetric() << endl;
	tree->postOrder_I(log);
	for (int i = 0; i < log->size(); i++) {
		const Events& e = log->get(i);
		if(e.type == EventTypes::VISIT)
		cout << "VISIT NODE ID: " << e.id << endl;
	}
}