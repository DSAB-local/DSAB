#ifndef TEST_H
#define TEST_H
#include <unordered_map>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <math.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <set>
#include "../sketch/SketchBase.h"
using namespace std;
class itemType
{
public:
    string id;
    int frequency;
};
void frequencyTest(vector<string> & v,unordered_map<string, int> & item2freq, SketchBase& sketch,const int bytesPerStr,string frequency_file_name );
void insertionSpeedTest(vector<string> & v,SketchBase& sketch,const int bytesPerStr,string throughput_file_name);
void topkTest(vector<string> & v,vector<itemType> & frequentItem,int k,SketchBase& sketch,const int bytesPerStr, string topk_file_name,unordered_map<string, int> & item2idx);
#endif