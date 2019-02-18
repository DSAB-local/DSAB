#ifndef Count_H //must change this MACRO
#define Count_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this

/*----optional according to your need----*/
#include<string>
#include<iostream>
#include<memory.h>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
using std::min;
using std::swap;
#define SQR(X) (X) * (X)
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


class Count: public SketchBase {
private:
	/*----optional according to your need----*/
	int memory_in_bytes;//parameter
	int hash_num;//parameter

	typedef pair <string, int> KV;
	typedef pair <int, string> VK;
	int w;
	int ** c_sketch;
	BOBHash * hash;//optional DSAB-builtin hashfunction
	BOBHash * hash_polar;
    /*----optional according to your need----*/
	
public:
    using SketchBase::sketch_name;//DO NOT change this declaration
    Count()
    {
        /*constructed function MUST BT non-parameter!!!*/
        sketch_name =  "Count";//please keep sketch_name the same as class name and .h file name
    }
    void parameterSet(const std::string& parameterName, double  parameterValue)
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
        if (parameterName=="hash_num")
        {

            hash_num = parameterValue;
            return;
        }
        if (parameterName=="memory_in_bytes")
        {
            memory_in_bytes = parameterValue;
            return;
        }
        /*----optional according to your need----*/
    }
    void init()
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
		w = memory_in_bytes / 4 / hash_num;

        c_sketch = new int*[hash_num];
        hash = new BOBHash[hash_num];
		hash_polar = new BOBHash[hash_num];
		srand(time(0));
        for (int i = 0; i<hash_num; ++i)
        {
			
			hash[i].SetSeed(uint32_t(rand() % MAX_PRIME32));
			hash_polar[i].SetSeed(uint32_t(rand() % MAX_PRIME32));
			c_sketch[i] = new int[w];
			memset(c_sketch[i], 0, sizeof(int)*w);
        }
        /*----optional according to your need----*/
    }
    void Insert(const char *str, const int & len)
    {
        /*MUST have this function DO NOT change parameter type*/

        /*----optional according to your need----*/
		int ans[hash_num];

		for (int i = 0; i < hash_num; ++i) {
			int idx = hash[i].Run(str, len) % w;
			int polar = hash_polar[i].Run(str, len) % 2;
			c_sketch[i][idx] += polar ? 1 : -1;
		}
        /*----optional according to your need----*/
    }
    int frequencyQuery(const char * str, const int & len)
    {
         /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		int ans[1000];

		for (int i = 0; i < hash_num; ++i) {
			int idx = hash[i].Run(str,len) % w;
			int polar = hash_polar[i].Run(str, len) % 2;

			int val = c_sketch[i][idx];

			ans[i] = polar ? val : -val;
		}

		sort(ans, ans + hash_num);

		int tmin;
		if (hash_num % 2 == 0) {
			tmin = (ans[hash_num / 2] + ans[hash_num / 2 - 1]) / 2;
		}
		else {
			tmin = ans[hash_num / 2];
		}
		return (tmin <= 1) ? 1 : tmin;

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
    void reset()
    {
         /*MUST have this function,reset sketch to the initial state */

        /*----optional according to your need----*/
        for (int i; i<hash_num; ++i)
        {
            memset(c_sketch[i],0,sizeof(int)*w);
        }
        /*----optional according to your need----*/
    }
    ~Count()
    {
        /*MUST have this function */

        /*----optional according to your need----*/
        for (int i; i<hash_num; ++i)
        {
            delete [] c_sketch[i];
        }
        delete [] c_sketch;
        delete [] hash;
		delete [] hash_polar;
        /*----optional according to your need----*/
    }

    /*----optional You can add your function----*/
};
REGISTER(Count);
#endif//DO NOT change this file