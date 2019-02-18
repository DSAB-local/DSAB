#ifndef Pyramid_CM_CU_H //must change this MACRO
#define Pyramid_CM_CU_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include "../hash/cuckoo_hashing.h" //If you want to use DSAB-builtin hashtable must include thiss
/*----optional according to your need----*/
#include<string>
#include<iostream>
#include<memory.h>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <math.h>

#ifndef _PyramidPARAMS_H
#define _PyramidPARAMS_H

#define MAX_INSERT_PACKAGE 1000000

#define MAX_HASH_NUM_P 20

#define FILTER_SIZE 32

#define COUNTER_SIZE 16

#define LOW_HASH_NUM 4

typedef long long lint;
typedef unsigned int uint;
typedef unsigned long long int uint64;
#endif //_PARAMS_H
using namespace std;
/*----optional according to your need----*/

/*----builtin hashfunction----*/
/*
DSAB-builtin hashfunction type:BOBHash\
HOW TO USE:
    define: e.g. BOBHash myhash
    setseed: e.g. myhash.SetSeed(1001)
    calculate hash: e.g. myhash.Run(const char *str, const int & len)
*/
/*----builtin hashfunction----*/

/*----builtin hashTable----*/
/*
DSAB-builtin hashTable type:cuckoo_hashtable\
HOW TO USE:
define: cuckoo::CuckooHashing<key_len> ht;
!!!MUST init: ht.init(capacity)
bool insert(char * key, uint32_t val, int from_k = -1, int remained = 5)
bool query(char * key, uint32_t & val)
bool find(char * key)
bool erase(char * key)
*/
/*----builtin hashTable----*/

/*----SketchBase virtual function must be finished----*/
/*
virtual ~SketchBase(){}
virtual void Insert(const char * str, const int & len) = 0;
virtual int frequencyQuery(const char * str, const int & len) = 0;
virtual std::vector<std::pair <std::string, int> > topkQuery(const int & k) = 0;
virtual void parameterSet(const std::string& parameterName, double  parameterValue)=0;
virtual void init() = 0;
virtual void reset() = 0;//reset sketch to the initial state
*/
/*----SketchBase virtual function must be finished----*/


class Pyramid_CM_CU: public SketchBase {
private:
	/*----optional according to your need----*/
	int word_num,d,word_size,capacity;//parameter

	uint64 *counter[60];
	int word_index_size, counter_index_size;
	int counter_num;
	BOBHash * bobhash[MAX_HASH_NUM_P];
	//word_num is the number of words in the first level.


	typedef pair <string, int> KV;
	typedef pair <int, string> VK;
	VK * heap;
	int heap_element_num;
	unordered_map<string, uint32_t> ht;
	void heap_adjust_down(int i) {
		while (i < heap_element_num / 2) {
			int l_child = 2 * i + 1;
			int r_child = 2 * i + 2;
			int larger_one = i;
			if (l_child < heap_element_num && heap[l_child] < heap[larger_one]) {
				larger_one = l_child;
			}
			if (r_child < heap_element_num && heap[r_child] < heap[larger_one]) {
				larger_one = r_child;
			}
			if (larger_one != i) {
				swap(heap[i], heap[larger_one]);
				swap(ht[heap[i].second], ht[heap[larger_one].second]);
				heap_adjust_down(larger_one);
			}
			else {
				break;
			}
		}
	}

	void heap_adjust_up(int i) {
		while (i > 1) {
			int parent = (i - 1) / 2;
			if (heap[parent] <= heap[i]) {
				break;
			}
			swap(heap[i], heap[parent]);
			swap(ht[heap[i].second], ht[heap[parent].second]);
			i = parent;
		}
	}
    /*----optional according to your need----*/
public:
    using SketchBase::sketch_name;//DO NOT change this declaration
    Pyramid_CM_CU()
    {
        /*constructed function MUST BT non-parameter!!!*/
        sketch_name =  "Pyramid_CM_CU";//please keep sketch_name the same as class name and .h file name
    }
    void parameterSet(const std::string& parameterName, double  parameterValue)
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
	
        if (parameterName=="word_num")
        {

			word_num = parameterValue;
            return;
        }
        if (parameterName=="d")
        {
            d = parameterValue;
            return;
        }
         if (parameterName=="word_size")
        {
			word_size = parameterValue;
            return;
        }
        /*----optional according to your need----*/
    }
    void init()
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/



		//for calculating the four hash value constrained in one certain word;
		word_index_size = 18;

		counter_index_size = (int)(log(word_size) / log(2)) - 2;//4-8->16-256 counters in one word;
		counter_num = (word_num << counter_index_size);


		for (int i = 0; i < 15; i++)
		{
			counter[i] = new uint64[word_num >> i];
			memset(counter[i], 0, sizeof(uint64) * (word_num >> i));
		}

		for (int i = 0; i < d; i++)
		{
			bobhash[i] = new BOBHash;
			bobhash[i]->SetSeed(i + 1000);
		}

        /*----optional according to your need----*/
    }
    void Insert(const char *str, const int & len)
    {
        /*MUST have this function DO NOT change parameter type*/

        /*----optional according to your need----*/
		int min_value = 1 << 30;

		int value[MAX_HASH_NUM];
		int index[MAX_HASH_NUM];
		int counter_offset[MAX_HASH_NUM];

		uint64 hash_value = (bobhash[0]->Run(str, len));
		int my_word_index = (hash_value & ((1 << word_index_size) - 1)) % word_num;
		hash_value >>= word_index_size;


		for (int i = 0; i < d; i++)
		{
			counter_offset[i] = (hash_value & 0xFFF) % (1 << counter_index_size);
			index[i] = ((my_word_index << counter_index_size) + counter_offset[i]) % counter_num;
			hash_value >>= counter_index_size;

			value[i] = (counter[0][my_word_index] >> (counter_offset[i] << 2)) & 0xF;
			min_value = value[i] < min_value ? value[i] : min_value;
		}

		if (min_value != 15)
		{
			for (int i = 0; i < d; i++)
			{
				value[i] = (counter[0][my_word_index] >> (counter_offset[i] << 2)) & 0xF;
				counter[0][my_word_index] += value[i] == min_value ? ((uint64)0x1 << (counter_offset[i] << 2)) : 0;

			}
			return;
		}


		for (int i = 0; i < d; i++)
		{
			value[i] = (counter[0][my_word_index] >> (counter_offset[i] << 2)) & 0xF;

			if (value[i] == 0)
				continue;

			counter[0][my_word_index] &= (~((uint64)0xF << (counter_offset[i] << 2)));
			carry(index[i]);
		}
		return;
		
        /*----optional according to your need----*/
    }
    int frequencyQuery(const char *str, const int & len)
    {
         /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		int min_value = 1 << 30;

		int value[MAX_HASH_NUM];
		int index[MAX_HASH_NUM];
		int counter_offset[MAX_HASH_NUM];

		uint64 hash_value = (bobhash[0]->Run(str, len));
		int my_word_index = (hash_value & ((1 << word_index_size) - 1)) % word_num;
		hash_value >>= word_index_size;

		for (int i = 0; i < d; i++)
		{
			counter_offset[i] = (hash_value & 0xFFF) % (1 << counter_index_size);
			index[i] = ((my_word_index << counter_index_size) + counter_offset[i]) % counter_num;
			hash_value >>= counter_index_size;

			value[i] = (counter[0][my_word_index] >> (counter_offset[i] << 2)) & 0xF;
			value[i] += get_value(index[i]);
			min_value = value[i] < min_value ? value[i] : min_value;
		}
		return min_value;
        /*----optional according to your need----*/
    }
	std::vector<std::pair <std::string, int> >  topkQuery(const int & k)
    {
        /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		std::vector<std::pair <std::string, int> > topkItem;
		return topkItem;
        /*----optional according to your need----*/
    }
	void down_carry(int index)
	{
		int left_or_right, up_left_or_right;

		int value, up_value;
		int word_index = index >> 4, up_word_index;
		int offset = index & 0xF;
		int up_offset = offset;

		for (int i = 1; i < 15; i++)
		{

			left_or_right = word_index & 1;
			word_index >>= 1;

			up_word_index = (word_index >> 1);
			up_left_or_right = up_word_index & 1;

			value = (counter[i][word_index] >> (offset << 2)) & 0xF;

			if ((value & 3) >= 2)
			{
				counter[i][word_index] -= ((uint64)0x1 << (offset << 2));
				return;
			}
			else if ((value & 3) == 1)
			{
				up_value = (counter[i + 1][up_word_index] >> (up_offset << 2)) & 0xF;

				//change this layer's flag bit;
				if (((up_value >> (2 + up_left_or_right)) & 1) == 0)
				{
					counter[i][word_index] &= (~((uint64)0x1 << (2 + left_or_right + (offset << 2))));
				}

				counter[i][word_index] -= ((uint64)0x1 << (offset << 2));
				return;
			}
			else
			{
				counter[i][word_index] |= ((uint64)0x3 << (offset << 2));
			}
		}
	}

	void carry(int index)
	{
		int left_or_right;

		int value;
		int word_index = index >> 4;
		int offset = index & 0xF;

		for (int i = 1; i < 15; i++)
		{

			left_or_right = word_index & 1;
			word_index >>= 1;

			counter[i][word_index] |= ((uint64)0x1 << (2 + left_or_right + (offset << 2)));
			value = (counter[i][word_index] >> (offset << 2)) & 0xF;

			if ((value & 3) != 3)
			{
				counter[i][word_index] += ((uint64)0x1 << (offset << 2));
				return;
			}
			counter[i][word_index] &= (~((uint64)0x3 << (offset << 2)));
		}
	}

	int get_value(int index)
	{
		int left_or_right;
		int anti_left_or_right;

		int value;
		int word_index = index >> 4;
		int offset = index & 0xF;


		int high_value = 0;

		for (int i = 1; i < 15; i++)
		{

			left_or_right = word_index & 1;
			anti_left_or_right = (left_or_right ^ 1);

			word_index >>= 1;

			value = (counter[i][word_index] >> (offset << 2)) & 0xF;

			if (((value >> (2 + left_or_right)) & 1) == 0)
				return high_value;

			high_value += ((value & 3) - ((value >> (2 + anti_left_or_right)) & 1)) * (1 << (2 + 2 * i));
		}
	}
    void reset()
    {
         /*MUST have this function,reset sketch to the initial state */

        /*----optional according to your need----*/
        /*----optional according to your need----*/
    }
    ~Pyramid_CM_CU()
    {
        /*MUST have this function */

        /*----optional according to your need----*/
        /*----optional according to your need----*/
    }
    /*----optional You can add your function----*/
};
REGISTER(Pyramid_CM_CU);
#endif//DO NOT change this file