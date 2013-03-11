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
 * Used to initiate the root and height of tree
 */
RC BTreeIndex::init()
{
  char buffer[1024];
  TreeMeta tm;
  tm.root = rootPid = 1;
  tm.height = treeHeight = 1;
  memcpy(buffer, &tm, sizeof(struct TreeMeta));
  return pf.write(0, buffer);
}

/*
 * Sets the metadata (root and height) for tree in pid 0
 */
RC BTreeIndex::setMeta()
{
  char buffer[1024];
  TreeMeta tm;
  tm.root = rootPid;
  tm.height = treeHeight;
  memcpy(buffer, &tm, sizeof(struct TreeMeta));
  return pf.write(0, buffer);
}

/*
 * Loads metadata from pid 0
 */
RC BTreeIndex::loadMeta()
{
  char buffer[1024];
  TreeMeta tm;
  RC ret;
  ret = pf.read(0, buffer);
  memcpy(&tm, buffer, sizeof(struct TreeMeta));
  rootPid = tm.root;
  treeHeight = tm.height;
  return ret;
}

/*
 * Reads leaf at pid, and outputs it to lf
 */
RC BTreeIndex::getLeaf(PageId pid, BTLeafNode &lf)
{
    char buffer[pf.PAGE_SIZE];
    RC ret;
    ret = pf.read(pid, buffer);
    memcpy(&lf, buffer, sizeof(struct BTLeafNode));
    return ret;
}

/*
 * Reads nonleaf at pid, and outputs it to nlf
 */
RC BTreeIndex::getNonLeaf(PageId pid, BTNonLeafNode &nlf)
{
    char buffer[pf.PAGE_SIZE];
    RC ret;
    ret = pf.read(pid, buffer);
    memcpy(&nlf, buffer, sizeof(struct BTNonLeafNode));
    return ret;
}
/*
 * Prints height of tree
 */
void BTreeIndex::print_height()
{
  cout << "Tree Height: " << treeHeight << endl;
}

/* 
 * Prints data stored in cursor
 */
void BTreeIndex::printIndex(IndexCursor ic)
{
  BTLeafNode leaf;
  int key;
  RecordId rid;

  cout << "Printing Index Cursor: " << endl;
  cout << "Index pid: " << ic.pid << endl;
  cout << "Index eid: " << ic.eid << endl;

  getLeaf(ic.pid, leaf);
  leaf.readEntry(ic.eid, key, rid);
  cout << "Key: " << key << endl;
  cout << "Pid: " << rid.pid << endl;
  cout << "Sid: " << rid.sid << endl;
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
    if(ret !=0) return ret;
    if (mode == 'w')
    {
      init();
    }
    else if(mode=='r')
    {
      loadMeta();
    }
    return ret;
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
RC BTreeIndex::insert_rec(int cur_height, PageId pid, int key, const RecordId& rid, PageId &sibling_pid, int &sibling_key, int &mid_key)
{
  int child_pid = -1, end_pid = -1, propagate = -1; 
  BTLeafNode leaf, sibling;
  BTNonLeafNode non_leaf, non_leaf_sibling;

  // At level above tree
  if (cur_height == 0)
  {
    // Recurse
    propagate = insert_rec(cur_height+1, pid, key, rid, sibling_pid, sibling_key, mid_key);

    // Propogate key up to root
    if (propagate == 1)
    {
      // Set new root and meta
      end_pid = pf.endPid();
      rootPid = end_pid;
      treeHeight++;
      setMeta();

      // Initialize new root
      if (treeHeight == 2)
        non_leaf.initializeRoot(pid, sibling_key, sibling_pid);
      else
        non_leaf.initializeRoot(pid, mid_key, sibling_pid);

      non_leaf.write(end_pid, pf);
    }
  }
  // End case if reached leaf
  else if (cur_height == treeHeight)
  {
    // Get leaf at pid
    if (pf.endPid() > 1)
      getLeaf(pid, leaf);

    // Split keys if overflow 
    RC err = leaf.insert(key, rid);
    if ( err == RC_NODE_FULL )
    {
      // Split up keys
      // sibling_key set to first key of sibling
      err = leaf.insertAndSplit(key, rid, sibling, sibling_key);
      if (err < 0)
        return err;
      end_pid = pf.endPid();

      // Write update leaf
      leaf.setNextNodePtr(end_pid);
      leaf.write(pid, pf);
      // Write new sibling
      sibling_pid = end_pid;
      sibling.write(end_pid, pf);

      return 1;
    }
    else if (err != 0 )
    {
      return err;
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
    RC err2 =  non_leaf.locateChildPtr(key, child_pid);
    // Recurse to next height with child_pid
    propagate = insert_rec(cur_height+1, child_pid, key, rid, sibling_pid, sibling_key, mid_key);

    // Propagate sibling keys up
    if (propagate == 1)
    {
      RC err;
      // At some non leaf node
      if (cur_height == treeHeight -1)
        err = non_leaf.insert(sibling_key, sibling_pid);
      else
        err = non_leaf.insert(mid_key, sibling_pid);

      if (err == RC_NODE_FULL)
      {
        // Split up keys
        // Sibling_key set to mid key from non_leaf
        if (cur_height == treeHeight -1)
          non_leaf.insertAndSplit(sibling_key, sibling_pid, non_leaf_sibling, mid_key);
        else 
        {
          int prev_mid_key = mid_key;
          non_leaf.insertAndSplit(prev_mid_key, sibling_pid, non_leaf_sibling, mid_key);
        }

        // Write updated non leaf
        non_leaf.write(pid, pf);

        // Write new non leaf sibling
        end_pid = pf.endPid();
        sibling_pid = end_pid;
        non_leaf_sibling.write(end_pid, pf);

        return 1;
      }
      else if (err != 0)
      {
        return err;
      }

      // Write non leaf if no overflow
      non_leaf.write(pid, pf);
    }

  }

  return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    // Error check
   if (key < 0)
     return RC_INVALID_ATTRIBUTE;
   if (rid.pid < 0 || rid.sid < 0)
     return RC_INVALID_RID;
    // Assume Tree is initialized
    PageId pid = -5;
    int sibling_key = -6;
    int mid_key = -1;
    return insert_rec(0, rootPid, key, rid, pid, sibling_key, mid_key);
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

RC BTreeIndex::locate_rec(int cur_height, int pid, int searchKey, IndexCursor& cursor)
{
    BTLeafNode leaf;
    BTNonLeafNode non_leaf;
    int eid = 0, child_pid = 0;
    if (cur_height == treeHeight)
    {
      //cout<<"GOT THIS FAR IN LOCATE!"<<endl;
      getLeaf(pid, leaf);
      RC ret = leaf.locate(searchKey, eid);
      cursor.pid = pid;
      cursor.eid = eid;
      return ret;
    }
    else
    {
      getNonLeaf(pid, non_leaf);
      RC ret = non_leaf.locateChildPtr(searchKey, child_pid);
      if (ret <0)
        return ret;

      locate_rec(cur_height+1, child_pid, searchKey, cursor);
    }
    return RC_END_OF_TREE;
}

RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    if (searchKey < 0)
      return RC_INVALID_ATTRIBUTE;
    return locate_rec(1, rootPid, searchKey, cursor);
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
    // Error check
    if(cursor.pid < 0 || cursor.eid < 0)
      return RC_INVALID_CURSOR;

    // Initalize
    BTLeafNode lf;
    getLeaf(cursor.pid, lf);

    // See if eid is in current leaf node
    if (lf.getKeyCount() <= cursor.eid)
    {
      // Set to sibling
      if (lf.getNextNodePtr() == -1)
        return RC_END_OF_TREE;
      cursor.pid = lf.getNextNodePtr();
      cursor.eid = 0;
      getLeaf(cursor.pid, lf);
    }

    // Set output values
    key = (lf.get_element(cursor.eid)).key;
    rid = (lf.get_element(cursor.eid)).rec_id;

    // Iterate eid to go to to next element
    cursor.eid += 1;

    return 0;
}
