#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include<string>
#include<iostream>
#include<memory.h>
#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <x86intrin.h>
#include <bmiintrin.h>
#include<memory.h>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <cmath>
#include <sstream>
#define SQR(X) (X) * (X)
#ifndef myCMCUsketch_H
#define myCMCUsketch_H
template<uint8_t univ_key_len, int capacity, int d = 3>
struct myCMCUSketch {
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

	myCMCUSketch(int mem_in_bytes_) : mem_in_bytes(mem_in_bytes_), heap_element_num(0) {
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
		name_buf << "myCMCUSketch@" << mem_in_bytes;
		name = name_buf.str();
	}

	void insert(const char * key) {
		int idx = hash[0]->Run(key, univ_key_len) % w;
		int tmin = cm_sketch[0][idx];
		int tidx = 0;

		for (int i = 1; i < d; ++i) {
			idx = hash[i]->Run(key, univ_key_len) % w;
			if (tmin > cm_sketch[i][idx])
			{
				tmin =cm_sketch[i][idx];
				tidx = i;
			}

		}
		cm_sketch[tidx][hash[tidx]->Run(key, univ_key_len) % w]++;
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

	~myCMCUSketch() {
		for (int i = 0; i < d; ++i) {
			delete hash[i];
			delete hash_polar[i];
			delete cm_sketch[i];
		}
		return;
	}
};
#endif
