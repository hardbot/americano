/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeNode.h"

#include <assert.h>
#include <iostream>
using namespace std;

int main()
{
  // run the SQL engine taking user commands from standard input (console).
  //SqlEngine::run(stdin);

  /* Test cases for n = 3
  // Test file
  BTLeafNode leaf;  
  RecordId rid;
  rid.pid = 5;
  rid.sid = 10;
  //leaf.print_leaf_node();
  // first insert
  assert(leaf.insert(1,rid) == 0);
  //leaf.print_leaf_node();
  rid.pid = 6;
  rid.sid = 11;

  // second insert
  assert(leaf.insert(2,rid) == 0);
  //leaf.print_leaf_node();
  rid.pid = 7;
  rid.sid = 12;

  // third insert
  assert(leaf.insert(0,rid) == 1);
  assert(leaf.insert(1,rid) == 1);
  assert(leaf.insert(2,rid) == 1);
  assert(leaf.insert(3,rid) == 1);
  assert(leaf.insert(4,rid) == 1);
  assert(leaf.insert(5,rid) == 1);
  //leaf.print_leaf_node();
  
  //Test 2
  BTLeafNode leaf2;
  rid.pid = 8;
  rid.sid = 12;
  //leaf2.print_leaf_node();
  // first insert
  assert(leaf2.insert(5,rid) == 0);
  //leaf2.print_leaf_node();
  rid.pid = 9;
  rid.sid = 13;
  // second insert with smaller key
  assert(leaf2.insert(4,rid) == 0);
  //leaf2.print_leaf_node();
  
  // third insert
  assert(leaf2.insert(0,rid) == 1);
  assert(leaf2.insert(1,rid) == 1);
  assert(leaf2.insert(2,rid) == 1);
  assert(leaf2.insert(3,rid) == 1);
  assert(leaf2.insert(4,rid) == 1);
  assert(leaf2.insert(5,rid) == 1);
  assert(leaf2.insert(6,rid) == 1);
  assert(leaf2.insert(7,rid) == 1);
  cout << "Passed all test cases for insert!\n" << endl;

  // Test 3
  BTLeafNode leaf3;
  rid.pid = 1;
  rid.sid = 2;
  int eid = -1;
  // Locate in empty node
  assert(leaf3.locate(1, eid) == 1);
  assert(leaf3.insert(1,rid) == 0);
  assert(leaf3.locate(1, eid) == 0);
  assert(leaf3.locate(2, eid) == 1);
  assert(eid == 1);
  // Locate something that has not been inserted
  assert(leaf3.insert(2,rid) == 0);
  assert(leaf3.locate(2, eid) == 0);
  assert(leaf3.insert(3,rid) == 1);
  assert(leaf3.locate(3, eid) == 1);
  assert(leaf3.locate(2, eid) == 0);
  assert(eid == 2);

  BTLeafNode leaf31;
  assert(leaf31.insert(5,rid) == 0);
  assert(leaf31.locate(3,eid) == 0);
  assert(leaf31.insert(9,rid) == 0);
  assert(leaf31.locate(3,eid) == 0);
  assert(eid == 5);
  assert(leaf31.locate(8,eid) == 0);
  assert(eid == 9);
  eid = 1;
  assert(leaf31.locate(6,eid) == 0);
  cout << eid << endl;
  assert(eid == 9);
  assert(leaf31.locate(9,eid) == 0);
  assert(eid == 9);
  assert(leaf31.locate(10,eid) == 1);
  cout << "Passed all test cases for locate!\n" << endl;
  */

  /* Test Cases for n = 6 */
  BTLeafNode leaf;  
  RecordId rid;
  rid.pid = 14;
  rid.sid = 15;
  assert(leaf.insert(1,rid) == 0);
  assert(leaf.insert(2,rid) == 0);
  assert(leaf.insert(3,rid) == 0);
  assert(leaf.insert(4,rid) == 0);
  assert(leaf.insert(5,rid) == 0);
  assert(leaf.insert(6,rid) == 1);
  assert(leaf.insert(7,rid) == 1);
  assert(leaf.insert(8,rid) == 1);

  BTLeafNode leaf2;
  assert(leaf2.insert(-1,rid) == 1);

  assert(leaf2.insert(5,rid) == 0);
  assert(leaf2.insert(3,rid) == 0);
  assert(leaf2.insert(4,rid) == 0);
  assert(leaf2.insert(1,rid) == 0);
  assert(leaf2.insert(2,rid) == 0);
  assert(leaf2.insert(6,rid) == 1);

  assert(leaf2.insert(0,rid) == 1);
  assert(leaf2.insert(1,rid) == 1);
  assert(leaf2.insert(2,rid) == 1);
  assert(leaf2.insert(3,rid) == 1);
  assert(leaf2.insert(4,rid) == 1);
  assert(leaf2.insert(5,rid) == 1);
  cout << "Passed all test cases for insert!" << endl;

  BTLeafNode leaf3;
  int eid = -1;
  assert(leaf3.locate(1,eid) == 1);
  assert(eid == -1);
  assert(leaf3.insert(1,rid) == 0);
  assert(leaf3.locate(1,eid) == 0);
  assert(eid == 0);
  assert(leaf3.insert(3,rid) == 0);
  assert(leaf3.locate(3,eid) == 0);
  assert(eid == 1);
  assert(leaf3.locate(2,eid) == 0);
  assert(eid == 1);
  assert(leaf3.locate(0,eid) == 0);
  assert(eid == 0);
  assert(leaf3.locate(-1,eid) == 1);
  assert(leaf3.insert(5,rid) == 0);
  assert(leaf3.insert(9,rid) == 0);
  assert(leaf3.insert(12,rid) == 0);
  assert(leaf3.insert(15,rid) == 1);
  assert(leaf3.locate(4,eid) == 0);
  assert(eid == 2);
  assert(leaf3.locate(7,eid) == 0);
  assert(eid == 3);
  assert(leaf3.locate(13,eid) == 1);
  assert(eid == 3);
  assert(leaf3.locate(12,eid) == 0);
  assert(eid == 4);
  eid = -1;
  assert(leaf3.locate(11,eid) == 0);
  assert(eid == 4);
  cout << "Passed all test cases for locate!" << endl;

  BTLeafNode a;
  assert(a.insert(1,rid) == 0);
  assert(a.insert(2,rid) == 0);
  assert(a.insert(3,rid) == 0);
  assert(a.insert(4,rid) == 0);
  assert(a.insert(5,rid) == 0);
  assert(a.insert(6,rid) == 1);
  assert(a.insert(7,rid) == 1);
  BTLeafNode siblinga;
  int sibling_key = -1;
  assert(a.insertAndSplit(-1,rid, siblinga, sibling_key) == 1);
  assert(a.insertAndSplit(6,rid, siblinga, sibling_key) == 0);
  assert(sibling_key == 3);
  assert(a.insertAndSplit(7,rid, siblinga, sibling_key) == 1);
  cout << "Passed all test cases for insertAndSplit!" << endl;
  
  return 0;
}
