//implemention based MASketch 
#ifndef LossyCounting_H //must change this MACRO
#define LossyCounting_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include "../hash/cuckoo_hashing.h" //If you want to use DSAB-builtin hashtable must include thiss
#define DEBUG
/*----optional according to your need----*/
#include<string>
#include<iostream>
#include<memory.h>
using std::min;
typedef struct counter
{
	string item;
	int count;
} Counter;

typedef struct LC_type
{
	Counter *bucket;
	Counter *holder;
	Counter *newcount;
	int buckets;
	int holdersize;
	int maxholder;
	int window;
	int epoch;
} LC_type;

bool LossyCountcmp(const pair<string, uint32_t> a, const pair<string, uint32_t> b) {
	return a.second > b.second;
}
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


class LossyCounting: public SketchBase {
private:
	/*----optional according to your need----*/
	int window_size;//parameter

	LC_type * result;
    /*----optional according to your need----*/
	void countershell(int n, Counter a[])
	{
		int i, j, inc;
		Counter v;
		inc = 1;
		do
		{
			inc *= 3;
			inc++;
		} while (inc <= n);
		do
		{
			inc /= 3;
			for (i = inc + 1; i <= n; i++)
			{
				v = a[i - 1];
				j = i;
				while (a[j - inc - 1].item > v.item)
				{
					a[j - 1] = a[j - inc - 1];
					j -= inc;
					if (j <= inc) break;
				}
				a[j - 1] = v;
			}
		} while (inc > 1);
	}
	int countermerge(Counter *newcount, Counter *left, Counter *right,
		int l, int r, int maxholder)
	{  // merge up two lists of counters. returns the size of the lists. 
		int i, j, m;

		if (l + r>maxholder)
		{ // a more advanced implementation would do a realloc here...
			printf("Out of memory -- trying to allocate %d counters\n", l + r);
			exit(1);
		}
		i = 0;
		j = 0;
		m = 0;

		while (i<l && j<r)
		{ // merge two lists
			if (left[i].item == right[j].item)
			{ // sum the counts of identical items
				newcount[m].item = left[i].item;
				newcount[m].count = right[j].count;
				while (left[i].item == right[j].item)
				{
					newcount[m].count += left[i].count;
					i++;
				}
				j++;
			}
			else if (left[i].item<right[j].item)
			{ // else take the left item, creating counts appropriately
				newcount[m].item = left[i].item;
				newcount[m].count = 0;
				while (left[i].item == newcount[m].item)
				{
					newcount[m].count += left[i].count;
					i++;
				}
			}
			else
			{
				newcount[m].item = right[j].item;
				newcount[m].count = right[j].count;
				j++;
			}
			newcount[m].count--;
			if (newcount[m].count>0) m++;
			else
			{ // adjust for items which may have negative or zero counts
				newcount[m].item = -1;
				newcount[m].count = 0;
			}
		}

		// now that the main part of the merging has been done
		// need to copy over what remains of whichever list is not used up

		if (j<r)
		{
			while (j<r)
			{
				if (right[j].count > 1)
				{
					newcount[m].item = right[j].item;
					newcount[m].count = right[j].count - 1;
					m++;
				}
				j++;
			}
		}
		else
			if (i<l)
			{
				while (i<l)
				{
					newcount[m].item = left[i].item;
					newcount[m].count = -1;
					while ((newcount[m].item == left[i].item) && (i<l))
					{
						newcount[m].count += left[i].count;
						i++;
					}
					if (newcount[m].count>0)
						m++;
					else
					{
						newcount[m].item = -1;
						newcount[m].count = 0;
					}
				}
			}

		return(m);
	}

public:
    using SketchBase::sketch_name;//DO NOT change this declaration
    LossyCounting()
    {
        /*constructed function MUST BE non-parameter!!!*/
        sketch_name =  "LossyCounting";//please keep sketch_name the same as class name and .h file name
    }
    void parameterSet(const std::string& parameterName, double  parameterValue)
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
        if (parameterName=="window_size")
        {

            window_size = parameterValue;
            return;
        }
        /*----optional according to your need----*/
    }
    void init()
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
		result = (LC_type *)calloc(1, sizeof(LC_type));
		result->buckets = 0;
		result->holdersize = 0;
		result->epoch = 0;
		result->window = window_size;
		result->maxholder = result->window * 4;
		result->bucket = (Counter*)calloc(result->window + 2, sizeof(Counter));
		result->holder = (Counter*)calloc(result->maxholder, sizeof(Counter));
		result->newcount = (Counter*)calloc(result->maxholder, sizeof(Counter));
        /*----optional according to your need----*/
    }
    void Insert(const char *str, const int & len)
    {
        /*MUST have this function DO NOT change parameter type*/

        /*----optional according to your need----*/
		Counter *tmp;
		// interpret a negative item identifier as a removal
		result->bucket[result->buckets].item = string(str,len);
		result->bucket[result->buckets].count = 1;
		if (result->buckets == result->window)
		{
			countershell(result->window, result->bucket);
			result->holdersize = countermerge(result->newcount, result->bucket, result->holder,
				result->window, result->holdersize, result->maxholder);
			tmp = result->newcount;
			result->newcount = result->holder;
			result->holder = tmp;
			result->buckets = 0;
			result->epoch++;
		}
		else
			result->buckets++;
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
		int i, point=0;
	
	
		// should do a countermerge here.
		countershell(result->buckets, result->bucket);
		result->holdersize = countermerge(result->newcount, result->bucket, result->holder,
			result->buckets, result->holdersize, result->maxholder);
	
		auto swap_tmp = result->newcount;
		result->newcount = result->holder;
		result->holder = swap_tmp;
		result->buckets = 0;
		vector<pair<string, uint32_t> >tmp;
		for (i = 0; i<result->holdersize; i++)
		{
			tmp.emplace_back(make_pair(result->holder[i].item, result->holder[i].count + result->epoch));
		}
		sort(tmp.begin(), tmp.end(),LossyCountcmp);
		
		cout << k << endl;
		cout << result->holdersize << endl;
		for (int i = 0; i < min(k,result->holdersize); i++)
		{
			topkItem.push_back(tmp[i]);
		}
        return topkItem;
        /*----optional according to your need----*/
    }
    void reset()
    {
         /*MUST have this function,reset sketch to the initial state */

        /*----optional according to your need----*/
		free(result->bucket);
		free(result->holder);
		free(result->newcount);
		free(result);
		result = (LC_type *)calloc(1, sizeof(LC_type));
		result->buckets = 0;
		result->holdersize = 0;
		result->epoch = 0;
		result->window = (int) 1.0 / window_size;
		result->maxholder = result->window * 4;
		result->bucket = (Counter*)calloc(result->window + 2, sizeof(Counter));
		result->holder = (Counter*)calloc(result->maxholder, sizeof(Counter));
		result->newcount = (Counter*)calloc(result->maxholder, sizeof(Counter));
        /*----optional according to your need----*/
    }
    ~LossyCounting()
    {
        /*MUST have this function */

        /*----optional according to your need----*/
		free(result->bucket);
		free(result->holder);
		free(result->newcount);
		free(result);
        /*----optional according to your need----*/
    }

    /*----optional You can add your function----*/
};
REGISTER(LossyCounting);
#endif//DO NOT change this file