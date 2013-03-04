/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include <iostream>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    RC ret = pf.open(indexname, mode);
    if (mode == 'w')
      init();
    return ret;
}

/*
RC BTreeIndex::printTree(int cur_height, int pid)
{
  if (cur_height == treeHeight)
  {
    getLeaf(pid).print_buffer();
  }
  else
  {
  }
}
*/

RC BTreeIndex::init()
{
  char buffer[1024];
  TreeMeta tm;
  tm.root = rootPid = 1;
  tm.height = treeHeight = 1;
  memcpy(buffer, &tm, sizeof(struct TreeMeta));
  pf.write(0, buffer);
}

RC BTreeIndex::setMeta()
{
  char buffer[1024];
  TreeMeta tm;
  tm.root = rootPid;
  tm.height = treeHeight;
  memcpy(buffer, &tm, sizeof(struct TreeMeta));
  pf.write(0, buffer);
}

RC BTreeIndex::getLeaf(PageId pid, BTLeafNode &lf)
{
    char buffer[1024];
    pf.read(pid, buffer);
    memcpy(&lf, buffer, sizeof(struct BTLeafNode));
    return 0;
}

RC BTreeIndex::getNonLeaf(PageId pid, BTNonLeafNode &nlf)
{
    char buffer[1024];
    pf.read(pid, buffer);
    memcpy(&nlf, buffer, sizeof(struct BTNonLeafNode));
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    return pf.close();
}

// insert rec error codes:
// 1 = propogate
// 0 = success
// < 0 = error
RC BTreeIndex::insert_rec(int cur_height, PageId pid, int key, const RecordId& rid, PageId &sibling_pid, int &sibling_key)
{
  int child_pid, propagate = -1; 
  int end_pid = pf.endPid();
  BTLeafNode leaf, sibling;
  BTNonLeafNode non_leaf, non_leaf_sibling;

  // At level above tree
  if (cur_height == 0)
  {
    // Recurse
    propagate = insert_rec(cur_height+1, pid, key, rid, sibling_pid, sibling_key);

    // Propogate key up to root
    if (propagate == 1)
    {
      // Set new root and meta
      rootPid = pf.endPid();
      treeHeight++;
      setMeta();

      // Initialize new root
      getNonLeaf(rootPid, non_leaf);
      non_leaf.initializeRoot(pid, sibling_key, sibling_pid);
      non_leaf.write(rootPid, pf);
    }
  }
  // End case if reached leaf
  else if (cur_height == treeHeight)
  {
    // Get leaf at pid
    getLeaf(pid, leaf);

    cout << "Hello" << endl;
    // Split keys if overflow 
    if ( leaf.insert(key, rid) != 0)
    {
      cout << "World" << endl;
      // Split up keys
      // sibling_key set to first key of sibling
      sibling.read(end_pid, pf);
      leaf.insertAndSplit(key, rid, sibling, sibling_key);
      // Write update leaf
      leaf.write(pid, pf);
      // Write new sibling
      sibling_pid = end_pid;
      sibling.write(end_pid, pf);
      leaf.print_buffer();
      sibling.print_buffer();

      return 1;
    }

    // Write leaf if no overflow
    leaf.write(pid, pf);
    return 0;
  }
  // Look through nonleaf nodes for key
  else
  {
    // Get non leaf at pid
    getNonLeaf(pid, non_leaf);

    // Get pid to travel to in child_pid
    non_leaf.locateChildPtr(key, child_pid);

    // Recurse to next height with child_pid
    propagate = insert_rec(cur_height+1, child_pid, key, rid, sibling_pid, sibling_key);
    // Propagate sibling keys up
    if (propagate == 1)
    {
      // Check if keys are valid
      //if (sibling_key == -1 || sibling_pid == -1)
        //return -1;
      
      // At some non leaf node
      if (non_leaf.insert(sibling_key, sibling_pid) != 0)
      {
        // Split up keys
        // Sibling_key set to mid key from non_leaf
        non_leaf.insertAndSplit(key, pid, non_leaf_sibling, sibling_key);
        // Write updated non leaf
        non_leaf.write(pid, pf);
        // Write new non leaf sibling
        sibling_pid = pf.endPid();
        non_leaf_sibling.write(pf.endPid(),pf);

        return 1;
      }
      // Write non leaf if no overflow
      non_leaf.write(sibling_pid, pf);
      return 0;
    }
    else
    {
      return 0;
    }
  }

  return 0;
}

void BTreeIndex::print_height()
{
  cout << "Tree Height: " << treeHeight << endl;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    // Assume Tree is initialized
    PageId pid = -1;
    int sibling_key = -1;
    return insert_rec(0, rootPid, key, rid, pid, sibling_key);
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
  /*
    // Initalize
    BTLeafNode lf;
    memcpy(lf,&pf+8+cursor.pid, ));

    // See if eid is in current leaf node
    if (lf.getKeyCount() < cursor.eid)
    {
      // Set to sibling
      cursor.pid = lf.getNextNodePtr();
      cursor.eid = 0;

      memcpy(lf,&pf+8+cursor.pid,sizeof(BTLeafNode));
    }
    key = (lf.get_element(cursor.eid)).key;
    rid = (lf.get_element(cursor.eid)).rec_id;
    return 0;
    */
}
  RC BTreeIndex::printTree()
  {
    /*
    char buffer[1024];
    RC ret = pf.read(0, buffer);
    TreeMeta tm;
    memcpy(&tm, buffer, sizeof(struct TreeMeta));

  */
    if(treeHeight >=1)
    {
      //call printTreeRecursive on the root node
      printTreeRecursive(1, 1);
      return 0;
    }
    else
    {
      cout<<"Unable to print when treeHeight < 1!"<<endl;
      return 1;
    }

  }

  RC BTreeIndex::printTreeRecursive(int pid, int cur_height)
  {
    char buffer[1024];
    RC ret;

    //base case
    if(cur_height == treeHeight)
    {
      BTLeafNode leafNode;
      ret = leafNode.read(pid, pf);
      leafNode.print_buffer();
      return ret;
    }

    //not at maxHeight level => must be a nonLeafNode

    /*
    //print current
    BTNonLeafNode nonLeafNode;
    ret = nonLeafNode.read(pid, pf);
    if(ret != 0)
    {
      cout<<"Unable to load NonLeafNode with pid "<<pid<<endl;
    }
    nonLeafNode.print_buffer();

    NonLeafNodeElement nextNonLeafNode;
    for(int i = 0; i < nonLeafNode.getKeyCount(); i++)
    {
      nextNonLeafNode = nonLeafNode.get_element(i);
      printTreeRecursive(nextNonLeafNode.pid, cur_height+1);
    }
    PageId rightmost_pid = -1;
    nonLeafNode.get_rightmost_child_ptr(rightmost_pid);
    if(rightmost_pid>0)
    {
      printTreeRecursive(rightmost_pid, cur_height+1);
    }

    cout<<"Printed out complete tree"<<endl;
    */

  }
