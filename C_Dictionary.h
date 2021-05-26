/*    
	Copyright (C) 2021
	This minimal hash table library was written in C based on the standard Python(2.7.6) library for a dictionary object:
		https://www.python.org/downloads/release/python-276/
		All Python derivative work was written based on Gnu Public License compatible material licensed to the Python Software Foundation.
	All rights reserved.
	
	License: MIT
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
	
	Author: Chris Burgoyne <chris@burgoyne.co.za>

  Supported Types:
    Key:   double
    Value: int
*/

#ifndef Dictionary_H
#define Dictionary_H

#include <math.h>
#include <stdbool.h>

typedef struct {
	int used;
	double first;
	int second;
}dict_item;

#define MAX_VAL 2147483648.0
#define MINSIZE 8
#define PERTURB_SHIFT 5
#define PERTURB_A 2
#define PERTURB_B 1
#define RESIZEFACTOR 4
#define MAXFILL 2.0/3.0
#define DBL_MAX 1E+37
#define USED 1
#define EMPTY 0

typedef struct{
  dict_item* table;
	unsigned long table_size, table_used;
	int table_first_entry, table_last_entry;
} Dict;

Dict* new_dict() {
  Dict* new_dictionary = malloc(sizeof(Dict));
  if (new_dictionary == NULL)
    return (NULL);
  new_dictionary->table = malloc(MINSIZE * sizeof(dict_item));
	new_dictionary->table_size = MINSIZE;
  new_dictionary->table_used = 0;
	new_dictionary->table_first_entry = MINSIZE;
  new_dictionary->table_last_entry = 0;
  return new_dictionary;
}

unsigned long hash_double(double v) {
  double intpart, fractpart;
  int expo;
  unsigned long hipart;
  unsigned long hash_value;
  fractpart = modf(v, &intpart);
  if (fractpart == 0.0) {
    hash_value = (long)intpart;
    return hash_value;
  }
  v = frexp(v, &expo);
  v *= MAX_VAL;
  hipart = (long)v;
  v = (v - (double)hipart) * MAX_VAL;
  hash_value = hipart + (long)v + (expo << 15);
  return hash_value;
}

void track_table_start_end(Dict* dict, unsigned long insertion_idx) {
  // maintaining the iteration list for iteration in time proportional to (last used bin - first used bin)
  if (dict->table_used == 1) {
    dict->table_first_entry = insertion_idx;
    dict->table_last_entry = insertion_idx;	
  }
  // new last entry
  else if ((int)insertion_idx > dict->table_last_entry)
    dict->table_last_entry = (int)insertion_idx;	
  // new first entry
  else if ((int)insertion_idx < dict->table_first_entry)
    dict->table_first_entry = (int)insertion_idx;
}

unsigned long find_insertion_idx(Dict* dict, double key){
  unsigned long hash = hash_double(key);
  unsigned long mask = dict->table_size - 1;
  unsigned long i = hash & mask;
  unsigned long freeslot = -1;
  if (dict->table[i & mask].first == key)
    return i & mask;
  if (dict->table[i & mask].used == EMPTY)
    freeslot = i & mask;
  
  for (unsigned long perturb = hash; ; perturb >>= PERTURB_SHIFT) {
    i = (i << 2) + i + perturb + 1;
    if (dict->table[i & mask].used == EMPTY && dict->table[i & mask].first != 0.0)
      continue;
    if (dict->table[i & mask].used == EMPTY)
      return freeslot == -1 ? (i & mask) : freeslot;
    if (dict->table[i & mask].first == key)
      return i & mask;
  }
  // failed to find empty slot
  return -1;
}

unsigned long find_idx(Dict* dict, double key){
  unsigned long hash = hash_double(key);
  unsigned long mask = dict->table_size - 1;
  unsigned long i = hash & mask;
  unsigned long perturb = hash;
  int counter = 0;
  
  if (dict->table[i & mask].first == key)
    return i & mask;
  for ( ; counter < dict->table_size; perturb >>= PERTURB_SHIFT) {
    i = (i * 5) + perturb + 1;
    if (dict->table[i & mask].used == USED && dict->table[i & mask].first == key) {
      return i & mask;
    }
    ++counter;
  }
  return i & mask;
}

void insert_key_val_pair(Dict* dict, double key, int val) {
  // if key is new find an available location in the table, else find the existing location of the key
  unsigned long insertion_idx = find_insertion_idx(dict, key);
  // new entry
  if (dict->table[insertion_idx].used != USED) {
    // insert key-value pair into the table
    dict->table[insertion_idx].used = USED;
    dict->table[insertion_idx].first = key;
    dict->table[insertion_idx].second = val;
    dict->table_used++;
    // reset tracking of table first and last entries for iterator
    track_table_start_end(dict, insertion_idx);
  // update existing entry
  } else
    dict->table[insertion_idx].second = val;
}

void resize(Dict* dict, ssize_t _newsize, double multiplier) {
  int old_table_used = dict->table_used, old_table_last = dict->table_last_entry, old_table_first = dict->table_first_entry;
  unsigned long old_size = dict->table_size;
  dict->table_used = 0;
  // find new table size using bit shifting where new_size is a power of 2 greater than (old_size * RESIZEFACTOR)
  for (dict->table_size = MINSIZE; dict->table_size < _newsize && dict->table_size > 0; dict->table_size <<= PERTURB_B)
    ;
  // reset tracking for table first and last entries to relative initial values
  dict->table_first_entry = dict->table_size - 1, dict->table_last_entry = 0;
  
  // save pointer to old data
  dict_item *old_table = dict->table;
  
  // allocate new memory for table
  dict->table = malloc(dict->table_size * sizeof(dict_item));
  
  // insert all dict_entries into new memory
  for (int i = old_table_first; i <= old_table_last; ++i)
    if (old_table[i].used == USED)
      insert_key_val_pair(dict, old_table[i].first, old_table[i].second);
  
  // clear the memory in the old table
  free(old_table);
}

void insert(Dict* dict, double key, int value) {
  insert_key_val_pair(dict, key, value);
  if(dict->table_used > dict->table_size * MAXFILL) {
    resize(dict, dict->table_size * RESIZEFACTOR, RESIZEFACTOR);
  }
}

void update(Dict* dict, Dict *from) {
    if (from->table_used == 0)
      return;
    if ((dict->table_used + from->table_used)*3 >= dict->table_size*2)
        resize(dict, (dict->table_used + from->table_used) * 2, 2);
    for (int i = from->table_first_entry; i <= from->table_last_entry; ++i) {
      if (from->table[i].used == USED)
        insert(dict, from->table[i].first, from->table[i].second);
    }
}

void copy_nodes(Dict* dict, Dict* from) {
  // if table to copy is larger than MINSIZE(8), create new table of incoming table size
  if (from->table_size > dict->table_size) {
    free(dict->table);
    dict->table_size = from->table_size;
    dict->table = malloc(dict->table_size*sizeof(dict_item));
  }
  dict->table_used = from->table_used;
  // copy values
  for (int i = 0; i < dict->table_size; ++i) {
    if (from->table[i].used == USED) {
      dict->table[i].used = USED;
      dict->table[i].first = from->table[i].first;
      dict->table[i].second = from->table[i].second;
    }
  }
  
  // track first and last entries for iterator
  for (int i = 0; i < dict->table_size; ++i)
    if (dict->table[i].used == USED) {
      dict->table_first_entry = i;
      break;
    }
  for (int i = dict->table_size - 1; i > 0; --i)
    if (dict->table[i].used == USED) {
      dict->table_last_entry = i;
      break;
    }
}

void erase(Dict* dict, double key) {
  // find position of key in the table
  int insertion_idx = -1;
  for (int i = dict->table_first_entry; i <= dict->table_last_entry; ++i)
    if (dict->table[i].used == USED && dict->table[i].first == key){
      insertion_idx = i;
      break;
    }
  // key not found
  if (insertion_idx == -1)
    return;
  // removal process
  dict->table[insertion_idx].used = EMPTY;
  dict->table_used--;
  // reset position of first or last for the iterator
  if (dict->table_used == 0)
    dict->table_first_entry = dict->table_size, dict->table_last_entry = 0;
  else {
    if ((int)insertion_idx == dict->table_last_entry)
      while (dict->table[dict->table_last_entry].used != USED && dict->table_last_entry > 0)
        --dict->table_last_entry;
    if ((int)insertion_idx == dict->table_first_entry)
      while (dict->table[dict->table_first_entry].used != USED && dict->table_first_entry < dict->table_size)
        ++dict->table_first_entry;
  }
}

bool contains(Dict* dict, double key) {
  if (dict->table_used > 0) {
    unsigned long query_idx = find_idx(dict, key);
    if (dict->table[query_idx].used == USED && dict->table[query_idx].first == key)
      return &dict->table[query_idx];
  }
  return false;
}

dict_item* get(Dict* dict, double key) {
  if (dict->table_used > 0) {
    unsigned long query_idx = find_idx(dict, key);
    if (dict->table[query_idx].used == USED && dict->table[query_idx].first == key)
      return &dict->table[query_idx];
  }
  return NULL;
}

int size(Dict* dict){
  return dict->table_used;
}

double min_entry(Dict* dict) {
  double min = DBL_MAX;
  for (int i = dict->table_first_entry; i <= dict->table_last_entry; ++i)
    if (dict->table[i].used == USED && dict->table[i].first < min)
      min = dict->table[i].first;
  return min;
}

double max_entry(Dict* dict) {
  double max = 0;
  for (int i = dict->table_first_entry; i <= dict->table_last_entry; ++i)
    if (dict->table[i].used == USED && dict->table[i].first > max)
      max = dict->table[i].first;
  return max;
}

void print_dict(Dict* dict) {
  printf("Printing Dictionary Contents:\n");
  for(int i = dict->table_first_entry; i <= dict->table_last_entry; ++i)
    if (dict->table[i].used == USED)
      printf("%.3f: %d; ",dict->table[i].first, dict->table[i].second);
  printf("\n");
}

typedef struct {
  unsigned long index;
  double key;
  int value;
} Iterator;

Iterator* get_iterator(Dict* dict, dict_item* item) {
  Iterator* new_iter = malloc(sizeof(Iterator*));
  new_iter->index = find_idx(dict, item->first);
  new_iter->key = dict->table[new_iter->index].first; 
  new_iter->value = dict->table[new_iter->index].second;
}

Iterator* begin(Dict* dict) {
  return get_iterator(dict, &dict->table[dict->table_first_entry]);
}

Iterator* end(Dict* dict) {
  return get_iterator(dict, &dict->table[dict->table_last_entry + 1]);
}

Iterator* next(Dict* dict, Iterator* item) {
  ++item->index;
  while(dict->table[item->index].used != USED && item->index < dict->table_last_entry) {
    ++item->index;
  }
  item->key = dict->table[item->index].first;
  item->value = dict->table[item->index].second;
  return item;
}


#endif