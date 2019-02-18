#include "../dataset/StreamData.h"
#include "../sketch/sketchList.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <set>
#include "test.h"
#define ROOT_DIR "/root/pku-sketch-benchmark/"
using namespace std;
bool negative_sort_item(itemType &a, itemType &b)
{
    return a.frequency > b.frequency;
}
tuple<string,double> parseArg(string argument) {
    int equPos = argument.find('=');
    string key(argument, 0, equPos);
    double value = stod(string(argument, equPos+1, argument.length()));
    return make_tuple(key, value);
}
void splitString(const string& s, vector<string>& v, const string& sep){
  string::size_type pos1, pos2;
  pos2 = s.find(sep);
  pos1 = 0;
  while(string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));
 
    pos1 = pos2 + sep.size();
    pos2 = s.find(sep, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}
int main(int argc, char *argv[]) {
    /* 
    datasetName+sketchName+task1+...+key1=value1+...: argv[k]   k>=2
    */
    const int bytesPerStr = 4;
    vector< vector<string> > args;
    vector<string> datasets;
    for(int n=1;n<argc;n++) {
        vector<string> items;
        splitString(string(argv[n]), items, "+");
        args.push_back(items);
        datasets.push_back(items[0]);
    }
    for(auto iter = datasets.begin(); iter != datasets.end(); iter++) {
        string datasetName(*iter);
        StreamData dat((string(ROOT_DIR) + string("dataset/")+datasetName).c_str(), bytesPerStr);
        unordered_map<string, int> item2freq;
        unordered_map<string, int> item2idx;
        int idx =0;
        char str[bytesPerStr];
        vector<string> v;
        while (dat.GetNext(str)){
            v.push_back(string(str, bytesPerStr));
            ++item2freq[string(str, bytesPerStr)];
            item2idx[string(str, bytesPerStr)]= idx++;
        }
        vector<itemType> frequentItem;
        for (const auto& p: item2freq)
        {
            itemType tmp;
            tmp.id = p.first;
            tmp.frequency = p.second;
            frequentItem.push_back(tmp);
        }
        sort(frequentItem.begin(),frequentItem.end(),negative_sort_item);
        for(int k=0; k<args.size(); k++) {
            vector<string> items = args[k];
            if(items[0] == datasetName) {
                string sketchName(items[1]);

                vector<string> tasks;
                vector< tuple<string,double> > keyValuePairs;                
                int i=2;
                while(items[i].find("=") == string::npos) {
                    tasks.push_back(items[i]);
                    i++;
                }
                int split = i;
                for(;i<items.size();i++){
                    tuple<string,double> keyValue = parseArg(items[i]);
                    // cout << get<0>(keyValue) << " " << get<1>(keyValue) << endl;
                    keyValuePairs.push_back(keyValue);
                }

                for(int i=0; i<tasks.size();i++){
                	// Now we have datasetName, sketchName, tasks, and keyValuePairs!!
	                SketchBase* player = (SketchBase*)ClassFactory::getInstance().getClassByName(sketchName);
	                for(auto iter=keyValuePairs.begin(); iter!=keyValuePairs.end(); iter++) {
	                    string key = get<0>(*iter);
	                    double val = get<1>(*iter);
	                    // cout << "hello" << endl;
	                    // cout << key << " " << val << endl;
	                    player->parameterSet(key,val);
	                }
	                player->init();
                    string task = tasks[i];
                    string filename = datasetName+"+"+sketchName+"+"+task;
                    for(int j=split;j<items.size();j++){
                        filename += "+"+items[j];
                    }
                    if(task=="freq") {
                        frequencyTest(v,item2freq,*player,bytesPerStr,filename);
                    }
                    else if(task=="topk") {
                        int k_top;
                        for(int i=0; i<keyValuePairs.size();i++){
                            tuple<string,double> keyValue = keyValuePairs[i];
                            if(get<0>(keyValue)=="k") k_top = get<1>(keyValue);
                        }
                        topkTest(v,frequentItem,k_top,*player,bytesPerStr, filename,item2idx);
                    }
                    else if (task == "speed") {
                        insertionSpeedTest(v,*player,bytesPerStr,filename);
                    }
                }

            }
        }
    }
    return 0;
}
