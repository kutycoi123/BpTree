#include <iostream>
#include "BpTree.h"

int main(){
  BpTree bptree(4);
  bptree.insert(30, "A");
  bptree.insert(130, "B");
  bptree.insert(9, "C");
  bptree.insert(81, "D");
  bptree.insert(150, "E");
  bptree.insert(40, "F");
  bptree.insert(1, "G");
  std::cout << "Printing out keys:\n";
  bptree.printKeys();
  std::cout << "Printing out values:\n";
  bptree.printValues();
  return 0;
}
