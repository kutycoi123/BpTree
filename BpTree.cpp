#include "BpTree.h"

BpTree::BpTree() : BpTree(3){
}
BpTree::BpTree(int limit) : keysLimit(limit){
	root = std::unique_ptr<Node>(new LeafNode(limit));	
}

BpTree::BpTree(const BpTree& tree) : keysLimit(tree.keysLimit){
		
	root = deepCopy(tree.root.get(), nullptr);
	connectAllLeafs();	
}

BpTree& BpTree::operator= (const BpTree& tree) noexcept{
	keysLimit = tree.keysLimit;
	std::move(root);
	root = deepCopy(tree.root.get(), nullptr);
	connectAllLeafs();
	return *this;
}

std::unique_ptr<Node> BpTree::deepCopy(Node* node, Node* par) noexcept{
	std::unique_ptr<Node> copy;
	if(node->isLeafNode()){
		copy = std::unique_ptr<Node>(new LeafNode(node->keysLimit));
		copy->keys = node->keys;
		static_cast<LeafNode*>(copy.get())->vals = static_cast<LeafNode*>(node)->vals;
	}else{
		copy = std::unique_ptr<Node>(new InteriorNode(node->keysLimit));
		copy->keys = node->keys;
		InteriorNode* castedCopy = static_cast<InteriorNode*>(copy.get());
		InteriorNode* castedNode = static_cast<InteriorNode*>(node);
		
		for(auto& elem : castedNode->next){
			castedCopy->next.push_back(std::move(deepCopy(elem.get(), castedCopy)));
		}
	}
	copy->parent = par;
	return copy;
}

void BpTree::getAllLeafNodes(Node* cur, std::vector<LeafNode*>& leafNodes) const noexcept{
	if(cur->isLeafNode())	
		leafNodes.push_back(static_cast<LeafNode*>(cur));
	else{
		InteriorNode* tmp = static_cast<InteriorNode*>(cur);
		for(auto& nextNode : tmp->next){
			getAllLeafNodes(nextNode.get(), leafNodes);
		}
	}

}

void BpTree::connectAllLeafs() noexcept{
	std::vector<LeafNode*> leafNodes;
	getAllLeafNodes(root.get(), leafNodes);
	if(leafNodes.size() > 0){
		leafNodes.push_back(nullptr);
		leafNodes.insert(leafNodes.begin(), nullptr);
		for(int i = 1; i < leafNodes.size() - 1; ++i){
			leafNodes[i]->nextLeaf = leafNodes[i+1];
			leafNodes[i]->prevLeaf = leafNodes[i-1];	
		}		
	}
}
LeafNode* BpTree::findNodeOfKey(int k) const noexcept{
	if(root->isLeafNode())
		return static_cast<LeafNode*>(root.get());
	Node* cur = static_cast<Node*>(root.get());
	while(cur != nullptr){
		if(cur->isLeafNode()){
			return static_cast<LeafNode*>(cur);
		}
		cur = static_cast<InteriorNode*>(cur->getNextNode(k));
	}
	return nullptr;

}

std::string BpTree::find(int k) const noexcept{
	LeafNode* foundNode = findNodeOfKey(k);
	if(foundNode && foundNode->hasKey(k))
		return foundNode->getVal(k);	
	return "";
}

void BpTree::printKeys() const noexcept{
	std::vector<Node*> nodeList;
	nodeList.push_back(root.get());
	while(!nodeList.empty()){
		std::vector<Node*> tmp;
		//Loop through each node
		for(auto& node : nodeList){
			std::string out = node->keysToString();
			std::cout << out << " ";
			if(node->isLeafNode()) continue;
			auto castedNode = static_cast<InteriorNode*>(node);
			//Push all the next node into the list to process in next loop
			for(auto& nextNode : castedNode->next){
				if(nextNode.get() != nullptr)
					tmp.push_back(nextNode.get());
			}
			
		}
		std::cout << "\n";
		//Update the node list for next loop
		nodeList = std::move(tmp);	
	}


}

void BpTree::printValues() const noexcept{
	Node* currentNode = root.get();

	//while leaftMostLeafNode is still interiornode
	while(currentNode && 
			!static_cast<InteriorNode*>(currentNode)->isLeafNode()){ 
		currentNode = static_cast<InteriorNode*>(currentNode)->next.front().get();	
	}

	std::string out;
	LeafNode* leftMostLeafNode = static_cast<LeafNode*>(currentNode);
	//Loop through all leaf nodes by following the nextLeaf pointer of each leaf node
	while(leftMostLeafNode){
		std::for_each(leftMostLeafNode->vals.begin(), leftMostLeafNode->vals.end(), 
						[&out](const std::string& val){
							out += val + "\n";				
						});
		leftMostLeafNode = static_cast<LeafNode*>(leftMostLeafNode)->nextLeaf;
	}
	std::cout << out;
}

bool BpTree::insert(int k, const std::string& v) noexcept{

	LeafNode* node = findNodeOfKey(k);	
	if(!node)
		return false;
	if(node->hasKey(k)) //Duplicated key
		return false;
	insertLeafNode(node, k, v);	
	return true;

}

void BpTree::insertLeafNode(LeafNode* node, int k, const std::string& v){

	node->insertKey(k, v);

  // Limit is not exceeded, so no more work to do
	if(!node->isLimitExceeded()){
		return;
	}

  // Need to do some split work
	if(!node->parent){ //node is root
		std::unique_ptr<Node> newRootNode(new InteriorNode(keysLimit, std::move(root)));
		root = std::move(newRootNode);
		node->parent = root.get();
	}		
	
	
	std::unique_ptr<Node> rightNode(new LeafNode(keysLimit));
	
	int newKeyInsertedKeyInParent = node->split(static_cast<LeafNode*>(rightNode.get()));	

	insertInteriorNode(static_cast<InteriorNode*>(node->parent), 
						newKeyInsertedKeyInParent, std::move(rightNode));
	
}

void BpTree::insertInteriorNode(InteriorNode*node, int k, std::unique_ptr<Node> newNode){
	
	node->insertKey(k, std::move(newNode));
	
	
	if(!node->isLimitExceeded()){
		return ;
	}
	
	if(!node->parent){ //node is root
		std::unique_ptr<Node> newRootNode(new InteriorNode(keysLimit));
		node = static_cast<InteriorNode*>(root.release());
		root = std::move(newRootNode);
		auto tmp = static_cast<InteriorNode*>(root.get());
		tmp->next.push_back(std::unique_ptr<Node>(node));
		node->parent = root.get();
	}	
	
	
	std::unique_ptr<Node> splitNode(new InteriorNode(keysLimit));

		
	auto removedKey = node->split(static_cast<InteriorNode*>(splitNode.get()));
	
	insertInteriorNode(static_cast<InteriorNode*>(node->parent), removedKey, std::move(splitNode));

}

bool BpTree::remove(int k) noexcept{

	LeafNode* node = findNodeOfKey(k);	
	if(!node)
		return false;
	if(!node->hasKey(k))
		return false;
	Node* lastModifiedNode = node->removeKey(k); 
	if(root.get() == lastModifiedNode && root->keys.empty()){
		if(!static_cast<InteriorNode*>(lastModifiedNode)->next.empty()){
			root = std::move(static_cast<InteriorNode*>(lastModifiedNode)->next.front());
			root->parent = nullptr;
		}
	}
	return true;
}
