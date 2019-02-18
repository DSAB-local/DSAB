#ifndef CM_CU_SKETCH_H
#define CM_CU_SKETCH_H
#include "SketchBase.h"
#include "factor.h"
#include "../hash/hashfunction.h"
#include<string>
#include<iostream>
#include<memory.h>
/*
int is type of frequencyQuery Return
hashfunction type:BOBHash\
virtual ~SketchBase(){}
virtual void Insert(const char * str, const int & len) = 0;
virtual int frequencyQuery(const char * str, const int & len) = 0;
virtual std::vector<std::pair <std::string, int> > topkQuery(const int & k) = 0;
virtual void parameterSet(const std::string& parameterName, double  parameterValue)=0;
virtual void init() = 0;
virtual void reset() = 0;//reset sketch to the initial state
*/
class CM_CU : public SketchBase {
private:
	int hash_num;//parameter
	int memory_in_bytes;//parameter
	int w = 0;
	BOBHash * hash;
	int **data;
public:
	using SketchBase::sketch_name;
	CM_CU()
	{
		sketch_name = "CM_CU";
	}
	void parameterSet(const std::string& parameterName, double  parameterValue)
	{

		if (parameterName == "hash_num")
		{

			hash_num = parameterValue;
			return;
		}
		if (parameterName == "memory_in_bytes")
		{
			memory_in_bytes = parameterValue;
			return;
		}
	}
	void init()
	{
		w = memory_in_bytes / 4 / hash_num;
		data = new int*[hash_num];
		hash = new BOBHash[hash_num];
		for (int i = 0; i<hash_num; ++i)
		{
			data[i] = new int[w];
			memset(data[i], 0, sizeof(int)*w);
			hash[i].SetSeed(i + 750);
		}

	}
	void Insert(const char * str, const int & len)
	{

		int tmin = 1 << 30;
		int idxs[100];
		for (int i = 0; i < hash_num; ++i) {
			idxs[i] = hash[i].Run(str, len) % w;
			tmin = min(data[i][idxs[i]], tmin);
		}
		for (int i = 0; i < hash_num; ++i) {
			if (data[i][idxs[i]] == tmin)
				++data[i][idxs[i]];
		}
	}
	int frequencyQuery(const char * str, const int & len)
	{
		int res = data[0][hash[0].Run(str, len) % w];
		for (int i = 1; i < hash_num; ++i) {
			int t = data[i][hash[i].Run(str, len) % w];
			res = res < t ? res : t;
		}
		return res;
	}
	std::vector<std::pair <std::string, int> > topkQuery(const int & k)
	{
		std::vector<std::pair <std::string, int> > topkItem;
		return topkItem;
	}
	void reset()
	{
		for (int i; i<hash_num; ++i)
		{
			memset(data[i], 0, sizeof(int)*w);
		}
	}
	~CM_CU()
	{
		for (int i; i<hash_num; ++i)
		{
			delete[] data[i];
		}
		delete[] data;
	}
};
REGISTER(CM_CU);
#endif