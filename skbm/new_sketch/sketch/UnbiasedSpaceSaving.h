#ifndef UnbiasedSpaceSaving_H //must change this MACRO
#define UnibiasedSpaceSaving_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include "../hash/cuckoo_hashing.h" //If you want to use DSAB-builtin hashtable must include thiss
#define key_len 4
/*----optional according to your need----*/
#include<string>
#include<random>
#include<iostream>
#include<memory.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <sstream>
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
virtual ~SketchBase();
virtual void parameterSet(const string& parameterName, double  parameterValue)=0;
virtual init() = 0;
virtual void Insert(const uint8_t *str, const int & len) = 0;
virtual int frequencyQuery(const uint8_t *str, const int & len) = 0;
virtual vector<string>  topkQuery(const int & k) = 0;
virtual void reset() = 0;//reset sketch to the initial state
*/
/*----SketchBase virtual function must be finished----*/
template<int keylen> struct ussKeyNode;
template<int keylen>
struct ussValueNode
{
	ussValueNode<keylen> * prev = NULL;
	ussValueNode<keylen> * next = NULL;
	ussKeyNode<keylen> * first = NULL;
	uint32_t val = 0;
};

template<int keylen>
struct ussKeyNode
{
	ussValueNode<keylen> * parent = NULL;
	ussKeyNode<keylen> * prev = NULL;
	ussKeyNode<keylen> * next = NULL;
	char key[keylen];
};
class UnbiasedSpaceSaving : public SketchBase {
private:
	/*----optional according to your need----*/
	int capacity;//parameter
	uint32_t mem_in_bytes;



	int bytes_per_item = 36 + key_len * 2;
	typedef ussKeyNode<key_len> SSKeyNode;
	typedef ussValueNode<key_len> SSValNode;
	SSKeyNode *key_nodes;
	SSValNode *val_nodes;


	//    SSKeyNode * key_node_pool[capacity];
	SSValNode ** val_node_pool;
	//    int key_node_empty_cnt;
	int val_node_empty_cnt;

	SSValNode * tail_node;

	unordered_map<string, SSKeyNode *> hash_table;
	/*----optional according to your need----*/
	void append_new_key(const char * key) {
		// exact first key in tail node
		SSKeyNode * victim = tail_node->first;
		random_device rd;
		if (!(rd() % (tail_node->val + 1)))
		{
			const char * old_key = victim->key;
			hash_table.erase(string(old_key, key_len));
			hash_table[string(key, key_len)] = victim;

			memcpy(victim->key, key, key_len);
		}
		add_counter(victim);
	}

	void add_counter(SSKeyNode * my)
	{
		if (my->next == my)
		{
			my->parent->val++;
			SSValNode * brother = my->parent->prev;
			if (brother && brother->val == my->parent->val)
			{
				val_node_pool[val_node_empty_cnt++] = my->parent; // release my parent
																  //                cout << "release" << endl;
				SSValNode * old_parent = my->parent;
				if (old_parent->next)
					old_parent->next->prev = old_parent->prev;
				if (old_parent == tail_node)
					tail_node = old_parent->prev;
				old_parent->prev->next = old_parent->next;
				my->parent = brother;
				my->next = brother->first;
				my->prev = brother->first->prev;
				my->prev->next = my;
				my->next->prev = my;
				brother->first = my;
			}
			else
			{
				return;
			}
		}
		else
		{
			my->next->prev = my->prev;
			my->prev->next = my->next;
			if (my->parent->first == my) {
				my->parent->first = my->next;
			}

			SSValNode * brother = my->parent->prev;
			if (brother && brother->val == my->parent->val + 1) {
				my->parent = brother;
				my->next = brother->first;
				my->prev = brother->first->prev;
				my->prev->next = my;
				my->next->prev = my;
				brother->first = my;
			}
			else {
				SSValNode * new_parent = val_node_pool[--val_node_empty_cnt];
				SSValNode * old_parent = my->parent;
				new_parent->next = old_parent;
				new_parent->prev = brother;
				new_parent->val = old_parent->val + 1;
				new_parent->first = my;
				if (brother)
					brother->next = new_parent;
				old_parent->prev = new_parent;
				my->parent = new_parent;
				my->next = my;
				my->prev = my;
			}
		}
	}
public:
	using SketchBase::sketch_name;//DO NOT change this declaration
	UnbiasedSpaceSaving()
	{
		/*constructed function MUST BT non-parameter!!!*/
		sketch_name = "UnbiasedSpaceSaving";//please keep sketch_name the same as class name and .h file name
	}
	void parameterSet(const std::string& parameterName, double  parameterValue)
	{
		/*MUST have this function even empty function body*/

		/*----optional according to your need----*/
		if (parameterName == "capacity")
		{

			capacity = parameterValue;
			return;
		}
		/*----optional according to your need----*/
	}
	void init()
	{
		/*MUST have this function even empty function body*/

		/*----optional according to your need----*/
		//capacity = mem_in_bytes / bytes_per_item;
		key_nodes = new SSKeyNode[capacity];
		val_nodes = new SSValNode[capacity];
		val_node_pool = new SSValNode*[capacity];
		memset(key_nodes, 0, sizeof(SSKeyNode)*capacity);
        memset(val_nodes, 0, sizeof(SSValNode)*capacity);
			val_node_empty_cnt = 0;
		for (int i = 1; i < capacity; ++i) {
			val_node_pool[val_node_empty_cnt++] = val_nodes + i;
		}

		tail_node = val_nodes;
		val_nodes[0].val = 0;
		val_nodes->first = key_nodes;
		for (int i = 0; i < capacity; ++i) {
			key_nodes[i].next = key_nodes + (i + 1 + capacity) % capacity;
			key_nodes[i].prev = key_nodes + (i - 1 + capacity) % capacity;
			key_nodes[i].parent = tail_node;
		}

		hash_table.reserve(10 * capacity);
		/*----optional according to your need----*/
	}
	void Insert(const char *str, const int & len)
	{
		/*MUST have this function DO NOT change parameter type*/

		/*----optional according to your need----*/
		auto itr = hash_table.find(string(str, len));
		if (itr == hash_table.end())
		{
			// key not found
			append_new_key(str);
		}
		else
		{
			// key found
			add_counter(itr->second);
		}
		/*----optional according to your need----*/
	}
	int frequencyQuery(const char *str, const int & len)
	{
		/*MUST have this function DO NOT change function head and parameter type */

		/*----optional according to your need----*/
		return 0;
		/*----optional according to your need----*/
	}
	std::vector<std::pair <std::string, int> >  topkQuery(const int & k)
	{
		/*MUST have this function DO NOT change function head and parameter type */

		/*----optional according to your need----*/
		std::vector<std::pair <std::string, int> > topkItem;

		SSValNode * p = tail_node;

		while (p->prev)
			p = p->prev;

		int i = 0;
		for (; p; p = p->next) {
			SSKeyNode * v = p->first;
			do {
				topkItem.push_back(make_pair(string(v->key, key_len), p->val));
				//result[i++].second = p->val;
				v = v->next;
				if (i == k) {
					return topkItem;
				}
			} while (v != p->first);
		}

		return topkItem;
		/*----optional according to your need----*/
	}
	void reset()
	{
		/*MUST have this function,reset sketch to the initial state */

		/*----optional according to your need----*/
		//capacity = mem_in_bytes / bytes_per_item;
		key_nodes = new SSKeyNode[capacity];
		val_nodes = new SSValNode[capacity];
		val_node_pool = new SSValNode*[capacity];
		memset(key_nodes, 0, sizeof(SSKeyNode)*capacity);
        memset(val_nodes, 0, sizeof(SSValNode)*capacity);
			val_node_empty_cnt = 0;
		for (int i = 1; i < capacity; ++i) {
			val_node_pool[val_node_empty_cnt++] = val_nodes + i;
		}

		tail_node = val_nodes;
		val_nodes[0].val = 0;
		val_nodes->first = key_nodes;
		for (int i = 0; i < capacity; ++i) {
			key_nodes[i].next = key_nodes + (i + 1 + capacity) % capacity;
			key_nodes[i].prev = key_nodes + (i - 1 + capacity) % capacity;
			key_nodes[i].parent = tail_node;
		}
		hash_table.clear();
		hash_table.reserve(10 * capacity);
		/*----optional according to your need----*/
	}
	~UnbiasedSpaceSaving()
	{
		/*MUST have this function */

		/*----optional according to your need----*/
		delete[] key_nodes;
		delete[] val_nodes;
		delete[] val_node_pool;
		/*----optional according to your need----*/
	}

	/*----optional You can add your function----*/
};
REGISTER(UnbiasedSpaceSaving);
#endif//DO NOT change this file