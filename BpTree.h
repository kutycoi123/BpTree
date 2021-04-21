#ifndef BPTREE_H
#define BPTREE_H

#include "Node.h"
#include <iostream>
class BpTree{
	public:
		BpTree();
		BpTree(int);
		BpTree(const BpTree&);
		bool insert(int, const std::string&) noexcept;
		bool remove(int) noexcept;
		std::string find(int) const noexcept;
		void printKeys() const noexcept;
		void printValues() const noexcept;
		std::unique_ptr<Node> deepCopy(Node*, Node*) noexcept;	
		BpTree& operator=(const BpTree&) noexcept;	
		LeafNode* findNodeOfKey(int) const noexcept; //Find the leaf node that contains the given key
		~BpTree(){}
	private:
		void insertLeafNode(LeafNode*, int, const std::string&); //Insert key and value into leaf node
		void insertInteriorNode(InteriorNode*, int, std::unique_ptr<Node> ); //Insert key and pointer of next node into a interior node
		void getAllLeafNodes(Node*, std::vector<LeafNode*>&) const noexcept;
		void connectAllLeafs() noexcept;
		int keysLimit;
		std::unique_ptr<Node> root;

};


#endif
