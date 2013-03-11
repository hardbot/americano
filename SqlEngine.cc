/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}


void SqlEngine::printTuple(const int& attr, const int& key, const string& value)
{
    // print the tuple 
    switch (attr) {
    case 1:  // SELECT key
      fprintf(stdout, "%d\n", key);
      break;
    case 2:  // SELECT value
      fprintf(stdout, "%s\n", value.c_str());
      break;
    case 3:  // SELECT *
      fprintf(stdout, "%d '%s'\n", key, value.c_str());
      break;
    }
}

bool SqlEngine::isValidValue(const char * value, const char * cond_value, SelCond::Comparator comp_type)
{

        int index_diff;
        index_diff = strcmp(value, cond_value);

        // skip the tuple if any condition is not met
        switch (comp_type) {
          case SelCond::EQ:
           if (index_diff != 0) return false;
           break;
          case SelCond::NE:
           if (index_diff == 0) return false;
           break;
          case SelCond::GT:
           // cout<<"Got inside the switch inside isValidValue"<<endl;
           if (index_diff <= 0) return false;
           break;
          case SelCond::LT:
           if (index_diff >= 0) return false;
           break;
          case SelCond::GE:
           if (index_diff < 0) return false;
           break;
          case SelCond::LE:
           if (index_diff > 0) return false;
           break;
        }
        return true;
}


//The current implementation of select() performs a scan across the entire table to answer the query. 
//Your last task will be to modify this behavior of select() to meet the following requirements:
//If a SELECT query has one or more conditions on the key column and if the table has a B+tree, use the B+tree to help answer the query as follows:
//-->If there exists an equality condition on key, you should always use the equality condition in looking up the index.
//-->Queries which specify a range (like key >= 10 and key < 100) must use the index. 
//----SqlEngine should try to avoid retrieving the tuples outside of this range from the underlying table by using the index.
//-->You should not to use an inequality condition <> on key in looking up the B+tree.
//-->You should avoid unnecessary page reads of getting information about the "value" column(s); 
//----for example, if ONLY "key" column(s) exist in a SELECT statement, or if travesing leaf nodes on B+tree returns the count(*).
//-->As a rule of thumb, you should avoid unecessary page reads if queries can be answered throgh the information stored in the nodes of a B+ Tree.

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  BTreeIndex b_tree;
  //true when there is a key in the where clause
  bool key_in_where = false;
  bool value_in_where = false;

  //true when there is a '>' in the where clause
  bool greater_than_not_equal = false;
  //true when there is a '<' in the where clause
  bool less_than_not_equal = false;

  bool not_equal_found = false;

  bool one_equal_statement = false;
  bool more_than_one_equal_statement = false;

  //used for locate
  IndexCursor cursor;
  cursor.pid =-1;
  cursor.eid =0;

  int key_min=0;
  int key_max=-1;

  RC     rc;
  int    key;     
  string value;
  int    count=0;
  int    diff;
  int index_diff;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  //cout<<"Size of cond.size(): "<<cond.size()<<endl;
  //check for 'key' in WHERE clause
  for(unsigned i = 0; i < cond.size(); i++)
  {
    //if there is a condition in where clause has a key
    if(cond[i].attr==1)
    {
      key_in_where = true;

      //if equal
      if(cond[i].comp == SelCond::EQ)
      {
        if(one_equal_statement==true)
        {
          more_than_one_equal_statement = true;
        }
        key_min = key_max = atoi(cond[i].value);
        //cout<<"Value of key_min: "<<key_min<<endl;
        one_equal_statement= true;
      }
      //if greater than
      else if(cond[i].comp == SelCond::GT)
      {
        greater_than_not_equal = true;
        int new_key_min = atoi(cond[i].value);
        if(new_key_min>key_min)
        {
          key_min = new_key_min;
        }
       // cout<<"Key_min for GT clause is now: "<<key_min<<endl;
      }
      //if less than
      else if(cond[i].comp == SelCond::LT)
      {
        less_than_not_equal = true;
        int new_key_max = atoi(cond[i].value);
        if(new_key_max < key_max || key_max == -1)
        {
          key_max = new_key_max;
        }
        //cout<<"Key_max for LT clause is now: "<<key_max<<endl;
      }
      //if less than or equal to
      else if(cond[i].comp == SelCond::LE)
      {
        int new_key_max = atoi(cond[i].value);
        if(new_key_max < key_max || key_max == -1)
        {
          key_max = new_key_max;
        }
        //cout<<"Key_max for LE clause is now: "<<key_min<<endl;
      }
      //if greater than or equal to
      else if(cond[i].comp == SelCond::GE)
      {
        int new_key_min = atoi(cond[i].value);
        if(new_key_min >key_min)
        {
          key_min = new_key_min;
        }
        //cout<<"Key_min for GE clause is now: "<<key_min<<endl;
      }
    }
    else if(cond[i].attr==2)
    {
      value_in_where = true;
    }

    if(cond[i].comp==SelCond::NE)
    {
      not_equal_found = true;
    }

  }

  //open the B+ Tree
  rc = b_tree.open(table + ".idx", 'r');
  //cout<<"RC value from opening B+ Tree: "<<rc<<endl;

  //if index file exists and there is a condition on key
  if((rc==0) && (key_in_where == true||attr==4) && (not_equal_found==false) && ((key_min<=key_max)||key_max==-1) && (more_than_one_equal_statement==false))
  {
    //if looking for just one tuple
    if(key_min == key_max)
    {
    //  cout<<"Got this far!"<<endl;
      //place cursur on the tuple
      b_tree.locate(key_max, cursor);

      //cout<<"Got past locate!"<<endl;

      //read the tuple into key, rid
      b_tree.readForward(cursor, key, rid);
      //cout<<"Rid.sid: "<<rid.sid<<" Rid.pid: "<<rid.pid<<endl;


      //if SELECT value or SELECT *
      if(attr==2 || attr ==3)
      {
        //read the tuple at rid
        if ((rc = rf.read(rid, key, value)) < 0) 
        {
          fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
          rf.close();
          return rc;
        }
      }
      if(key==key_max)
      {
        if(value_in_where==true)
        {
          for(unsigned i = 0; i < cond.size(); i++)
          {
            if(cond[i].attr==2)
            {
              if(isValidValue(value.c_str(), cond[i].value, cond[i].comp)==true)
              {

                printTuple(attr, key, value);
                count++;
              }
            }
          }
        }


      }
    }
    //range where max is not specified but minimum is, ie key > 800
    else if(key_max==-1)
    {
      b_tree.locate(key_min, cursor);
     // cout<<"Got past locate with key_min: "<<key_min<<endl;

      //if looking for greater than but not equal to, readForward one more
      //if(greater_than_not_equal)
      //{
      //  b_tree.readForward(cursor, key, rid);
      //}


      while(b_tree.readForward(cursor, key, rid)==0)
      {
        if((greater_than_not_equal==true) && (key == key_min))
        {
          continue;
        }
       // cout<<"Cursor Key: "<<key<<endl;
        //cout<<"Rid.sid: "<<rid.sid<<" Rid.pid: "<<rid.pid<<endl;
        if(attr ==2 || attr==3)
        {
          if ((rc = rf.read(rid, key, value)) < 0) 
          {
            fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
            rf.close();
            return rc;
          }
        }
        if(value_in_where==true)
        {
          for(unsigned i = 0; i < cond.size(); i++)
          {
            if(cond[i].attr==2)
            {
              if(isValidValue(value.c_str(), cond[i].value, cond[i].comp) ==true)
              {

                printTuple(attr, key, value);
                count++;
              }
            }
          }
        }
      }

    }
    else if(key_min < key_max)
    {
      //go from min until max
      b_tree.locate(key_min, cursor);

      //if(greater_than_not_equal)
      //{
      //  b_tree.readForward(cursor, key, rid);
      //}



      while((b_tree.readForward(cursor, key, rid)==0))
      {
        if((greater_than_not_equal==true) && (key==key_min))
        {
          continue;
        }
        //cout<<"Rid.sid: "<<rid.sid<<" Rid.pid: "<<rid.pid<<endl;
        if((less_than_not_equal==true) && (key >= key_max))
        {
          break;
        }
        if(attr==2 || attr==3)
        {
          if ((rc = rf.read(rid, key, value)) < 0) 
          {
            fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
            rf.close();
            return rc;
          }
        }

        if(value_in_where==true)
        {
          for(unsigned i = 0; i < cond.size(); i++)
          {
            if(cond[i].attr==2)
            {
              if(isValidValue(value.c_str(), cond[i].value, cond[i].comp)==true)
              {

                printTuple(attr, key, value);
                count++;
              }
            }
          }
        }

      }
    }
     // print matching tuple count if "select count(*)"
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }

    rf.close();
    b_tree.close();

  }
  else
  {
    //cout<<"Got inside else statement"<<endl;
    // scan the table file from the beginning
    rid.pid = rid.sid = 0;
    count = 0;
    while (rid < rf.endRid()) {
      // read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      // check the conditions on the tuple
      for (unsigned i = 0; i < cond.size(); i++) {
        // compute the difference between the tuple value and the condition value
        switch (cond[i].attr) {
          case 1:
	         diff = key - atoi(cond[i].value);
	         break;
          case 2:
	         diff = strcmp(value.c_str(), cond[i].value);
	         break;
        }

        // skip the tuple if any condition is not met
        switch (cond[i].comp) {
          case SelCond::EQ:
	         if (diff != 0) goto next_tuple;
	         break;
          case SelCond::NE:
	         if (diff == 0) goto next_tuple;
	         break;
          case SelCond::GT:
	         if (diff <= 0) goto next_tuple;
	         break;
          case SelCond::LT:
	         if (diff >= 0) goto next_tuple;
	         break;
          case SelCond::GE:
	         if (diff < 0) goto next_tuple;
	         break;
          case SelCond::LE:
	         if (diff > 0) goto next_tuple;
	         break;
        }
      }

      // the condition is met for the tuple. 
      // increase matching tuple counter
      count++;

      // print the tuple 
      switch (attr) {
        case 1:  // SELECT key
          fprintf(stdout, "%d\n", key);
          break;
        case 2:  // SELECT value
          fprintf(stdout, "%s\n", value.c_str());
          break;
        case 3:  // SELECT *
          fprintf(stdout, "%d '%s'\n", key, value.c_str());
          break;
      }

      // move to the next tuple
      next_tuple:
      ++rid;
    }

    // print matching tuple count if "select count(*)"
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }
    rc = 0;
  }
  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  //for part 2A, assume index is FALSE

  if(!index)
  {
    //initialize table_file with write functionality
    RecordFile table_file = RecordFile(table+".tbl", 'w');

    //initialize input stream from loadfile
    ifstream load_file(loadfile.c_str());

    //buffer for storing each line as a string
    string line_buffer;

    RecordId rec_id;
    int line_count=0;
    int key=0;
    string value;

    //while(!load_file.eof())
    while(getline(load_file, line_buffer))
    {
      //getline(load_file, line_buffer, '\n');
      parseLoadLine(line_buffer, key, value);
      if(table_file.append(key, value, rec_id ))
      {
        cout<<"Error appending to table line "<<line_count<<endl;
      }
      line_count++;
    }
  }
  else
  {
    //initialize table_file with write functionality
    RecordFile table_file = RecordFile(table+".tbl", 'w');
    BTreeIndex b_tree;
    b_tree.open(table+".idx", 'w');

    ifstream load_file(loadfile.c_str());

    string line_buffer;

    RecordId rec_id;
    int line_count = 0; //remove when testing for time
    int key=0;
    string value;

    //while(!load_file.eof())
    while(getline(load_file, line_buffer))
    {
     // getline(load_file, line_buffer, '\n');
      parseLoadLine(line_buffer, key, value);
      if(table_file.append(key, value, rec_id ))
      {
        cout<<"Error appending to table line "<<line_count<<endl;
      }

      if(b_tree.insert(key, rec_id) != 0)
      {
        cout<<"Error inserting into B+ Tree line "<<line_count<<endl;
      }

      line_count++; //remove when testing for time

    }

    b_tree.close();
   // cout<<"Reached end of load file INDEX with line count"<<line_count<<endl;

  }

 // cout<<"Reached end of load file with line count"<<line_count<<endl;

  return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
