#include "Node.h"


/*======== NODE class implementation ========*/
Node::Node(int limit) : keysLimit(limit), parent(nullptr){
}

bool Node::hasKey(int k) const noexcept{
	auto it = std::find(keys.begin(), keys.end(), k);
	return it != keys.end();

}


int Node::getIndexOfKey(int k) const noexcept{
	auto it = std::find(keys.begin(), keys.end(), k);
	if(it != keys.end()){
		return std::distance(keys.begin(), it);
	}
	return -1;
}


bool Node::isLimitExceeded() const noexcept{
	return keys.size() > keysLimit;
}



std::string Node::keysToString() const noexcept{
	if(keys.size() == 0)
		return "[]";
	std::string res = "[";
	std::for_each(keys.begin(), keys.end(), 
					[&res](int key){
						res += std::to_string(key) + ",";
					});
	res.pop_back(); //pop the last ',' 	
	res += "]";
	return res;	
}

Node* Node::getLeftSibling() const noexcept{
	if(!parent){
		return nullptr;
	}
	auto par = static_cast<InteriorNode*>(parent);
	auto keyAssociatedWithThisNode = std::upper_bound(par->keys.begin(), par->keys.end(), keys.front());
	int idxOfNextPtr = std::distance(par->keys.begin(), keyAssociatedWithThisNode);
	if(idxOfNextPtr > 0){
		return par->next[idxOfNextPtr-1].get();
	}
	return nullptr;
}

Node* Node::getRightSibling() const noexcept{
	if(!parent){
		return nullptr;
	}
	auto par = static_cast<InteriorNode*>(parent);
	auto keyAssociatedWithThisNode = std::upper_bound(par->keys.begin(), par->keys.end(), keys.front());
	int idxOfNextPtr = std::distance(par->keys.begin(), keyAssociatedWithThisNode);
	if(idxOfNextPtr+1 < par->next.size()){
		return par->next[idxOfNextPtr+1].get();
	}
	return nullptr;

}
bool Node::coalescible(int k) const noexcept{
	return keys.size() + k <= keysLimit ;
}
/*===================== END OF NODE =======================================*/


/*==================== InteriorNode class implementation ==========================*/
InteriorNode::InteriorNode(int keysLimit) : Node(keysLimit){
	next.reserve(keysLimit+1);
} 

InteriorNode::InteriorNode(int keysLimit, std::unique_ptr<Node> n) : Node(keysLimit){
	next.push_back(std::move(n));
}

bool InteriorNode::isLeafNode() const noexcept{
	return false;
}

bool InteriorNode::isFullEnough() const noexcept{
	return next.size() >= ((keysLimit + 1) / 2);
}

bool InteriorNode::isRedistributable() const noexcept{
	return next.size() - 1 >= ((keysLimit + 1) / 2);
}

void InteriorNode::cleanNode() noexcept{
	keys.clear();
	next.clear();
}
void InteriorNode::insertKey(int k, std::unique_ptr<Node> newNode) noexcept{
	auto keyPos = std::upper_bound(keys.begin(), keys.end(), k);
	auto nextPos = next.begin() + std::distance(keys.begin(), keyPos) + 1;
	keys.insert(keyPos, k);	
	newNode->parent = this;
	next.insert(nextPos, std::move(newNode));
}

Node* InteriorNode::getNextNode(int k) const noexcept{

	auto upperBoundOfK = std::upper_bound(keys.begin(), keys.end(), k);
	auto nextNodeIndex = std::distance(keys.begin(), upperBoundOfK);
		
	return next[nextNodeIndex].get();
}

int InteriorNode::split(InteriorNode* newSplitNode) noexcept{

	//Split keys in original interior node to new split node
	int numOfRemainingKeys = ceil(keysLimit/2.0);
	int numOfSplitKeys = floor(keysLimit/2.0);
	auto keysBegin = keys.begin();
	auto keysEnd = keys.end();

	newSplitNode->keys.resize(numOfSplitKeys); //Allocate memories for split keys
	
	std::copy(keysBegin + numOfRemainingKeys + 1, keysEnd, newSplitNode->keys.begin()); //Spit keys to new node
	
	keys.erase(keysBegin + numOfRemainingKeys + 1, keysEnd); //Erase split keys and keep remaining keys in original node
	

	//Last key is now to be removed and inserted into node's parent
	//The rest keys are kept in original node
	auto removedKey = keys.back();
	keys.pop_back();

	//Split next node pointers
	auto nextNodesBegin = next.begin();
	auto nextNodesEnd = next.end();
	int numOfNextNodesInOriginalNode = ceil((keysLimit+2)/2.0);
	
	auto nextNodesInSplitNodeBegin = nextNodesBegin + numOfNextNodesInOriginalNode;
	for(auto it = nextNodesInSplitNodeBegin; it != nextNodesEnd ; ++it){
		if((*it) != nullptr)
			(*it)->parent = newSplitNode;
		newSplitNode->next.push_back(std::move(*it));
	}
	
	next.erase(nextNodesInSplitNodeBegin, nextNodesEnd);
	
	return removedKey;

}

InteriorNode* InteriorNode::getLeftSibling() const noexcept{
	return static_cast<InteriorNode*>(Node::getLeftSibling());	
		
}

InteriorNode* InteriorNode::getRightSibling() const noexcept{
	return static_cast<InteriorNode*>(Node::getRightSibling());
}

Node* InteriorNode::removeKey(int k, int keyOfNextNodeToBeRemoved) noexcept{
	auto keyPos = getIndexOfKey(k);
	if(keyPos == -1)
		return this;
	auto nextPos = keyPos; 
	if(keyOfNextNodeToBeRemoved >= keys[keyPos])
		nextPos = keyPos + 1;
	keys.erase(keys.begin() + keyPos);
	next.erase(next.begin() + nextPos);
	if(isFullEnough())
		return this;
	InteriorNode* leftSibling = getLeftSibling();
	InteriorNode* rightSibling = getRightSibling();

	if(leftSibling && leftSibling->isRedistributable()){
		return redistributeLeftInterior(leftSibling);
	}else if(rightSibling && rightSibling->isRedistributable()){
		return redistributeRightInterior(rightSibling);
	}else if(leftSibling && leftSibling->coalescible(keys.size())){
		return coalescLeftInterior(leftSibling, k);	
	}else if(rightSibling && rightSibling->coalescible(keys.size())){
		return coalescRightInterior(rightSibling, k);
	}
	return this;	
}

Node* InteriorNode::redistributeLeftInterior(InteriorNode* sibling) noexcept{
	Node* par = sibling->parent;

	auto upperBoundOfRedistributedKey = std::upper_bound(par->keys.begin(), par->keys.end(), sibling->keys.front());
	auto parentKey = upperBoundOfRedistributedKey - 1;	
	keys.insert(keys.begin(), *parentKey);
	*parentKey = sibling->keys.back();
	sibling->keys.pop_back();

	std::unique_ptr<Node> newNext = std::move(sibling->next.back());
	sibling->next.pop_back();
	newNext->parent = this;
	next.insert(next.begin(), std::move(newNext));
	return this;
}

Node* InteriorNode::redistributeRightInterior(InteriorNode* sibling) noexcept{
	Node* par = sibling->parent;
	
	auto parentKey = std::upper_bound(par->keys.begin(), par->keys.end(), keys.front());
	keys.insert(keys.end(), *parentKey);
	*parentKey = sibling->keys.front();
	sibling->keys.erase(sibling->keys.begin());

	std::unique_ptr<Node> newNext = std::move(sibling->next.front());
	newNext->parent = this;
	sibling->next.erase(sibling->next.begin());

	next.insert(next.end(), std::move(newNext));
	return this;	
}

Node* InteriorNode::coalescLeftInterior(InteriorNode* sibling, int firstKeyAlreadyRemoved) noexcept{
	if(!sibling->coalescible(keys.size()))
		return this;

	auto keyBetween2Coalesced = std::upper_bound(parent->keys.begin(), parent->keys.end(), sibling->keys.front());
	sibling->keys.insert(sibling->keys.end(), *keyBetween2Coalesced);
	sibling->keys.insert(sibling->keys.end(), keys.begin(), keys.end());
	//Update parent pointer of each next node before coalescing with sibling
	for(auto& nextNode : next){
		nextNode->parent = static_cast<Node*>(sibling);
	}
	sibling->next.insert(sibling->next.end(), std::make_move_iterator(next.begin()), std::make_move_iterator(next.end()));	

	int keyToBeRemoved = firstKeyAlreadyRemoved;
	if(keys.size() > 0)
		keyToBeRemoved = keys.front();
	cleanNode();

	return static_cast<InteriorNode*>(parent)->removeKey(*keyBetween2Coalesced, keyToBeRemoved);
	
}

Node* InteriorNode::coalescRightInterior(InteriorNode* sibling, int firstKeyAlreadyRemoved) noexcept{
	if(!sibling->coalescible(keys.size()))
		return this;
	
	auto keyBetween2Coalesced = std::upper_bound(parent->keys.begin(), parent->keys.end(), keys.front());
	sibling->keys.insert(sibling->keys.begin(), *keyBetween2Coalesced);
	sibling->keys.insert(sibling->keys.begin(), keys.begin(), keys.end());
	//Update parent pointer of each next node before coalescing with sibling
	for(auto& nextNode : next){
		nextNode->parent = static_cast<Node*>(sibling);
	}
	sibling->next.insert(sibling->next.begin(), std::make_move_iterator(next.begin()), std::make_move_iterator(next.end()));

	int keyToBeRemoved = firstKeyAlreadyRemoved;
	if(keys.size() > 0)
		keyToBeRemoved = keys.front();
	cleanNode();

	return static_cast<InteriorNode*>(parent)->removeKey(*keyBetween2Coalesced, keyToBeRemoved);
}
	
/*===================== End of InteriorNode =========================================*/

/*==================== LeafNode class implementation =================================*/
LeafNode::LeafNode(int keysLimit) : Node(keysLimit), nextLeaf(nullptr), prevLeaf(nullptr){
	vals.reserve(keysLimit);	
}

void LeafNode::cleanNode() noexcept{
	keys.clear();
	vals.clear();
}
bool LeafNode::isFullEnough() const noexcept{
	return vals.size() >= ((keysLimit + 1) / 2);
}

bool LeafNode::isRedistributable() const noexcept{
	return vals.size() - 1 >= ((keysLimit + 1) / 2);
}

std::string LeafNode::getVal(int k) const noexcept{
	int keyIndex = getIndexOfKey(k);
	if(keyIndex == -1)
		return "";
	return vals[keyIndex];

}

std::string LeafNode::valsToString() const noexcept{
	std::string res = "";
	std::for_each(vals.begin(), vals.end(), 
					[&res](const std::string& val){
						res += val + "\n";
					});
	res.pop_back();
	return res;
}

bool LeafNode::isLeafNode() const noexcept{
	return true;
}


Node* LeafNode::getNextNode(int i) const noexcept{
	return nextLeaf;
}

void LeafNode::insertKey(int k, const std::string& v) noexcept{
	auto keyPos = std::upper_bound(keys.begin(), keys.end(), k);
	auto valPos = vals.begin() + std::distance(keys.begin(), keyPos);
	keys.insert(keyPos, k);
	vals.insert(valPos, v); 

}

int LeafNode::split(LeafNode* newSplitNode) noexcept{
	//Split keys
	auto keysBegin= keys.begin();
	auto keysEnd = keys.end();
	int numOfRemainingKeys = ceil((keysLimit + 1)/2.0);	
	int numOfSplitKeys = keys.size() - numOfRemainingKeys;

	newSplitNode->keys.resize(numOfSplitKeys); //Allocate memories for new node's keys
	
	std::copy(keysBegin + numOfRemainingKeys, keysEnd, (newSplitNode->keys).begin()); //Split keys to new node
	keys.erase(keysBegin + numOfRemainingKeys, keysEnd); //Erase split keys and keep remaining keys in original node

	
	//Split values
	auto valsBegin = vals.begin();
	auto valsEnd = vals.end();
	auto numOfRemainingVals = numOfRemainingKeys;
	newSplitNode->vals.resize(numOfSplitKeys); //Allocate memories for new node's vals

	std::copy(valsBegin + numOfRemainingVals, valsEnd, newSplitNode->vals.begin()); //Split vals to new node
	vals.erase(valsBegin + numOfRemainingVals, valsEnd); //Erase split vals and keep remaining vals in original node

	newSplitNode->nextLeaf = nextLeaf;
	newSplitNode->prevLeaf = this;
	nextLeaf = newSplitNode;
	if(newSplitNode->nextLeaf)
		newSplitNode->nextLeaf->prevLeaf = newSplitNode;
	return newSplitNode->keys.front();
}

LeafNode* LeafNode::getLeftSibling() const noexcept{
	return static_cast<LeafNode*>(Node::getLeftSibling());
}

LeafNode* LeafNode::getRightSibling() const noexcept{
	return static_cast<LeafNode*>(Node::getRightSibling());
}

Node* LeafNode::removeKey(int k) noexcept{
	auto keyPos = getIndexOfKey(k);
	if(keyPos == -1) 
		return this;
	auto valPos = keyPos;
	keys.erase(keys.begin() + keyPos);
	vals.erase(vals.begin() + valPos);
	if(isFullEnough())
		return this;
	LeafNode* leftSibling = getLeftSibling();
	LeafNode* rightSibling = getRightSibling();
	
	if(leftSibling && leftSibling->isRedistributable()){
		return redistributeLeftLeaf(leftSibling);
	}else if(rightSibling && rightSibling->isRedistributable()){
		return redistributeRightLeaf(rightSibling);
	}else if(leftSibling && leftSibling->coalescible(keys.size())){
		return coalescLeftLeaf(leftSibling, k);		
	}else if(rightSibling && rightSibling->coalescible(keys.size())){
		return coalescRightLeaf(rightSibling, k);
	}
	return this;
}

Node* LeafNode::redistributeLeftLeaf(LeafNode* sibling) noexcept{
	int distributedKey = sibling->keys.back();
	sibling->keys.pop_back();
	std::string distributedVal = sibling->vals.back();
	sibling->vals.pop_back();	
	
	keys.insert(keys.begin(), distributedKey);
	vals.insert(vals.begin(), distributedVal); 
	
	Node* par = parent;
	auto parentKey = std::upper_bound(par->keys.begin(), par->keys.end(), distributedKey);
	*parentKey = distributedKey;
	return this;
}

Node* LeafNode::redistributeRightLeaf(LeafNode* sibling) noexcept{
	int distributedKey = sibling->keys.front();
	sibling->keys.erase(sibling->keys.begin());
	std::string distributedVal = sibling->vals.front();
	sibling->vals.erase(sibling->vals.begin());

	keys.push_back(distributedKey);
	vals.push_back(distributedVal);	


	Node* par = parent;
	auto upperBoundOfDistributedKey = std::upper_bound(par->keys.begin(), par->keys.end(), distributedKey);
	auto parentKey = upperBoundOfDistributedKey - 1; 
	*parentKey = sibling->keys.front();
	return this;

}

Node* LeafNode::coalescLeftLeaf(LeafNode* sibling, int firstKeyAlreadyRemoved) noexcept{
	if(!sibling->coalescible(keys.size()))
		return this;
	sibling->keys.insert(sibling->keys.end(), keys.begin(), keys.end());
	sibling->vals.insert(sibling->vals.end(), vals.begin(), vals.end());	
	
	auto parentKey = std::upper_bound(sibling->parent->keys.begin(), sibling->parent->keys.end(),sibling->keys.front());
	sibling->nextLeaf = nextLeaf;
	if(nextLeaf)
		nextLeaf->prevLeaf = sibling;

	int keyToBeRemoved = firstKeyAlreadyRemoved;
	if(keys.size() > 0)
		keyToBeRemoved = keys.front();

	cleanNode();

	return static_cast<InteriorNode*>(parent)->removeKey(*parentKey, keyToBeRemoved);
}

Node* LeafNode::coalescRightLeaf(LeafNode* sibling, int firstKeyAlreadyRemoved) noexcept{
	if(!sibling->coalescible(keys.size()))
		return this;
	sibling->keys.insert(sibling->keys.begin(), keys.begin(), keys.end());
	sibling->vals.insert(sibling->vals.begin(), vals.begin(), vals.end());

	int keyToBeRemoved = firstKeyAlreadyRemoved;
	if(keys.size() > 0)
		keyToBeRemoved = keys.front();

	auto parentKey = std::upper_bound(sibling->parent->keys.begin(), sibling->parent->keys.end(), keyToBeRemoved);
	sibling->prevLeaf = prevLeaf;
	if(prevLeaf)
		prevLeaf->nextLeaf = sibling;
	

	cleanNode();

	return static_cast<InteriorNode*>(parent)->removeKey(*parentKey, keyToBeRemoved);

}

/*=======================End of LeafNode ============================================*/

