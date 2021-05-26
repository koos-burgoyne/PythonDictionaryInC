# Limited Functionality Python 2.7 Dictionary in C and C++
## General Info
This repository contains two header files that contain the source code, one for C and one for C++. The functionality is limited in that not all functions available in Python are available in these implementations. For example, the dictionaries are limited in the data types they accept as the key because I built these for specific applications that only use type double as key (more on this below).

The main difference between the two implementations is that C does not support classes, and functions are not typically associated with structs. As a result,in the C implementation the pointer for the dictionary is passed to every function.

I built these for specific applications; as a result the C++ version accepts the following types:
* As Key:
  * Double
* As Value:
  * Dictionary
  * Any primitive data type

The C version accepts:
* 'double' as the key and 
* 'int' as the value 

for the key-value pair in the dictionary. 

The 'value' can easily be edited in the source to accept any of the C data types. The 'key' cannot easily be modified because the only hash function is for type 'double'; the contents of the hashing function can be modified to return a hash value for any type if the user needs this functionality, however the hash value returned will only match that of Python's if the CPython source is followed for that specific data type.

## Usage
### C++
```
#include <iostream>
#include "C++_Dictionary.h"

int main() {

  Dict<double,char> hash_table;
  hash_table.insert(20.31, 'A');
  hash_table.insert(43.67, 'B');
  hash_table.insert(90.13, 'C');
  hash_table.insert(72.82, 'D');
  hash_table.insert(81.91, 'G');
  hash_table.insert(12.32, 't');
  hash_table.insert(19.87, 'p');

  std::cout << "Testing Get Method:\n";
  double get_value = 90.13;
  dict_item<double,char>* get_item = hash_table.get(get_value);
  if (get_item != NULL)
    std::cout << get_item->first << ": " << get_item->second << "\n";
  else
    std::cout << "Get method returned null for " << get_value << "\n";

  hash_table.print_all();

  std::cout << "Testing Iterator:\n";
  for (auto &pair : hash_table)
    std::cout << pair.first << ": " << pair.second << "  ";
  std::cout << "\n";

  return 0;
}
```
### C
```
#include <stdio.h>
#include <stdlib.h>

#include "C_Dictionary.h"

int main() {

  Dict* hash_table = new_dict();
  insert(hash_table, 20.31, 12);
  insert(hash_table, 43.67, 29);
  insert(hash_table, 90.13, 67);
  insert(hash_table, 72.82, 92);
  insert(hash_table, 81.91, 92);
  insert(hash_table, 12.32, 92);
  insert(hash_table, 19.87, 92);
  
  printf("Testing Get Method:\n");
  double get_value = 90.13;
  dict_item* get_item = get(hash_table, get_value);
  if (get_item != NULL)
    printf("%.3f: %d\n", get_item->first, get_item->second);
  else
    printf("Get returned null for %.3f\n", get_value);
  
  print_dict(hash_table);

  printf("Testing Iterator:\n");
  for(Iterator* item = begin(hash_table); item->index != hash_table->table_last_entry + 1; next(hash_table,item)) {
    dict_item* this_item = get(hash_table, item->key);
    if (this_item != NULL)
      printf("%.3f: %d  ", this_item->first, this_item->second);
    else
      printf("\nGet returned null for %.3f\n", item->key);
  }
  printf("\n");

  return 0;
}
```
