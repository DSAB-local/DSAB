#ifndef YourSketchName_H //must change this MACRO
#define YourSketchName_H //must change this MACRO
#include "SketchBase.h" //DO NOT change this include
#include "factor.h"//DO NOT change this include
#include "../hash/hashfunction.h"//If you want to use DSAB-builtin hashfunction must include this
#include "../hash/cuckoo_hashing.h" //If you want to use DSAB-builtin hashtable must include thiss

/*----optional according to your need----*/
#include<string>
#include<iostream>
#include<memory.h>
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


class YourSketchName: public SketchBase {
private:
    /*----optional according to your need----*/
    BOBHash *hash;//optional DSAB-builtin hashfunction
    int hash_num;
    int counter_per_array;
    int bit_per_counter;
    int **data;
    /*----optional according to your need----*/
public:
    using SketchBase::sketch_name;//DO NOT change this declaration
    YourSketchName()
    {
        /*constructed function MUST BE non-parameter!!!*/
        sketch_name =  "YourSketchName";//please keep sketch_name the same as class name and .h file name
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
        if (parameterName=="bit_per_counter")
        {
            bit_per_counter = parameterValue;
            return;
        }
         if (parameterName=="counter_per_array")
        {
            counter_per_array = parameterValue;
            return;
        }
        /*----optional according to your need----*/
    }
    void init()
    {
        /*MUST have this function even empty function body*/

        /*----optional according to your need----*/
        data = new int*[hash_num];
        hash = new BOBHash[hash_num];
        for (int i = 0; i<hash_num; ++i)
        {
            data[i] = new int[counter_per_array];
            memset(data[i],0,sizeof(int)*counter_per_array);
            hash[i].SetSeed(1001*i)
        }
        /*----optional according to your need----*/
    }
    void Insert(const char *str, const int & len)
    {
        /*MUST have this function DO NOT change parameter type*/

        /*----optional according to your need----*/
        for (int i = 0; i < hash_num; ++i)
        {
            ++data[i][hash[i].Run((char *)str, len) % counter_per_array];
        }
        /*----optional according to your need----*/
    }
    int frequencyQuery(const char *str, const int & len)
    {
         /*MUST have this function DO NOT change function head and parameter type */

        /*----optional according to your need----*/
        int res = data[0][hash[i].Run((char *)str, len) % counter_per_array];
        for (int i = 1; i < hash_num; ++i) {
            int t = data[i][hash[i].Run((char *)str, len) % counter_per_array];
            res = res < t ? res : t;
        }
        return res;
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
            memset(data[i],0,sizeof(int)*counter_per_array);
        }
        /*----optional according to your need----*/
    }
    ~YourSketchName()
    {
        /*MUST have this function */

        /*----optional according to your need----*/
        for (int i; i<hash_num; ++i)
        {
            delete [] data[i];
        }
        delete [] data;
        delete [] hash;
        /*----optional according to your need----*/
    }

    /*----optional You can add your function----*/
};
REGISTER(YourSketchName);
#endif//DO NOT change this file