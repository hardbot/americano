#include "BTreeNode.h"
#include <iostream>

using namespace std;

/* testing git again
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	RC rc = pf.read(pid, buffer);
	return rc;
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
  RC rc = pf.write(pid, buffer);
  return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
  return element_size;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */

RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
  // Check for negative keys
  if (key < 0)
    return 1;
  // Check if node is full
  if (element_size == MAX_NUM_POINTERS-1)
    return 1;
  int eid = -1;
  // Check for duplicates;
  locate(key,eid);
  if (eid == element_array[eid].get_key())
    return 1;

  LeafNodeElement tmp;

  // Insert to end of leaf node
  element_array[element_size].set_key(key);
  element_array[element_size].set_rec_id(rid);

  // Bubble Sort backwards
  for (int i = element_size; i > 0; i--)
  {
    if(element_array[i].get_key() < element_array[i-1].get_key())
    {
      // Swap
      tmp.set_key(element_array[i].get_key());
      tmp.set_rec_id(element_array[i].get_rec_id());
      element_array[i].set_key(element_array[i-1].get_key());
      element_array[i].set_rec_id(element_array[i-1].get_rec_id());
      element_array[i-1].set_key(tmp.get_key());
      element_array[i-1].set_rec_id(tmp.get_rec_id());
    }
    else
    {
      break;
    }
  }

  element_size += 1;
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
  if (key < 0)
    return 1;
  if (!sibling.is_empty())
    return 1;
  int eid = -1;
  locate(key, eid);
  if (key == element_array[eid].get_key())
    return 1;

  // Hold all overflowed elements in tmp array
  int overflow_size = element_size+1;
  LeafNodeElement *overflow = new LeafNodeElement[overflow_size];
  for (int i = 0; i < element_size; i++)
  {
    overflow[i] = element_array[i];
  }
  overflow[element_size].set_key(key);
  overflow[element_size].set_rec_id(rid);

  // Bubble Sort backwards
  LeafNodeElement tmp;
  for (int i = overflow_size; i > 0; i--)
  {
    if(overflow[i].get_key() < overflow[i-1].get_key())
    {
      // Swap
      tmp.set_key(overflow[i].get_key());
      tmp.set_rec_id(overflow[i].get_rec_id());
      overflow[i].set_key(overflow[i-1].get_key());
      overflow[i].set_rec_id(overflow[i-1].get_rec_id());
      overflow[i-1].set_key(tmp.get_key());
      overflow[i-1].set_rec_id(tmp.get_rec_id());
    }
    else
    {
      break;
    }
  }

  // Clear current node
  for (int i = 0; i < element_size; i++)
  {
    element_array[i].clear();
  }

  // Insert into current node
  int half = (overflow_size)/2;
  element_size = 0;
  for (int i = 0; i < half; i++)
  {
    insert(overflow[i].get_key(), overflow[i].get_rec_id());
  }

  // Insert into sibling node
  for (int i = half; i < overflow_size; i++)
  {
    sibling.insert(overflow[i].get_key(), overflow[i].get_rec_id());
  }
  siblingKey = overflow[half].get_key();

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
  if (searchKey < 0)
    return 1;
  // Binary search approach to be implemented later when
  // more than 2 keys can be stored
  if (element_size <= 0)
    return 1;
  if (searchKey > element_array[element_size-1].get_key())
    return 1;

  int low = 0;
  int mid = 0;
  int high = element_size-1;

  while (low <= high)
  {

    mid = (low + high)/2;
    if (searchKey < element_array[mid].get_key())
    {
      high = mid - 1;
      if ( low > high )
      {
        eid = mid;
        return 0;
      }
    }
    else if (searchKey > element_array[mid].get_key())
    {
      low = mid + 1;
      if ( low > high )
      {
        eid = mid+1;
        return 0;
      }
    }
    else
    {
      eid = mid;
      return 0;
    }
  }
  return 1; 
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
  if(eid>element_size) return 1;

  key = element_array[eid].get_key();
  rid.sid = element_array[eid].get_rec_id().sid;
  rid.pid = element_array[eid].get_rec_id().pid;
  return 0;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
  return next;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
  //if(pid == NULL) return 1;
  if(pid<0) return 1;
  next = pid;
  return 0;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ return 0; }

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ return 0; }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ return 0; }

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
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; }

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; }
