#ifndef NODE_H
#define NODE_H

#include<vector>
#include<algorithm>
#include<cmath>
#include<memory>
#include<string>
#include<iterator>
#include<iostream>
class Node{
	public:
		Node(int);
		bool hasKey(int) const noexcept;
		bool isLimitExceeded() const noexcept;	
		bool coalescible(int) const noexcept;
		std::string keysToString() const noexcept;
		virtual ~Node(){
		}

		Node* parent;
		std::vector<int> keys;

		friend class BpTree;
		friend class InteriorNode;
		friend class LeafNode;
	protected:
		virtual bool isFullEnough() const noexcept = 0;
		virtual bool isRedistributable() const noexcept = 0;
		virtual bool isLeafNode() const noexcept = 0;
		virtual void cleanNode() noexcept = 0;
		virtual Node* getNextNode(int) const noexcept = 0;
		Node* getLeftSibling() const noexcept;
		Node* getRightSibling() const noexcept;
		int getIndexOfKey(int) const noexcept;

		int keysLimit;
};

class InteriorNode : public Node{
	public:
		InteriorNode(int);
		InteriorNode(int, std::unique_ptr<Node>);
		bool isLeafNode() const noexcept override;
		Node* getNextNode(int) const noexcept;
		int split(InteriorNode*) noexcept;
		void insertKey(int, std::unique_ptr<Node>) noexcept;
		Node* removeKey(int, int) noexcept ;
		~InteriorNode(){}
		friend class BpTree;
		friend class Node;
	private:
		bool isFullEnough() const noexcept override;
		bool isRedistributable() const noexcept override;
		InteriorNode* getLeftSibling() const noexcept ;
		InteriorNode* getRightSibling() const noexcept;
		Node* redistributeLeftInterior(InteriorNode*) noexcept;
		Node* redistributeRightInterior(InteriorNode*) noexcept;
		Node* coalescLeftInterior(InteriorNode*, int) noexcept;
		Node* coalescRightInterior(InteriorNode*, int) noexcept;
		void cleanNode() noexcept override;		

		std::vector<std::unique_ptr<Node>> next; //List of pointers pointing to next nodes
};

class LeafNode : public Node{
	public:
		LeafNode(int);
		std::string getVal(int) const noexcept;
		std::string valsToString() const noexcept;
		bool isLeafNode() const noexcept override;
		Node* getNextNode(int i) const noexcept override;
		int split(LeafNode*) noexcept ;
		void insertKey(int, const std::string&) noexcept;
		Node* removeKey(int) noexcept ;
		~LeafNode(){}
		friend class BpTree;
		friend class Node;
	private:
		bool isFullEnough() const noexcept override;
		bool isRedistributable() const noexcept override;
		LeafNode* getLeftSibling() const noexcept ;
		LeafNode* getRightSibling() const noexcept ;
		Node* redistributeLeftLeaf(LeafNode*) noexcept;
		Node* redistributeRightLeaf(LeafNode*) noexcept;
		Node* coalescLeftLeaf(LeafNode*, int) noexcept;
		Node* coalescRightLeaf(LeafNode*, int) noexcept;
		void cleanNode() noexcept override;

		LeafNode* nextLeaf;
		LeafNode* prevLeaf;
		std::vector<std::string> vals; //List of values
	};



#endif
