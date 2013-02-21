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

  
  return 0;
}
