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
#include "BTreeIndex.h"

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
  
  RecordId rid;
  rid.pid = 14;
  rid.sid = 15;
  PageFile pf;
  pf.open("temp.tbl", 'w');

  PageId pid = 0;
  BTLeafNode leaf;  

  char * buffer;
  buffer = (char *) malloc(1024);

  int i =0;
  memcpy(buffer, &i, sizeof(int));

  pf.write(0, buffer); 
  leaf.read(pid, pf);

  assert(leaf.insert(1,rid) == 0);
  assert(leaf.insert(5,rid) == 0);
  assert(leaf.insert(3,rid) == 0);
  assert(leaf.insert(9,rid) == 0);
  assert(leaf.insert(2,rid) == 0);
  assert(leaf.insert(6,rid) == 1);
  assert(leaf.insert(7,rid) == 1);
  assert(leaf.insert(8,rid) == 1);
  //leaf.print_buffer();

  leaf.write(pid, pf);

  BTLeafNode leaf2;
  assert(leaf2.insert(-1,rid) == 1);
  char * buffer2 = (char *) malloc(1024);

  leaf2.read(pid,pf);
  assert(leaf2.insert(5,rid) == 1);
  i =0;
  memcpy(buffer2, &i, sizeof(int));

  pf.write(0, buffer2); 
  leaf2.read(pid, pf);
  assert(leaf2.insert(3,rid) == 0);
  assert(leaf2.insert(4,rid) == 0);
  assert(leaf2.insert(1,rid) == 0);
  assert(leaf2.insert(2,rid) == 0);
  assert(leaf2.insert(6,rid) == 0);

  //leaf2.print_buffer();
  assert(leaf2.insert(0,rid) == 1);
  assert(leaf2.insert(1,rid) == 1);
  assert(leaf2.insert(2,rid) == 1);
  assert(leaf2.insert(3,rid) == 1);
  assert(leaf2.insert(4,rid) == 1);
  assert(leaf2.insert(5,rid) == 1);
  cout << "Passed all test cases for BTLeafNode insert!" << endl;

  BTLeafNode leaf3;
  char * buffer3 = (char *) malloc(1024);
  i = 0;
  memcpy(buffer2, &i, sizeof(int));
  pf.write(0, buffer3); 
  leaf3.read(pid,pf);
  int eid = -1;
  assert(leaf3.locate(1,eid) == 1);
  assert(eid == -1);
  assert(leaf3.insert(1,rid) == 0);
  //leaf3.print_buffer();
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
  cout << "Passed all test cases for BTLeafNode locate!" << endl;

  BTLeafNode a;
  a.read(pid,pf);
  
  assert(a.insert(1,rid) == 0);
  assert(a.insert(2,rid) == 0);
  assert(a.insert(3,rid) == 0);
  assert(a.insert(4,rid) == 0);
  assert(a.insert(5,rid) == 0);
  assert(a.insert(6,rid) == 1);
  assert(a.insert(7,rid) == 1);
  BTLeafNode siblinga;
  siblinga.read(pid,pf);
  int sibling_key = -1;
  assert(a.insertAndSplit(-1,rid, siblinga, sibling_key) == 1);
  assert(a.insertAndSplit(6,rid, siblinga, sibling_key) == 0);
  assert(sibling_key == 4);
  assert(a.insertAndSplit(7,rid, siblinga, sibling_key) == 1);
  cout << "Passed all test cases for BTLeafNode insertAndSplit!" << endl;

  
  //Testing NonLeafNode
  PageId pid2 = 0;
  PageFile pf2;
  pf2.open("temp2.tbl", 'w');
  BTNonLeafNode nonleaf;
  char * nonleafbuffer;
  nonleafbuffer = (char*) malloc(1024);

  int i2 = 0;
  memcpy(nonleafbuffer, &i2, sizeof(int));
  pf2.write(0, nonleafbuffer);
  nonleaf.read(pid2, pf2);

  //cout<<nonleaf.insert(1,pid2)<<"is the value returned by insert(1, pid2)\n";
  assert(nonleaf.insert(1,pid2) == 0);
  assert(nonleaf.insert(5,pid2) == 0);
  assert(nonleaf.insert(3,pid2) == 0);
  assert(nonleaf.insert(9,pid2) == 0);
  assert(nonleaf.insert(2,pid2) == 0);
  nonleaf.print_buffer();
  assert(nonleaf.insert(6,pid2) == 1);
  assert(nonleaf.insert(7,pid2) == 1);
  assert(nonleaf.insert(8,pid2) == 1);
  nonleaf.write(pid2, pf2);

/*
  PageId pid3 = 0;
  PageFile pf3;
  pf3.open("temp3.tbl", 'w');

  BTNonLeafNode nonleaf2;
  char* nonleafbuffer2;
  nonleafbuffer2 = (char*) malloc(1024);

  int i3 = 0;
  memcpy(nonleafbuffer, &i3, sizeof(int));
  pf3.write(0, nonleafbuffer2);
  nonleaf2.read(pid3, pf3);

  assert(nonleaf2.insert(-1, pid3)==1);
 // assert(nonleaf2.insert(5, pid3)==1);

*/
  PageId pid3 = 0;
  PageFile pf3;
  pf3.open("temp3.tbl", 'w');

  BTNonLeafNode nonleaf2;
  nonleaf2.read(pid3,pf3);
  assert(nonleaf2.insert(-1,pid2) == 1);
  char * nonleafbuffer2 = (char *) malloc(1024);

  nonleaf2.read(pid2, pf2);
  assert(nonleaf2.insert(5,pid2) == 1);
  i =0;
  memcpy(nonleafbuffer2, &i, sizeof(int));

  pf2.write(0, nonleafbuffer2); 
  nonleaf2.read(pid2, pf2);
  assert(nonleaf2.insert(3,pid2) == 0);
  assert(nonleaf2.insert(4,pid2) == 0);
  assert(nonleaf2.insert(1,pid2) == 0);
  assert(nonleaf2.insert(2,pid2) == 0);
  assert(nonleaf2.insert(6,pid2) == 0);

  //leaf2.print_buffer();
  assert(nonleaf2.insert(0,pid2) == 1);
  assert(nonleaf2.insert(1,pid2) == 1);
  assert(nonleaf2.insert(2,pid2) == 1);
  assert(nonleaf2.insert(3,pid2) == 1);
  assert(nonleaf2.insert(4,pid2) == 1);
  assert(nonleaf2.insert(5,pid2) == 1);
  cout << "Passed all test cases for BTNonLeafNode insert!" << endl;

  BTNonLeafNode nonleaf3;
  char * nonleafbuffer3 = (char *) malloc(1024);
  int rightmost_child_ptr = 100;
  memcpy(nonleafbuffer3+4, &rightmost_child_ptr, sizeof(int));
  i = 0;
  //enter size in buffer
  memcpy(nonleafbuffer3, &i, sizeof(int));
  //write to pagefile p2
  pf2.write(0, nonleafbuffer3); 
  //read pid2 into nonleaf3
  nonleaf3.read(pid2,pf2);


  int pageid = -1;
  //check if locate errors if there are no children and nonleaf is empty
  assert(nonleaf3.locateChildPtr(1, pageid)==1);
  assert(pageid==-1);

  //checking if can locate rightmost child ptr
  assert(nonleaf3.insert(1, 5)==0);
  assert(nonleaf3.locateChildPtr(1, pageid)==0);
 // cout<<"Value of pageid: "<<pageid<<endl;
  assert(pageid==100);

  //check for leftmostpointer
  assert(nonleaf3.insert(3, 7)==0);
  assert(nonleaf3.locateChildPtr(0, pageid)==0);
  assert(pageid==5);

  //check for in between 1 and 3
  assert(nonleaf3.locateChildPtr(2, pageid)==0);
  assert(pageid==7);
  assert(nonleaf3.locateChildPtr(3, pageid)==0);
  assert(pageid==100);
  assert(nonleaf3.locateChildPtr(5, pageid)==0);
  assert(pageid==100);

  //check for 
  assert(nonleaf3.insert(7, 9)==0);
  assert(nonleaf3.locateChildPtr(5, pageid)==0);
  assert(pageid==9);
  assert(nonleaf3.locateChildPtr(10, pageid)==0);
  assert(pageid==100);
  assert(nonleaf3.locateChildPtr(-1, pageid)==1);
  assert(pageid==100);
  cout<<"Passed all test cases for BTNonLeafNode locateChildPtr!"<<endl;


  BTNonLeafNode nonleaf4;
  char * nonleafbuffer4 = (char *) malloc(1024);
  rightmost_child_ptr = 100;
  memcpy(nonleafbuffer4+4, &rightmost_child_ptr, sizeof(int));
  i = 0;
  //enter size in buffer
  memcpy(nonleafbuffer4, &i, sizeof(int));
  //write to pagefile p2
  pf2.write(0, nonleafbuffer4); 
  //read pid2 into nonleaf3
  nonleaf4.read(pid2,pf2);
  assert(nonleaf4.insert(1, 1)==0);
  assert(nonleaf4.insert(2,2)==0);
  assert(nonleaf4.insert(3,3)==0);
  assert(nonleaf4.insert(4,4)==0);
  assert(nonleaf4.insert(5,5)==0);
  assert(nonleaf4.insert(6,6)==1);
  assert(nonleaf4.insert(7,7)==1);
  assert(nonleaf4.insert(8,8)==1);

  BTNonLeafNode siblingn;
  siblingn.read(pid2,pf2);
  int midKey = -1;
  assert(nonleaf4.insertAndSplit(-1, 6,siblingn, midKey)==1);
  assert(midKey==-1);
  assert(nonleaf4.insertAndSplit(6, 6, siblingn, midKey)==0);
 // cout<<"Value of midKey is: "<<midKey<<endl;
  //cout<<"Number of elements in sibling 1: "<<nonleaf4.getKeyCount()<<endl;
  //cout<<"Number of elements in sibling 2: "<<siblingn.getKeyCount()<<endl;
  cout<<"Passed all test cases for BTNonLeafNode insertAndSplit!\n";



/*


  pageid = -1;
  assert(nonleaf3.locateChildPtr(1,pageid) == 1);
  assert(pageid == -1);
  assert(nonleaf3.insert(1,pid2) == 0);
  //leaf3.print_buffer();
  assert(nonleaf3.locateChildPtr(1,pageid) == 0);
  assert(pageid == 0);
  assert(nonleaf3.insert(3,pid2) == 0);
  assert(nonleaf3.locateChildPtr(3,pageid) == 0);
  assert(pageid == 1);
  assert(nonleaf3.locateChildPtr(2,pageid) == 0);
  assert(pageid == 1);
  assert(nonleaf3.locateChildPtr(0,pageid) == 0);
  assert(pageid == 0);
  assert(nonleaf3.locateChildPtr(-1,pageid) == 1);
  assert(nonleaf3.insert(5,pid2) == 0);
  assert(nonleaf3.insert(9,pid2) == 0);

  assert(nonleaf3.insert(12,pid2) == 0);
  assert(nonleaf3.insert(15,pid2) == 1);
  assert(nonleaf3.locateChildPtr(4,pageid) == 0);
  assert(pageid == 2);
  assert(nonleaf3.locateChildPtr(7,pageid) == 0);
  assert(pageid == 3);
  assert(nonleaf3.locateChildPtr(13,pageid) == 1);
  assert(pageid == 3);
  assert(nonleaf3.locateChildPtr(12,pageid) == 0);
  assert(pageid == 4);
  pageid = -1;
  assert(nonleaf3.locateChildPtr(11,pageid) == 0);
  assert(pageid == 4);
  cout << "Passed all test cases for BTNonLeafNode locateChildPtr!" << endl;
*/
  // Test cases for BTreeIndex Insert
  BTreeIndex tree1;
  RecordId t_rid;
  t_rid.pid = 1;
  t_rid.sid = 2;
  tree1.open("pf.test", 'w');

  //tree1.init();
  tree1.print_height();
  assert(tree1.insert(1, t_rid) == 0);
  assert(tree1.insert(2, t_rid) == 0);
  assert(tree1.insert(3, t_rid) == 0);
  assert(tree1.insert(4, t_rid) == 0);
  assert(tree1.insert(5, t_rid) == 0);
  tree1.print_height();
  assert(tree1.insert(6, t_rid) == 0);
  assert(tree1.insert(7, t_rid) == 0);
  assert(tree1.insert(8, t_rid) == 0);
  assert(tree1.insert(9, t_rid) == 0);
  assert(tree1.insert(10, t_rid) == 0);
  tree1.print_height();
  assert(tree1.insert(11, t_rid) == 0);
  assert(tree1.insert(12, t_rid) == 0);
  assert(tree1.insert(13, t_rid) == 0);
  assert(tree1.insert(14, t_rid) == 0);
  assert(tree1.insert(15, t_rid) == 0);
  tree1.print_height();
  assert(tree1.insert(16, t_rid) == 0);
  assert(tree1.insert(17, t_rid) == 0);
  assert(tree1.insert(18, t_rid) == 0);
  assert(tree1.insert(19, t_rid) == 0);
  assert(tree1.insert(20, t_rid) == 0);
  BTNonLeafNode nonLeafNode; 
  PageFile pf4;
  pf4.open("pf.test", 'r');
  nonLeafNode.read(1, pf4);
  nonLeafNode.print_buffer();
  tree1.print_height();
 //tree1.printTree();

  tree1.close();

  cout << "Passed all test casses for BTreeIndex insert!" << endl;

  return 0;
}
