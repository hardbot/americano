#include "BTreeNode.h"
#include <iostream>

using namespace std;

/* testing git again
testing git one more time
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	return pf.read(pid, buffer);
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
  return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
  int count = 0;
  memcpy(&count, buffer, sizeof(int));
  return count;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to 1nsert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */

//assume that before insert is called, the LeafNode Page file is loaded into buffer
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
  int eid = -1;
  int num_elements = getKeyCount();
  int element_size = sizeof(struct LeafNodeElement);
  LeafNodeElement element, left_element, right_element, temp;

  // Check for negative keys
  if (key < 0)
    return RC_INVALID_ATTRIBUTE;
  // Check if node is full
  if(num_elements >= MAX_NUM_POINTERS-1)
    return RC_NODE_FULL;
  if(locate(key, eid) == 0 && get_element(eid).key == key)
  {
    cout << "DUPLICATE DETECTED" << endl;
    exit(1);
    return -1;
  }

  // Set element to insert
  element.rec_id.pid = rid.pid;
  element.rec_id.sid = rid.sid;
  element.key = key;
  num_elements = getKeyCount();
  set_element(element, num_elements);

  // Bubble Sort 
  for(int i = num_elements; i > 0; i--)
  {
    // Get elements from buffer
    left_element = get_element(i-1);
    right_element = get_element(i);

    if(right_element.key < left_element.key)
    {
      // Swap elements
      temp.key = right_element.key;
      temp.rec_id.pid = right_element.rec_id.pid;
      temp.rec_id.sid = right_element.rec_id.sid;
      right_element.key = left_element.key;
      right_element.rec_id.pid = left_element.rec_id.pid;
      right_element.rec_id.sid = left_element.rec_id.sid;
      left_element.key = temp.key;
      left_element.rec_id.pid = temp.rec_id.pid;
      left_element.rec_id.sid = temp.rec_id.sid;

      // Write swap to buffer
      set_element(left_element, i-1);
      set_element(right_element, i);
    }
    else
    {
      break;
    }

  }
  // Iterate size
  num_elements++;
  memcpy(buffer, &num_elements, sizeof(int));

  return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
    BTLeafNode& sibling, int& siblingKey)
{ 
  //int eid = -1;
  int num_elements = getKeyCount();
  int element_size = sizeof(struct LeafNodeElement);
  int num_overflow = num_elements + 1;
  int half = (num_overflow+1)/2;
  LeafNodeElement tmp;
  int eid = -1;
  if(locate(key, eid) == 0 && get_element(eid).key == key)
  {
    cout << "DUPLICATE DETECTED" << endl;
    exit(1);
    return -1;
  }

  // Check for negative parameters
  if (key < 0)
    return RC_INVALID_ATTRIBUTE;
  // Check if sibling is empty
  if (sibling.getKeyCount() != 0)
    return RC_INVALID_ATTRIBUTE;

  // Hold all overflow elements in temp array
  LeafNodeElement *overflow = new LeafNodeElement[num_overflow];
  for (int i = 0; i < num_elements; i++)
  {
    overflow[i].key = get_element(i).key;
    overflow[i].rec_id.sid = get_element(i).rec_id.sid;
    overflow[i].rec_id.pid = get_element(i).rec_id.pid;

  }
  overflow[num_elements].key = key;
  overflow[num_elements].rec_id.sid = rid.sid;
  overflow[num_elements].rec_id.pid = rid.pid;

  // Backwards Bubble Sort
  for (int i = num_elements; i > 0; i--)
  {
    if(overflow[i].key < overflow[i-1].key)
    {
      // Swap Elements
      tmp.key = overflow[i].key;
      tmp.rec_id.pid = overflow[i].rec_id.pid;
      tmp.rec_id.sid = overflow[i].rec_id.sid;
      overflow[i].key = overflow[i-1].key;
      overflow[i].rec_id.pid = overflow[i-1].rec_id.pid;
      overflow[i].rec_id.sid = overflow[i-1].rec_id.sid;
      overflow[i-1].key = tmp.key;
      overflow[i-1].rec_id.pid = tmp.rec_id.pid;
      overflow[i-1].rec_id.sid = tmp.rec_id.sid;
    }
    else
    {
      break;
    }
  }

  // Clear current node
  // Set number of elemnts in node to zero
  num_elements = 0;
  memcpy(buffer, &num_elements, sizeof(int));

  // Insert into current node
  for (int i = 0; i < half; i++)
  {
    insert(overflow[i].key, overflow[i].rec_id);
  }

  // Insert into sibling node
  for (int i = half; i < num_overflow; i++)
  {
    sibling.insert(overflow[i].key, overflow[i].rec_id);
  }
  siblingKey = overflow[half].key;

  free(overflow);

  return 0; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
  int num_elements = getKeyCount();

  // Check for negative parameters
  if (searchKey < 0)
    return RC_INVALID_ATTRIBUTE;
  if (num_elements <= 0)
    return RC_END_OF_TREE;

  for (int i = 0; i < num_elements; i++)
  {
    if (searchKey <= get_element(i).key)
    {
      eid = i;
      return 0;
    }
  }
  return RC_NO_SUCH_RECORD;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
  //error
  if( eid > getKeyCount() ) 
    return RC_INVALID_ATTRIBUTE;
  LeafNodeElement lfe = get_element(eid);

  key = lfe.key;
  rid = lfe.rec_id;
  return 0;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
  PageId pid;
  memcpy(&pid,buffer+4,sizeof(int));
  return pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
  if( pid < 0 ) 
    return RC_INVALID_PID;
  memcpy(buffer+4,&pid,sizeof(int));
  return 0;
}

LeafNodeElement BTLeafNode::get_element(int eid)
{
  LeafNodeElement lfe;
  int element_size = sizeof(struct LeafNodeElement);
  memcpy(&lfe,buffer+8+(eid * element_size),element_size);
  return lfe;
}

void BTLeafNode::set_element(LeafNodeElement lfe, int eid)
{
  int element_size = sizeof(struct LeafNodeElement);
  memcpy(buffer+8+(eid * element_size),&lfe, element_size);
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
  return pf.read(pid, buffer);
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
  return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
  int count = 0;
  memcpy(&count, buffer, sizeof(int));
  return count;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
  //int eid = -1;
  int num_elements = getKeyCount();
  int element_size = sizeof(struct NonLeafNodeElement);
  NonLeafNodeElement element, left_element, right_element, temp;

  //check for negative keys
  if(key < 0)
    return RC_INVALID_ATTRIBUTE;
  if(pid < 0)
    return RC_INVALID_PID;
  //check if node is full
  if(num_elements >= MAX_NUM_POINTERS-1)
    return RC_NODE_FULL;
  int eid = -1;
  //if(locate(key, eid) == 0 && get_element(eid).key == key)
  {
    //cout << "DUPLICATE DETECTED" << endl;
    //exit(1);
    //return -1;
  }

  // Set element to insert
  element.pid = pid;
  element.key = key;
  set_element(element, num_elements);

  // Bubble Sort 
  for(int i = num_elements; i > 0; i--)
  {
    // Get elements from buffer
    left_element = get_element(i-1);
    right_element = get_element(i);

    if(right_element.key < left_element.key)
    {
      // Swap elements
      temp.key = right_element.key;
      temp.pid = right_element.pid;
      right_element.key = left_element.key;
      right_element.pid = left_element.pid;
      left_element.key = temp.key;
      left_element.pid = temp.pid;

      // Write swap to buffer
      set_element(left_element, i-1);
      set_element(right_element, i);
    }
    else
    {
      break;
    }

  }
  // Iterate size
  num_elements++;
  memcpy(buffer, &num_elements, sizeof(int));

  return 0;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ 
  int num_elements = getKeyCount();
  int element_size = sizeof(struct NonLeafNodeElement);
  int num_overflow = num_elements + 1;
  int half = (num_overflow)/2;
  
  NonLeafNodeElement tmp;

  // Check for negative parameters
  if (key < 0)
    return RC_INVALID_ATTRIBUTE;
  if (pid < 0)
    return RC_INVALID_PID;
  // Check if sibling is empty
  if (sibling.getKeyCount() != 0)
    return RC_INVALID_ATTRIBUTE;
  //int eid = -1;
  //if(locate(key, eid) == 0 && get_element(eid).key == key)
  //{
    //cout << "DUPLICATE DETECTED" << endl;
    //exit(1);
    //return -1;
  //}

  // Hold all overflow elements in temp array
  NonLeafNodeElement *overflow = new NonLeafNodeElement[num_overflow];
  for (int i = 0; i < num_elements; i++)
  {
    overflow[i].key = get_element(i).key;
    overflow[i].pid = get_element(i).pid;
  }
  overflow[num_elements].key = key;
  overflow[num_elements].pid = pid;

  // Backwards Bubble Sort
  for (int i = num_elements; i > 0; i--)
  {
    if(overflow[i].key < overflow[i-1].key)
    {
      // Swap Elements
      tmp.key = overflow[i].key;
      tmp.pid = overflow[i].pid;
      overflow[i].key = overflow[i-1].key;
      overflow[i].pid = overflow[i-1].pid;
      overflow[i-1].key = tmp.key;
      overflow[i-1].pid = tmp.pid;
    }
    else
    {
      break;
    }
  }

  // Clear current node
  // Set number of elemnts in node to zero
  num_elements = 0;
  memcpy(buffer, &num_elements, sizeof(int));//element_size);

  // Insert into current node
  for (int i = 0; i < half; i++)
  {
    insert(overflow[i].key, overflow[i].pid);
  }

  midKey = overflow[half].key;

  // Initalize sibling 
  sibling.initializeRoot(overflow[half].pid, overflow[half+1].key, overflow[half+1].pid);

  // Insert into sibling node
  for (int i = half+2; i < num_overflow; i++)
  {
    sibling.insert(overflow[i].key, overflow[i].pid);
  }
  free(overflow);

  return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ 
  //assume buffer is sorted
  int num_elements = getKeyCount();
  int pid_to_return = 0;

  // Check for negative parameters
  if (searchKey < 0)
    return RC_INVALID_ATTRIBUTE;
  if (num_elements <= 0)
    return RC_INVALID_ATTRIBUTE;

  if (searchKey < (get_element(0).key))
  {
    memcpy(&pid_to_return, buffer+4, sizeof(int));
    pid = pid_to_return;

    return 0;
  }

  for(int i = 1; i < num_elements; i++)
  {
    if(searchKey < (get_element(i).key))
    {
      pid = (get_element(i-1).pid);

      return 0;
    }
  }
  if(searchKey >= get_element(num_elements-1).key)
  {
    pid = get_element(num_elements-1).pid;

    return 0;
  }

  return RC_NO_SUCH_RECORD;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
  // Pointer to pageid greater than greatest key
  // Always set to eid = 1
  memcpy(buffer+4, &pid1, sizeof(int));
  // Rest of root
  insert(key, pid2);
  return 0;
}

NonLeafNodeElement BTNonLeafNode::get_element(int eid)
{
  NonLeafNodeElement nlfe;
  int element_size = sizeof(struct NonLeafNodeElement);
  memcpy(&nlfe,buffer+8+(eid*element_size),element_size);
  return nlfe;

}

void BTNonLeafNode::set_element(NonLeafNodeElement nlfe, int eid)
{
  int element_size = sizeof(struct NonLeafNodeElement);
  memcpy(buffer+8+(eid*element_size),&nlfe,element_size);
}

RC BTNonLeafNode::get_rightmost_child_ptr(PageId& pid)
{
  memcpy(&pid, buffer+4, sizeof(int));
}
