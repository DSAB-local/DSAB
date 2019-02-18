#ifndef ColdFilter_Count_Heap_H //must change this MACRO
#define ColdFilter_Count_Heap_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include "../hash/cuckoo_hashing.h" //If you want to use DSAB-builtin hashtable must include this
#include "../sketch/Count_Heap.h"
#pragma pack (16)
/*----optional according to your need----*/
#include<string>
#include<iostream>
#include<memory.h>




#ifndef _SC_H
#define _SC_H

//#include "params.h"
#include <cstring>
#include <algorithm>
#include <emmintrin.h>
#include <immintrin.h>
#include <stdexcept>
#include <x86intrin.h>
using namespace std;

#define MAX_HASH_NUM_SC 4


template<int threshold = 240, int l1_ratio = 65>
class StreamClassifier
{
	//    static constexpr int bucket_num = 1000;
	//    static constexpr int counter_num = 16;
	int memory_in_bytes,bucket_num,counter_num;
	
	int buffer_size;
	int remained;
	int d1;
	int m1_in_bytes;
	int d2;
	int m2_in_bytes;


	//uint32_t **ID;
	//int **counter;
	//int *cur_pos;
	uint32_t ID[1005][16] __attribute__((aligned(16)));
	int counter[1005][16];
	int cur_pos[1005];

	int w1;
	int w_word;
	int w2;
	uint64_t * L1;
	uint16_t * L2;
	//uint64_t L1[m1_in_bytes * 8 / 4 / 16]; // Layer 1 is organized as word, one word contains 16 counter, one counter consist of 4 bit
	//uint16_t L2[m2_in_bytes * 8 / 16]; // Layer 2 is organized as counter, one counter consist of 16 bit

	SketchBase * spa;

	BOBHash * bobhash1;
	BOBHash ** bobhash2;

	int cur_kick;

	void clear_data()
	{
		cur_kick = 0;
		memset(ID, 0, sizeof(ID));
		memset(counter, 0, sizeof(counter));
		memset(cur_pos, 0, sizeof(cur_pos));
		memset(L1, 0, sizeof(L1));
		memset(L2, 0, sizeof(L2));
	}
public:
	StreamClassifier(int memory_in_bytes__,int bucket_num__,int  counter_num__):memory_in_bytes(memory_in_bytes__),bucket_num(bucket_num__),counter_num(counter_num__)
	{
		buffer_size = bucket_num * counter_num * 8;
		remained = memory_in_bytes - buffer_size;

		d1 = 3;
		m1_in_bytes = int(remained * l1_ratio / 100.0);
		d2 = 3;
		m2_in_bytes = int(remained * (100 - l1_ratio) / 100.0);
		//ID = new uint32_t*[bucket_num];
		//counter = new int*[bucket_num];
		//cur_pos = new int[bucket_num];
		//for (int i = 0; i < bucket_num; ++i)
		//{

		//	char* buf = new char[counter_num*4 + 16];
		//	ID[i] = (uint32_t *)(((uint64_t)buf + 15) & ~15);
		//	//ID[i] = new uint32_t[counter_num];
		//	counter[i] = new int[counter_num];
		//}
		w1 = m1_in_bytes * 8 / 4;
		w_word = m1_in_bytes * 8 / 4 / 16;
		w2 = m2_in_bytes * 8 / 16;
		
		L1 = new uint64_t[m1_in_bytes * 8 / 4 / 16];
		L2 = new uint16_t[m2_in_bytes * 8 / 16];

		bobhash2 = new BOBHash*[d2];
		bobhash1 = new BOBHash;
		bobhash1->SetSeed(500);
		for (int i = 0; i < d2; i++) {
			bobhash2[i] = new BOBHash;
			bobhash2[i]->SetSeed(1000 + i);
		}
		clear_data();
		spa = NULL;
	}

	void print_basic_info()
	{
		printf("Stream Classifer\n");
		printf("\tSIMD buffer: %d counters, %.4lf MB occupies\n", bucket_num * counter_num, bucket_num * counter_num * 8.0 / 1024 / 1024);
		printf("\tL1: %d counters, %.4lf MB occupies\n", w1, w1 * 0.5 / 1024 / 1024);
		printf("\tL2: %d counters, %.4lf MB occupies\n", w2, w2 * 2.0 / 1024 / 1024);
	}

	~StreamClassifier()
	{
		delete bobhash1;
		for (int i = 0; i < d2; i++)
			delete bobhash2[i];
	}

	//periodical refreshing for continuous top-k;
	void init_array_period()
	{
		for (int i = 0; i < w_word; i++) {
			uint64_t temp = L1[i];

			temp = (temp & (0xF)) == 0xF ? temp : (temp & 0xFFFFFFFFFFFFFFF0);
			temp = (temp & (0xF0)) == 0xF0 ? temp : (temp & 0xFFFFFFFFFFFFFF0F);
			temp = (temp & (0xF00)) == 0xF00 ? temp : (temp & 0xFFFFFFFFFFFFF0FF);
			temp = (temp & (0xF000)) == 0xF000 ? temp : (temp & 0xFFFFFFFFFFFF0FFF);

			temp = (temp & (0xF0000)) == 0xF0000 ? temp : (temp & 0xFFFFFFFFFFF0FFFF);
			temp = (temp & (0xF00000)) == 0xF00000 ? temp : (temp & 0xFFFFFFFFFF0FFFFF);
			temp = (temp & (0xF000000)) == 0xF000000 ? temp : (temp & 0xFFFFFFFFF0FFFFFF);
			temp = (temp & (0xF0000000)) == 0xF0000000 ? temp : (temp & 0xFFFFFFFF0FFFFFFF);

			temp = (temp & (0xF00000000)) == 0xF00000000 ? temp : (temp & 0xFFFFFFF0FFFFFFFF);
			temp = (temp & (0xF000000000)) == 0xF000000000 ? temp : (temp & 0xFFFFFF0FFFFFFFFF);
			temp = (temp & (0xF0000000000)) == 0xF0000000000 ? temp : (temp & 0xFFFFF0FFFFFFFFFF);
			temp = (temp & (0xF00000000000)) == 0xF00000000000 ? temp : (temp & 0xFFFF0FFFFFFFFFFF);

			temp = (temp & (0xF000000000000)) == 0xF000000000000 ? temp : (temp & 0xFFF0FFFFFFFFFFFF);
			temp = (temp & (0xF0000000000000)) == 0xF0000000000000 ? temp : (temp & 0xFF0FFFFFFFFFFFFF);
			temp = (temp & (0xF00000000000000)) == 0xF00000000000000 ? temp : (temp & 0xF0FFFFFFFFFFFFFF);
			temp = (temp & (0xF000000000000000)) == 0xF000000000000000 ? temp : (temp & 0x0FFFFFFFFFFFFFFF);

			L1[i] = temp;
		}

		for (int i = 0; i < w2; i++) {
			short int temp = L2[i];
			L2[i] = (temp == threshold) ? temp : 0;
		}
	}

	void init_array_all()
	{
		memset(L1, 0, sizeof(uint64_t) * w_word);
		memset(L2, 0, sizeof(short int) * w2);
	}

	void init_spa(SketchBase * _spa)
	{
		spa = _spa;
	}

	void insert(const char * str)
	{
		uint32_t key;
		memcpy(&key, str, 4);
		int bucket_id = key % bucket_num;
		// int bucket_id = key & 0x2FF;

		// the code below assume counter per buckets is 16

#ifdef DEBUG
		cout << __LINE__ << endl;
#endif // DEBUG
		
		const __m128i item = _mm_set1_epi32((int)key);
		int matched;

		if (counter_num == 16) {
			__m128i *keys_p = (__m128i *)ID[bucket_id];
#ifdef DEBUG
			cout << __LINE__ << endl;
#endif // DEBUG
			__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);
			__m128i b_comp = _mm_cmpeq_epi32(item, keys_p[1]);
			__m128i c_comp = _mm_cmpeq_epi32(item, keys_p[2]);
			__m128i d_comp = _mm_cmpeq_epi32(item, keys_p[3]);
#ifdef DEBUG
			cout << __LINE__ << endl;
#endif // DEBUG

			a_comp = _mm_packs_epi32(a_comp, b_comp);
			c_comp = _mm_packs_epi32(c_comp, d_comp);
			a_comp = _mm_packs_epi32(a_comp, c_comp);

			matched = _mm_movemask_epi8(a_comp);
#ifdef DEBUG
			cout << __LINE__ << endl;
#endif // DEBUG
		}
		else if (counter_num == 4) {
#ifdef DEBUG
			cout << __LINE__ << endl;
			cout << bucket_id << endl;
#endif // DEBUG
			__m128i *keys_p = (__m128i *)ID[bucket_id];
#ifdef DEBUG
#endif // DEBUG
			__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);
#ifdef DEBUG
			cout << __LINE__ << endl;
#endif // DEBUG
			matched = _mm_movemask_ps(*(__m128 *)&a_comp);
#ifdef DEBUG
			cout << __LINE__ << endl;
#endif // DEBUG
		}
		else {
			throw std::logic_error("Not implemented.");
		}
#ifdef DEBUG
		cout << __LINE__ << endl;
#endif // DEBUG
		if (matched != 0) {
			//return 32 if input is zero;
			int matched_index = _tzcnt_u32((uint32_t)matched);

			++counter[bucket_id][matched_index];
			return;
		}
#ifdef DEBUG
		cout << __LINE__ << endl;
#endif // DEBUG
		int cur_pos_now = cur_pos[bucket_id];
		if (cur_pos_now != counter_num) {
			// printf("%d\n", cur_pos_now);
			ID[bucket_id][cur_pos_now] = key;
			++counter[bucket_id][cur_pos_now];
			++cur_pos[bucket_id];
			return;
		}

		/****************randomly choose one counter to kick!******************/
		char tmp2[4];
		memcpy(tmp2, &ID[bucket_id][cur_kick], 4);
		insert_SC(tmp2, counter[bucket_id][cur_kick]);
		ID[bucket_id][cur_kick] = key;
		counter[bucket_id][cur_kick] = 1;

		cur_kick = (cur_kick + 1) % counter_num;
	}

	void insert_SC(const char * kick_ID, int kick_f)
	{
#ifdef DEBUG
		cout << __LINE__ << endl;
#endif // DEBUG
		int v1 = 1 << 30;

		int value[MAX_HASH_NUM_SC];
		int index[MAX_HASH_NUM_SC];
		int offset[MAX_HASH_NUM_SC];

		uint64_t hash_value = bobhash1->Run(kick_ID, 4);
		int word_index = hash_value % w_word;
		hash_value >>= 16;

		uint64_t temp = L1[word_index];
		for (int i = 0; i < d1; i++) {
			offset[i] = (hash_value & 0xF);
			value[i] = (temp >> (offset[i] << 2)) & 0xF;
			v1 = std::min(v1, value[i]);

			hash_value >>= 4;
		}

		int temp2 = v1 + kick_f;
		if (temp2 <= 15) { // maybe optimized use SIMD?
			for (int i = 0; i < d1; i++) {
				int temp3 = ((temp >> (offset[i] << 2)) & 0xF);
				if (temp3 < temp2) {
					temp += ((uint64_t)(temp2 - temp3) << (offset[i] << 2));
				}
			}
			L1[word_index] = temp;
			return;
		}

		for (int i = 0; i < d1; i++) {
			temp |= ((uint64_t)0xF << (offset[i] << 2));
		}
		L1[word_index] = temp;

		int delta1 = 15 - v1;
		kick_f -= delta1;

		int v2 = 1 << 30;
		for (int i = 0; i < d2; i++) {
			index[i] = (bobhash2[i]->Run(kick_ID, 4)) % w2;
			value[i] = L2[index[i]];
			v2 = std::min(value[i], v2);
		}

		temp2 = v2 + kick_f;
		if (temp2 <= threshold) {
			for (int i = 0; i < d2; i++) {
				L2[index[i]] = (L2[index[i]] > temp2) ? L2[index[i]] : temp2;
			}
			return;
		}

		for (int i = 0; i < d2; i++) {
			L2[index[i]] = threshold;
		}

		int delta2 = threshold - v2;
		kick_f -= delta2;
		for (int i = 0; i < kick_f; ++i)
		{
			spa->Insert(kick_ID, 4);
		}
	}

	void refresh()
	{
		for (int i = 0; i < bucket_num; i++) {
			for (int j = 0; j < counter_num; j++) {
				insert_SC(ID[i][j], counter[i][j]);
				ID[i][j] = counter[i][j] = 0;
			}
			cur_pos[i] = 0;
		}
		return;
	}

	int query(const char * key)
	{
		int v1 = 1 << 30;

		//        constexpr int max_d = d1 > d2 ? d1 : d2;
		//        int value[max_d];
		//        int index[max_d];
		//        int offset[max_d];

		uint32_t hash_value = (bobhash1->Run(key, 4));
		int word_index = hash_value % w_word;
		hash_value >>= 16;

		uint64_t temp = L1[word_index];
		for (int i = 0; i < d1; i++) {
			int of, val;
			of = (hash_value & 0xF);
			val = (temp >> (of << 2)) & 0xF;
			v1 = std::min(val, v1);
			hash_value >>= 4;
		}

		if (v1 != 15)
			return v1;

		int v2 = 1 << 30;
		for (int i = 0; i < d2; i++) {
			int index = (bobhash2[i]->Run(key, 4)) % w2;
			int value = L2[index];
			v2 = std::min(value, v2);
		}

		return v1 + v2;
	}
};

#endif//_SC_H
/*----optional according to your need----*/

#define T1 15
#define T2 241
#define THRESHOLD (T1 + T2)

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


class ColdFilter_Count_Heap: public SketchBase {
private:
	/*----optional according to your need----*/
	int total_memory_in_bytes;//parameter
	int filter_memory_percent;//parameter
	int bucket_num;//parameter
	int counter_num;//parameter
	int capacity;
	StreamClassifier<T2, 65> *sc;
	Count_Heap sketch;
    /*----optional according to your need----*/
public:
    using SketchBase::sketch_name;//DO NOT change this declaration
    ColdFilter_Count_Heap()
    {
        /*constructed function MUST BT non-parameter!!!*/
        sketch_name =  "ColdFilter_CM_CU_Heap";//please keep sketch_name the same as class name and .h file name
    }
    void parameterSet(const std::string& parameterName, double  parameterValue)
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
        if (parameterName=="total_memory_in_bytes")
        {

			total_memory_in_bytes = parameterValue;
            return;
        }
		if (parameterName == "capacity")
		{

			capacity = parameterValue;
			return;
		}
        if (parameterName=="filter_memory_percent")
        {
			filter_memory_percent= parameterValue;
            return;
        }
         if (parameterName=="bucket_num")
        {
			 bucket_num = parameterValue;
            return;
        }
		 if (parameterName == "counter_num")
		 {
			 counter_num = parameterValue;
			 return;
		 }
        /*----optional according to your need----*/
    }
    void init()
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
		sc = new StreamClassifier<T2, 65>(int64_t(total_memory_in_bytes) * filter_memory_percent / 100, bucket_num, counter_num);
		sketch.parameterSet("memory_in_bytes", int((total_memory_in_bytes) * (100 - filter_memory_percent) / 100));
		sketch.parameterSet("hash_num",3);
		sketch.parameterSet("capacity", capacity);
		sketch.init();
		sc->init_spa(&sketch);
        /*----optional according to your need----*/
    }
    void Insert(const char *str, const int & len)
    {
        /*MUST have this function DO NOT change parameter type*/

        /*----optional according to your need----*/
		sc->insert(str);
        /*----optional according to your need----*/
    }
    int frequencyQuery(const char *str, const int & len)
    {
         /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		int ret = sc->query(str);
		if (ret == THRESHOLD)
			ret += sketch.frequencyQuery(str,len);

		return ret;
        /*----optional according to your need----*/
    }
	std::vector<std::pair <std::string, int> >  topkQuery(const int & k)
    {
        /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
        return sketch.topkQuery(k);
        /*----optional according to your need----*/
    }
    void reset()
    {
         /*MUST have this function,reset sketch to the initial state */

        /*----optional according to your need----*/
     
        /*----optional according to your need----*/
    }
    ~ColdFilter_Count_Heap()
    {
        /*MUST have this function */

        /*----optional according to your need----*/
		delete sc;
        /*----optional according to your need----*/
    }

    /*----optional You can add your function----*/
};
REGISTER(ColdFilter_Count_Heap);
#endif//DO NOT change this file