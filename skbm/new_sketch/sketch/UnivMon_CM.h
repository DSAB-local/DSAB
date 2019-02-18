#ifndef UnivMon_CM_H //must change this MACRO
#define UnivMon_CM_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include "../hash/cuckoo_hashing.h" //If you want to use DSAB-builtin hashtable must include thiss

/*----optional according to your need----*/
#include<string>
#include<iostream>
#include<memory.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>
#include <sstream>
/*----optional according to your need----*/
using std::min;
using std::swap;

#define SQR(X) (X) * (X)
bool cmcurCMP(std::pair<string, int> a, std::pair<string, int> b)
{
	return a.second > b.second;
}
template<uint8_t univ_key_len, int capacity, int d = 3>
struct CMSketch {
public:
	typedef pair <string, int> KV;
	typedef pair <int, string> VK;
	VK heap[capacity];
	int heap_element_num;
	int mem_in_bytes;
	int w;
	int * cm_sketch[d];
	BOBHash * hash[d];
	BOBHash * hash_polar[d];
	unordered_map<string, uint32_t> ht;

	double get_f2()
	{
		double res[d];
		for (int i = 0; i < d; ++i) {
			double est = 0;
			for (int j = 0; j < w; ++j) {
				est += SQR(double(cm_sketch[i][j]));
			}
			res[i] = est;
		}

		sort(res, res + d);
		if (d % 2) {
			return res[d / 2];
		}
		else {
			return (res[d / 2] + res[d / 2 - 1]) / 2;
		}
	}

	// heap
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

	//public:
	string name;

	CMSketch(int mem_in_bytes_) : mem_in_bytes(mem_in_bytes_), heap_element_num(0) {
		//        memset(heap, 0, sizeof(heap));
		w = mem_in_bytes / 4 / d;
		for (int i = 0; i < capacity; ++i) {
			heap[i].first = 0;
		}
		memset(cm_sketch, 0, sizeof(cm_sketch));
		srand(time(0));
		for (int i = 0; i < d; i++) {
			hash[i] = new BOBHash(uint32_t(rand() % MAX_PRIME32));
			hash_polar[i] = new BOBHash(uint32_t(rand() % MAX_PRIME32));
			cm_sketch[i] = new int[w];
			memset(cm_sketch[i], 0, sizeof(int)*w);
		}

		stringstream name_buf;
		name_buf << "CMSketch@" << mem_in_bytes;
		name = name_buf.str();
	}

	void insert(const char * key) {
		

		for (int i = 0; i < d; ++i) {
			int idx = hash[i]->Run(key, univ_key_len) % w;
			cm_sketch[i][idx] += 1;

	
		}
	}

	//    void get_top_k(uint16_t k, uint32_t * result) {
	//        VK * a = new VK[capacity];
	//        memcpy(a, heap, sizeof(heap));
	//        sort(a, a + capacity);
	//        int i;
	//        for (i = 0; i < k && i < capacity; ++i) {
	//            result[i] = a[capacity - 1 - i].second;
	//        }
	//        for (; i < k; ++i) {
	//            result[i] = 0;
	//        }
	//    }

	int frequencyQuery(const char * str, const int & len)
	{
		/*MUST have this function DO NOT change function head and parameter type */

		/*----optional according to your need----*/
		int idx = hash[0]->Run(str, len) % w;
		int tmin = cm_sketch[0][idx];

		for (int i = 1; i < d; ++i) {
			idx = hash[i]->Run(str, len) % w;
			tmin=  min(cm_sketch[i][idx],tmin);
		}
		
		return tmin;
		/*----optional according to your need----*/
	}
	void get_top_k_with_frequency(uint16_t k, vector<KV> & result) {
		VK * a = new VK[capacity];
		for (int i = 0; i < capacity; ++i) {
			a[i] = heap[i];
		}
		sort(a, a + capacity);
		int i;
		for (i = 0; i < k && i < capacity; ++i) {
			result[i].first = a[capacity - 1 - i].second;
			result[i].second = a[capacity - 1 - i].first;
		}
		for (; i < k; ++i) {
			result[i].second = 0;
		}
	}

	void get_l2_heavy_hitters(double alpha, vector<KV> & result)
	{
		get_top_k_with_frequency(capacity, result);
		double f2 = get_f2();
		for (int i = 0; i < capacity; ++i) {
			if (SQR(double(result[i].second)) < alpha * f2) {
				result.resize(i);
				return;
			}
		}
	}

	void get_heavy_hitters(uint32_t threshold, std::vector<pair<string, uint32_t> >& ret)
	{
		ret.clear();
		for (int i = 0; i < capacity; ++i) {
			if (heap[i].first >= threshold) {
				ret.emplace_back(make_pair(heap[i].second, heap[i].first));
			}
		}
	}

	~CMSketch() {
		for (int i = 0; i < d; ++i) {
			delete hash[i];
			delete hash_polar[i];
			delete cm_sketch[i];
		}
		return;
	}
};

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
define: cuckoo::CuckooHashing<univ_key_len> ht;
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


class UnivMon_CM: public SketchBase {
private:
	/*----optional according to your need----*/
	int mem_in_bytes;//parameter
	int level;//parameter
	int capacity=1000;
	typedef CMSketch<4, 1000, 3> L2HitterDetector;

    L2HitterDetector ** sketches;
    BOBHash ** polar_hash;
    int element_num = 0;
    /*----optional according to your need----*/
public:
    using SketchBase::sketch_name;//DO NOT change this declaration
    UnivMon_CM()
    {
        /*constructed function MUST BT non-parameter!!!*/
        sketch_name =  "UnivMon_CM";//please keep sketch_name the same as class name and .h file name
    }
    void parameterSet(const std::string& parameterName, double  parameterValue)
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
        if (parameterName=="mem_in_bytes")
        {

			mem_in_bytes = parameterValue;
            return;
        }
        if (parameterName=="level")
        {
			level = parameterValue;
            return;
        }
        /*----optional according to your need----*/
    }
    void init()
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
		//srand(time(0));
		double total = (1u << level) - 1;
		sketches = new  L2HitterDetector*[level];
		polar_hash = new BOBHash*[level];
		for (int i = 0; i < level; ++i) {
			//int mem_for_sk = int(mem_in_bytes) - level * (4 + 4) * capacity;
			int mem_for_sk = int(mem_in_bytes);
			int mem = int(mem_for_sk / level);
			sketches[i] = new L2HitterDetector(mem);
			auto idx = uint32_t(rand() % MAX_PRIME32);
			polar_hash[i] = new BOBHash;
			polar_hash[i]->SetSeed(idx);
		}
        /*----optional according to your need----*/
    }
    void Insert(const char *str, const int & len)
    {
        /*MUST have this function DO NOT change parameter type*/

        /*----optional according to your need----*/
		int polar;
		element_num++;
		sketches[0]->insert(str);
		for (int i = 1; i < level; ++i) {
			polar = ((polar_hash[i]->Run(str, len))) % 2;
			//            cout << polar << endl;
			if (polar) {
				sketches[i]->insert(str);
			}
			else {
				break;
			}
		}
        /*----optional according to your need----*/
    }
    int frequencyQuery(const char *str, const int & len)
    {
         /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		int polar;
		sketches[0]->insert(str);
		int res = sketches[0]->frequencyQuery(str, len);
		for (int i = 1; i < level; ++i) {
			polar = ((polar_hash[i]->Run(str, len))) % 2;
			//            cout << polar << endl;
			if (polar) {
				res = min(res, sketches[i]->frequencyQuery(str, len));
			}
			else {
				break;
			}
		}
		return res;
        /*----optional according to your need----*/
    }
	std::vector<std::pair <std::string, int> >  topkQuery(const int & k)
    {
        /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		unordered_map<std::string, int> results;
		vector<std::pair<std::string, int>> vec_top_k(k);
		for (int i = level - 1; i >= 0; --i) {
			sketches[i]->get_top_k_with_frequency(k, vec_top_k);
			for (auto kv : vec_top_k) {
				if (results.find(kv.first) == results.end()) {
					results[kv.first] = kv.second;
				}
			}
		}

		std::vector<std::pair <std::string, int> > topkItem;
		std::vector<std::pair <std::string, int> > curItem;
		for (auto & kv : results) {
			
				curItem.emplace_back(kv);
		}
		sort(curItem.begin(), curItem.end(), cmcurCMP);
		int t = curItem.size() > k ? k : curItem.size();
		for (int i = 0; i < t; ++i)
		{
			topkItem.push_back(curItem[i]);
		}
		return topkItem;
        /*----optional according to your need----*/
    }
    void reset()
    {
         /*MUST have this function,reset sketch to the initial state */

        /*----optional according to your need----*/
		element_num = 0;
        /*----optional according to your need----*/
    }
    ~UnivMon_CM()
    {
        /*MUST have this function */

        /*----optional according to your need----*/
        for (int i; i<level; ++i)
        {
			delete  sketches[i];
			delete  polar_hash[i];
        }
        delete [] sketches;
        delete [] polar_hash;
        /*----optional according to your need----*/
    }

    /*----optional You can add your function----*/
};
REGISTER(UnivMon_CM);
#endif//DO NOT change this file