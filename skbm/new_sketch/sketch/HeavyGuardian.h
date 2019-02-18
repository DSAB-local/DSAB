#ifndef HeavyGuardian_H //must change this MACRO
#define HeavyGuardian_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include "../hash/cuckoo_hashing.h" //If you want to use DSAB-builtin hashtable must include thiss

/*----optional according to your need----*/
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#define G 8
#define HK_b 1.08
#define str_len 4
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

bool ansCMP(std::pair<string, int> a, std::pair<string, int> b)
{
	return a.second > b.second;
}
class HeavyGuardian: public SketchBase {
private:
	/*----optional according to your need----*/
	struct node { int C; unsigned int FP; } HK[1000005][G + 2];
	int HL[1000005][16];

	BOBHash * bobhash;//optional DSAB-builtin hashfunction
	BOBHash * lighthash;//optional DSAB-builtin hashfunction
	int bucket_num;//parameter
	int threshold;//parameter
	int cell_num;//parameter
	int lightcell_num;//parameter
    /*----optional according to your need----*/
public:
    using SketchBase::sketch_name;//DO NOT change this declaration
	int cnt;
	string ans[1000005];
	void ADD(string x) { ans[++cnt] = x; }
    HeavyGuardian()
    {
        /*constructed function MUST BT non-parameter!!!*/
        sketch_name =  "HeavyGuardian";//please keep sketch_name the same as class name and .h file name
    }
    void parameterSet(const std::string& parameterName, double  parameterValue)
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
        if (parameterName=="cell_num")
        {

            cell_num = parameterValue;
            return;
        }
		if (parameterName == "lightcell_num")
		{

			lightcell_num = parameterValue;
			return;
		}
        if (parameterName=="bucket_num")
        {
			bucket_num = parameterValue;
            return;
        }
         if (parameterName=="threshold")
        {
			 threshold = parameterValue;
            return;
        }
        /*----optional according to your need----*/
    }
    void init()
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
		cnt = 0; 
		bobhash = new BOBHash();
		lighthash = new BOBHash();
		lighthash->SetSeed(750);
		bobhash->SetSeed(1001);
        /*----optional according to your need----*/
    }
    void Insert(const char *str, const int & len)
    {
        /*MUST have this function DO NOT change parameter type*/

        /*----optional according to your need----*/
		unsigned int H = bobhash->Run(str, len);
		unsigned int FP = (H >> 16), Hsh = H % bucket_num;
		bool FLAG = false;
		for (int k = 0; k<cell_num; k++)
		{
			int c = HK[Hsh][k].C;
			if (HK[Hsh][k].FP == FP)
			{
				HK[Hsh][k].C++;
				if (HK[Hsh][k].C == threshold) ADD(string(str,len));
				FLAG = true;
				break;
			}
		}
		if (!FLAG)
		{
			int X, MIN = 1000000000;
			for (int k = 0; k<cell_num; k++)
			{
				int c = HK[Hsh][k].C;
				if (c<MIN) { MIN = c; X = k; }
			}
			if (!(rand() % int(pow(HK_b, HK[Hsh][X].C))))
			{
				HK[Hsh][X].C--;
				if (HK[Hsh][X].C <= 0)
				{
					HK[Hsh][X].FP = FP;
					HK[Hsh][X].C = 1;
					FLAG = true;
				}
			}
		}
		if (!FLAG)
		{
		   unsigned int  pos = lighthash->Run(str, len) % lightcell_num;
		   HL[Hsh][pos] += 1;
		}
        /*----optional according to your need----*/
    }
    int frequencyQuery(const char *str, const int & len)
    {
         /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		unsigned int H = bobhash->Run(str, len);
		unsigned int FP = (H >> 16), Hsh = H % bucket_num;
		for (int k = 0; k<cell_num; k++)
		{
			int c = HK[Hsh][k].C;
			if (HK[Hsh][k].FP == FP) return HK[Hsh][k].C;
		}
		unsigned int  pos = lighthash->Run(str, len) % lightcell_num;
		return  HL[Hsh][pos];
        /*----optional according to your need----*/
    }
	std::vector<std::pair <std::string, int> >  topkQuery(const int & k)
    {
        /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
		std::vector<std::pair <std::string, int> > topkItem;
		std::vector<std::pair <std::string, int> > ansItem;
		for (int i = 1; i <= cnt; ++i)
		{
			ansItem.push_back(make_pair(ans[i], frequencyQuery(ans[i].c_str(),str_len)));
		}
		sort(ansItem.begin(), ansItem.end(), ansCMP);
		int t = cnt > k ? k : cnt;
		for (int i = 0; i < t; ++i)
		{
			topkItem.push_back(ansItem[i]);
		}
        return topkItem;
        /*----optional according to your need----*/
    }
    void reset()
    {
         /*MUST have this function,reset sketch to the initial state */

        /*----optional according to your need----*/
		cnt = 0;
		memset(HK, 0, sizeof(HK));
        /*----optional according to your need----*/
    }
    ~HeavyGuardian()
    {
        /*MUST have this function */

        /*----optional according to your need----*/
        /*----optional according to your need----*/
    }

    /*----optional You can add your function----*/
};
REGISTER(HeavyGuardian);
#endif//DO NOT change this file