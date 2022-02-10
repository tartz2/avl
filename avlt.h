/*avlt.h*/

//
// Threaded AVL tree
//

#pragma once

#include <iostream>
#include <vector>
#include <stack>

using namespace std;

template<typename KeyT, typename ValueT>
class avlt
{
private:
  struct NODE
  {
    KeyT   Key;
    ValueT Value;
    NODE*  Left;
    NODE*  Right;
    bool   isThreaded; // true => Right is a thread, false => non-threaded
    int    Height;     // height of tree rooted at this node
  };

  NODE* Root;  // pointer to root node of tree (nullptr if empty)
	NODE* RangeRoot;
  int   Size;  // # of nodes in the tree (0 if empty)
	
	NODE* ogRoot; // Allows "Root" to be moved while this one holds its original place
	bool hasBegun; // Checks whether or not the tree has called begin()

	
	
	// insertCopy()
	// 
	// Called when making a copy of a tree/node. It is called recursively
	// so that the whole tree ends up being copied to the new one.
	// If the node it's trying to insert is a nullptr, it doesnt insert and
	// returns
	// 
	void insertCopy(NODE* orig)
	{
		if(orig == nullptr)
			return;
			
		copy_insert(orig->Key, orig->Value, orig->Height);

		insertCopy(orig->Left);
		if(!orig->isThreaded)
			insertCopy(orig->Right);
		
	}
	
	/* copy_insert()
	 * 
	 * function to help the copy constructor and assignment operator
	 * by copying the contents of 1 tree in preorder fashion to another
	 * by inserting without rotations because it is already balanced
	 */
	
	void copy_insert(KeyT key, ValueT value, int height)
	{
		NODE* prev = nullptr;
		NODE* cur = ogRoot;
		
		bool goRight = false;
		bool goLeft = false;

		while (cur != nullptr)
		{  
			if (key == cur->Key)  // already in tree
				return;
				
			else if (key < cur->Key)  // search left:
			{
				prev = cur;
				cur = cur->Left;
				if(cur == nullptr)  // if cur is nullptr after moving left, it shouldnt be trying
					goLeft = true;    // to insert on the left side of this. it just fell out
			}
			else  //search right
			{
				if(cur->isThreaded) // if it hits this point and it's threaded and the key is greater
				{                   // than the current node, it wont go back up the tree. it will
					prev = cur;       // just insert on the right and make it unthreaded
					cur = nullptr;
					goRight = true;
					break;
				}
				else 
				{	
					prev = cur;
					cur = cur->Right;
					if(cur == nullptr) // if cur is nullptr after moving right, it shouldnt be trying
						goRight = true;  // to insert on the right of this. it just fell out here
				}
					
			}
			
		} //while
		
		
		
		
		// creating new node to insert in the tree...
		NODE* newNode = new NODE();

		newNode->Key = key;
		newNode->Value = value;
		newNode->Left = nullptr;
		newNode->Right = nullptr;
		newNode->Height = height;
		
		// Determining if its a new tree,
		if (prev == nullptr)
		{
		   Root = newNode;
			 ogRoot = Root;
		}
		
		// or a new node thats to the left of where it fell out,
		if(goLeft)
		{
			newNode->Right = prev;
			prev->Left = newNode;
			newNode->isThreaded = true;
		}
		
		// or a node to the right of where it fell out.
		if(goRight)
		{
			newNode->Right = prev->Right;
			prev->Right = newNode;
			//nodes.push(newNode);
			prev->isThreaded = false;     // dethreaded prev if it is going to be inserted on the right of prev
			
			if(newNode->Right != nullptr) // if prev was pointing at something, it uses its right to point there
				newNode->isThreaded = true;
			else
				newNode->isThreaded = false; // and if it wasn't, then it just makes 
		}                                // it null (could be on the right of the tree)
		
		Size++;
		
	}
	
	// inOrder()
	// 
	// called by the dump() function. This is a recursive function that 
	// prints the tree in order from smallest to largest value. If it is threaded,
	// it prints which node it is threaded to.
	// 
	void inOrder(NODE* cur, ostream& output) const
	{
		
	
		if(cur != nullptr)
		{
			inOrder(cur->Left, output);
			if(cur->isThreaded){
				output << "(" << cur->Key << "," << cur->Value << "," << cur->Height << "," << cur->Right->Key << ")"<< endl;
			} else {
				output << "(" << cur->Key << "," << cur->Value << "," << cur->Height << ")"<< endl;
				inOrder(cur->Right, output);
			}
			return;
		}
		return;
	}
	

	
	// clearTree()
	// 
	// function to clear the entire tree by recursively going from left to right
	// through the tree and then deleting the node before returning
	//  
	void clearTree(NODE* node)
	{
		if(node)
		{
			clearTree(node->Left);
			if(!node->isThreaded)
				clearTree(node->Right);
			delete node;
		}
		
		return;
	}
	
	/* assignHeight()
	 * 
	 * helper function to determine the height
	 * of a node. if null returns 1, otherwise return height
	 */

	int assignHeight(NODE* node){
		if(node == nullptr)
			return -1;
		else
			return node->Height;
	}
	
	/* right_rotate()
	 * 
	 * Helper function for insert/checkBalance to rotate the tree
	 * where the height on one side of a node differs by more than 1
	 * from the other side of a node
	 * 
	 * this rotates the top node of a sub tree down to the right, and
	 * brings the node on the left up to the top spot of that sub tree
	 * 
	 * then that new top node's right children go to the left of the
	 * original top node's left pointer
	 * 
	 * then the parent's pointer is updated
	 */
	
	void right_rotate(NODE* cur, NODE* par)  
	{ 
    NODE* cl = cur->Left;
		NODE* clR = cl->Right;
		
		int clLeft, clRight;
		int hclRL;
		
    // Perform rotation  
    cl->Right = cur;
		
		
		if(!cl->isThreaded){
			cur->Left = clR;
			hclRL = assignHeight(clR);		
		}
		else{
			cur->Left = nullptr;
			hclRL = -1;
		}
		
		clLeft = assignHeight(cl->Left);
		
    // Update heights 
    int curRightHeight;
    if(cur->isThreaded)
			curRightHeight = -1;
		else
			curRightHeight = assignHeight(cur->Right);
    cur->Height = max(hclRL, curRightHeight) + 1;
		
		if(!cl->isThreaded){
			clRight = assignHeight(cl->Right);
		}
		else {
			clRight = -1;
			cl->isThreaded = false;
		}
		
    cl->Height = max(clLeft, clRight) + 1;  
		
		//update parent
		if(par == nullptr){
			Root = cl;
			ogRoot = Root;
			return;
		}
		if(par->Left != nullptr){
			if(par->Left == cur){
				par->Left = cl;
			}
		}
		if(par->Right != nullptr){
			if(par->Right == cur){
				par->Right = cl;
			}
		}

}  
  
	/* left_rotate()
	 * 
	 * Helper function for insert/checkBalance to rotate the tree
	 * where the height on one side of a node differs by more than 1
	 * from the other side of a node
	 * 
	 * this rotates the top node of a sub tree down to the left, and
	 * brings the node on the right up to the top spot of that sub tree
	 * 
	 * then that new top node's left children go to the right of the
	 * original top node's left pointer
	 * 
	 * then the parent's pointer is updated
	 */

void left_rotate(NODE* cur, NODE* par)  
{  
    NODE* cr = cur->Right;  
    NODE* crL = cr->Left;  
		
		int crLeft, crRight;
		int hcrLR;
		
    // Perform rotation  
    cr->Left = cur;
		cur->Right = crL;
		
		if(cur->Right == nullptr)
		{
			cur->Right = cr;
			cur->isThreaded = true;
			hcrLR = -1;
		}
		else {
			hcrLR = assignHeight(crL);
		}
			
    // Update heights 
    cur->Height = max(hcrLR, assignHeight(cur->Left)) + 1;
		crLeft = assignHeight(cr->Left);
		
		if(cur->isThreaded)
			crRight = -1;
		else
			crRight = assignHeight(cr->Right);
			
    cr->Height = max(crLeft, crRight) + 1;  
		
		//update parent
		if(par == nullptr){
			Root = cr;
			ogRoot = Root;
			return;
		}
		if(par->Left != nullptr){
			if(par->Left == cur){
				par->Left = cr;
			}
		}
		if(par->Right != nullptr){
			if(par->Right == cur){
				par->Right = cr;
			}
		}
}  
 
	/* checkBalance()
	 * 
	 * Helper function for insert to check the balance of the nodes
	 * after a new one is inserted. it's passed a stack from insert
	 * from all the nodes it had to access to insert the new node.
	 * 
	 * it goes through the whole stack checking the height of the children
	 * and if they differ by too much, it will rotate accordingly based on 
	 * the position the nodes are in and where they differ
	 */

void checkBalance(stack<NODE*>& nodes, KeyT key)
	{
		NODE* cur = nullptr;
		NODE* prev = nullptr;
		
		while (!nodes.empty())
    {
      cur = nodes.top();
			if(cur == nullptr)
				break;

      nodes.pop();
			if(!nodes.empty())
				prev = nodes.top();
			else 
				prev = nullptr;
			
      int hL;
			int hR;
			
			if(cur->Left == nullptr)
				hL = -1;
			else
				hL = cur->Left->Height;
				
			if(cur->Right == nullptr || cur->isThreaded)
				hR = -1;
			 else 
				hR = cur->Right->Height;
			
			
      int hCur = 1 + std::max(hL, hR);
      if (cur->Height == hCur)  // didn't change, so no need to go further:
        break;
      else  // height changed, update and keep going:
        cur->Height = hCur;
			
			
			//find balance and rotate accordingly
			int balance = hL - hR;
			
			if(balance >= 2 && key < cur->Left->Key)       //Right Rotate
			{
				right_rotate(cur, prev);
			}
			else if(balance <= -2 && key > cur->Right->Key)     //Left Rotate
			{
				left_rotate(cur, prev);
			}
			else if(balance >= 2 && key > cur->Left->Key)       //Left Right Rotate
			{
				left_rotate(cur->Left, cur);
				right_rotate(cur, prev);	
			}
			else if(balance <= -2 && key < cur->Right->Key)       //Right Left Rotate
			{
				right_rotate(cur->Right, cur);
				left_rotate(cur, prev);
			}
			
		} //while
	}
	
	/* nextRange(NODE* node)
	 * 
	 * helper function for the search_range function.
	 * returns the next node by increasing key order to the 
	 * search_range function. First it movesd right, then it sees if
	 * it can move left while the keys are not equal to the original
	 * node's key
	 */
		
	NODE* nextRange(NODE* node)
	{
		if(node == nullptr){
			return node;
		}

		KeyT key = node->Key;
		node = node->Right;
		
		if(node != nullptr){
			while(node->Left != nullptr && node->Left->Key > key)
			{
				node = node->Left;
			}
		}
		return node;
	}
	
public:

  //
  // default constructor:
  //
  // Creates an empty tree.
  //
  avlt()
  {
    Root = nullptr;
    Size = 0;
		ogRoot = nullptr;
		hasBegun = false;
  }
	
	

  //
  // copy constructor
  //
  // NOTE: makes an exact copy of the "other" tree, such that making the
  // copy requires no rotations.
  //
  avlt (const avlt& other)
  {
    ogRoot = nullptr;
		Root = nullptr;
		insertCopy(other.Root);
		ogRoot = Root;
		Size = other.Size;
		hasBegun = other.hasBegun;
  }

	//
  // destructor:
  //
  // Called automatically by system when tree is about to be destroyed;
  // this is our last chance to free any resources / memory used by
  // this tree.
  //
  virtual ~avlt()
  {
		Root = ogRoot;
    clearTree(ogRoot);
		Size = 0;
  }

  //
  // operator=
  //
  // Clears "this" tree and then makes a copy of the "other" tree.
  //
  // NOTE: makes an exact copy of the "other" tree, such that making the
  // copy requires no rotations.
  //
  avlt& operator=(const avlt& other)
  {
    this->clear();
		Root = nullptr;
		insertCopy(other.Root);
		ogRoot = Root;
		Size = other.Size;
		hasBegun = other.hasBegun;
		return *this;
  }

  //
  // clear:
  //
  // Clears the contents of the tree, resetting the tree to empty.
  //
  void clear()
  {
    // calls clearTree and then resetting Root and ogRoot to nullptr
		clearTree(ogRoot);
		Root = nullptr;
		ogRoot = nullptr;
		Size = 0;
  }

  // 
  // size:
  //
  // Returns the # of nodes in the tree, 0 if empty.
  //
  // Time complexity:  O(1) 
  //
  int size() const
  {
    return Size;
  }

  // 
  // height:
  //
  // Returns the height of the tree, -1 if empty.
  //
  // Time complexity:  O(1) 
  //
  int height() const
  {
    if (Root == nullptr)
      return -1;
    else
      return Root->Height;
  }
	
	
	

  // 
  // search:
  //
  // Searches the tree for the given key, returning true if found
  // and false if not.  If the key is found, the corresponding value
  // is returned via the reference parameter.
  //
  // Time complexity:  O(lgN) worst-case
  //
  bool search(KeyT key, ValueT& value) const
	{
     //
     // TODO:
     //
		NODE* cur = Root;
		if(cur == nullptr)
		{
			return false;
		}
		
		
		while(cur != nullptr) // while cur is still a node
		{
			if(key == cur->Key) // checks if the key is found
			{
				value = cur->Value;
				return true;     // returns true if so and updates value
			} 
			else if(key < cur->Key)
			{
				cur = cur->Left;    // if cur->Key is bigger, it moves left (to a smaller)
			}                     // value and then runs the loop again
			else 
			{
				if(cur->isThreaded){ 
					return false;    // if the node is threaded but the key its trying to find is bigger
				}                  // it returns false because we dont need to go back up the tree. 
				else  
				{
					if(key > cur->Key)  // if its not threaded and the key is bigger than the node's key
						cur = cur->Right; // it moves right to a node with a greater value
				}
			}
		}

		return false; // if it breaks out of the loop it didn't find it and cur == nullptr
	}

  //
  // range_search
  //
  // Searches the tree for all keys in the range [lower..upper], inclusive.
  // It is assumed that lower <= upper.  The keys are returned in a vector;
  // if no keys are found, then the returned vector is empty.
  //
  // Time complexity: O(lgN + M), where M is the # of keys in the range
  // [lower..upper], inclusive.
  //
  // NOTE: do not simply traverse the entire tree and select the keys
  // that fall within the range.  That would be O(N), and thus invalid.
  // Be smarter, you have the technology.
  //

	vector<KeyT> range_search(KeyT lower, KeyT upper){
		//first go to most left node
		vector<KeyT> keys;
		
		if(Root == nullptr)
			return keys;

		NODE* current = nullptr;
		current = findLower(lower);
		
		if(current == nullptr){
			return keys;
		}

		if(current->Key < lower){
			return keys;
		}
		
		//now travel using right pointers
		while(current != nullptr && current->Key <= upper){
			keys.push_back(current->Key);
		
			current = nextRange(current);
		}
		
		return keys;
	}


	NODE* findLower(KeyT key){
		if(Root == nullptr){
			return Root;
		}
		
		NODE* cur = ogRoot;
		
		while(cur != nullptr) // while cur is still a node
		{
			//cout << "CUR KEY IS: " << cur->Key << endl;
			if(key == cur->Key) // checks if the key is found
			{
				return cur;     // returns true if so and updates value
			}
			else if(key < cur->Key)
			{
				if(cur->Left != nullptr){
					//cout << "SUPPOSED TO GO TO: "<< cur->Left->Key << endl;
					if(cur->Left->Key >= key || !cur->Left->isThreaded){
						cur = cur->Left;
						}
					else
						return cur;
				}
				else {
					return cur;						
				}
			}                     
			else 
			{
				if(cur->isThreaded){ 
					if(cur->Right->Key >= key)
						return cur->Right;
					else
						return nullptr;
				}                  
				else  
				{
						if(cur->Right != nullptr){
							cur = cur->Right;
						}
						else{
							return nullptr;
						}
				}
			}
		}
		return nullptr;
	}

	

  //
  // insert
  //
  // Inserts the given key into the tree; if the key has already been insert then
  // the function returns without changing the tree.  Rotations are performed
  // as necessary to keep the tree balanced according to AVL definition.
  //
  // Time complexity:  O(lgN) worst-case
  //
	
	void insert(KeyT key, ValueT value)
	{
		NODE* prev = nullptr;
		NODE* cur = ogRoot;
		stack<NODE*> nodes;
		
		bool goRight = false;
		bool goLeft = false;
	
		
		while (cur != nullptr)
		{  
		
			if (key == cur->Key)  // already in tree
				return;
				
			else if (key < cur->Key)  // search left:
			{
				prev = cur;
				nodes.push(cur);
				cur = cur->Left;
				if(cur == nullptr)  // if cur is nullptr after moving left, it shouldnt be trying
					goLeft = true;    // to insert on the left side of this. it just fell out
			}
			else  //search right
			{
				if(cur->isThreaded) // if it hits this point and it's threaded and the key is greater
				{                   // than the current node, it wont go back up the tree. it will
					prev = cur;       // just insert on the right and make it unthreaded
					nodes.push(cur);
					cur = nullptr;
					goRight = true;
					break;
				}
				else 
				{
					
					prev = cur;
					nodes.push(cur);
					cur = cur->Right;
					if(cur == nullptr) // if cur is nullptr after moving right, it shouldnt be trying
						goRight = true;  // to insert on the right of this. it just fell out here
				}
					
			}
			
		} //while
		
		// creating new node to insert in the tree...
		NODE* newNode = new NODE();

		newNode->Key = key;
		newNode->Value = value;
		newNode->Left = nullptr;
		newNode->Right = nullptr;
		newNode->Height = 0;
		
		// Determining if its a new tree,
		if (prev == nullptr)
		{
		   Root = newNode;
			 ogRoot = Root;
			 //nodes.push(newNode);
		}
		
		// or a new node thats to the left of where it fell out,
		if(goLeft)
		{
			newNode->Right = prev;
			prev->Left = newNode;
			//nodes.push(newNode);
			newNode->isThreaded = true;
		}
		
		// or a node to the right of where it fell out.
		if(goRight)
		{
			newNode->Right = prev->Right;
			prev->Right = newNode;
			//nodes.push(newNode);
			prev->isThreaded = false;     // dethreaded prev if it is going to be inserted on the right of prev
			
			if(newNode->Right != nullptr) // if prev was pointing at something, it uses its right to point there
				newNode->isThreaded = true;
			else
				newNode->isThreaded = false; // and if it wasn't, then it just makes 
		}                                // it null (could be on the right of the tree)
		
		Size++;
		
		// balance time
		
		checkBalance(nodes, key);

	}

  //
  // []
  //
  // Returns the value for the given key; if the key is not found,
  // the default value ValueT{} is returned.
  //
  // Time complexity:  O(lgN) worst-case
  //
  ValueT operator[](KeyT key) const
	{
    NODE* cur = ogRoot;
		while(cur != nullptr) // performs a search for the node with a key
		{
			if(key == cur->Key)
				return cur->Value;
				
			if(key < cur->Key)
				cur = cur->Left;
				
			else
			{
				if(cur->isThreaded)  // if the key is greater than the nodes key and its 
					return ValueT{};   // threaded, then the (key,value) node D.N.E.
				else      
					cur = cur->Right;
			}
		}
		

		return ValueT{ }; // if cur went to nullptr, it returns default
	}

  //
  // ()
  //
  // Finds the key in the tree, and returns the key to the "right".
  // If the right is threaded, this will be the next inorder key.
  // if the right is not threaded, it will be the key of whatever
  // node is immediately to the right.
  //
  // If no such key exists, or there is no key to the "right", the
  // default key value KeyT{} is returned.
  //
  // Time complexity:  O(lgN) worst-case
  //
  KeyT operator()(KeyT key) const
	{
    //
    // TODO
    //

    NODE* cur = ogRoot;
		
		if(ogRoot == nullptr) // if the real root d.n.e, there wont be one
		{                     // to the right
			return KeyT{};
		}
		
		cur = ogRoot;   // starts from the top of the tree

		while(cur != nullptr){
			if(key == cur->Key)
			{
				if(cur->Right != nullptr)      // found, return the value to the right
					return cur->Right->Key;   
				else
					return KeyT{};            // found, but the right node is null
			} 
			else if(key < cur->Key)
			{
				cur = cur->Left;  
			}
			else 
			{
				if(cur->isThreaded){
					return KeyT{};        // if it didnt find it, and its threaded
				}                       // dont go back, return default Key{}T
				else 
				{
					cur = cur->Right;
				}
			}
		}

			return KeyT{};       // if cur went to nullptr, return default
	}

  //
  // %
  //
  // Returns the height stored in the node that contains key; if key is
  // not found, -1 is returned.
  //
  // Example:  cout << tree%12345 << endl;
  //
  // Time complexity:  O(lgN) worst-case
  //
  int operator%(KeyT key) const
  {
    NODE* cur = Root;
		if(cur == nullptr)
		{
			return -1;
		}
		
		
		while(cur != nullptr) // while cur is still a node
		{
			if(key == cur->Key) // checks if the key is found
			{
				return cur->Height;     // returns true if so and updates value
			} 
			else if(key < cur->Key)
			{
				cur = cur->Left;    // if cur->Key is bigger, it moves left (to a smaller)
			}                     // value and then runs the loop again
			else 
			{
				if(cur->isThreaded){ 
					return -1;    // if the node is threaded but the key its trying to find is bigger
				}                  // it returns false because we dont need to go back up the tree. 
				else  
				{
					if(key > cur->Key)  // if its not threaded and the key is bigger than the node's key
						cur = cur->Right; // it moves right to a node with a greater value
				}
			}
		}

    return -1;
  }

  //
  // begin
  //
  // Resets internal state for an inorder traversal.  After the 
  // call to begin(), the internal state denotes the first inorder
  // key; this ensure that first call to next() function returns
  // the first inorder key.
  //
  // Space complexity: O(1)
  // Time complexity:  O(lgN) worst-case
  //
  // Example usage:
  //    tree.begin();
  //    while (tree.next(key))
  //      cout << key << endl;
  //
  void begin()
	{
		if(ogRoot == nullptr){
			return;
		}
		
		Root = ogRoot; // moving root back to the original spot
		
		while(Root->Left != nullptr) // and then moving all the way left
		{                            // to first inorder node
			Root = Root->Left;
		}
		
		hasBegun = true; // lets the tree know it has used begin() so it may use next()
    return;
	}


  //
  // next
  //
  // Uses the internal state to return the next inorder key, and 
  // then advances the internal state in anticipation of future
  // calls.  If a key is in fact returned (via the reference 
  // parameter), true is also returned.
  //
  // False is returned when the internal state has reached null,
  // meaning no more keys are available.  This is the end of the
  // inorder traversal.
  //
  // Space complexity: O(1)
  // Time complexity:  O(lgN) worst-case
  //
  // Example usage:
  //    tree.begin();
  //    while (tree.next(key))
  //      cout << key << endl;
  //
  bool next(KeyT& key)
	{
    //
    // TODO
    //
    if(Root == nullptr || hasBegun == false) // checks that it can use the function
		{
			return false;
		}
	
		key = Root->Key;         // updates key to current Root's key
		Root = Root->Right;      // moves the Root to the right no matter what
		
		if(Root != nullptr){                                     // now it checks if it can move left
			while(Root->Left != nullptr && Root->Left->Key > key)  // so it gets the next inorder value 
			{                                                      // and then it will move if the left values
				Root = Root->Left;                                   // are greater than the key it updated
			}
		}
		return true;
		
	}


  //
  // dump
  // 
  // Dumps the contents of the tree to the output stream, using a
  // recursive inorder traversal.
  //
  void dump(ostream& output) const
	{
		output << "**************************************************" << endl;
		output << "********************* AVLT ***********************" << endl;

		output << "** size: " << this->size() << endl;
		output << "** height: " << this->height() << endl;
    //
    // inorder traversal, with one output per line: either 
    // (key,value) or (key,value,THREAD)
    //
    // (key,value) if the node is not threaded OR thread==nullptr
    // (key,value,THREAD) if the node is threaded and THREAD denotes the next inorder key
    //

    //
    // TODO
    //
    
		
		NODE* cur = ogRoot;
		inOrder(cur, output);     // calls the recursive function to print
		
		output << "**************************************************" << endl;
	}
	
	
};

