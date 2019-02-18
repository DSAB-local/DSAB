#include "test.h"
#define ROOT_DIR "/root/pku-sketch-benchmark/"
void frequencyTest(vector<string> & v,unordered_map<string, int> & item2freq, SketchBase& sketch,const int bytesPerStr,string frequency_file_name )
{
    string sketch_name = sketch.sketch_name;

    for(auto iter = v.begin();iter!=v.end();iter++)
        sketch.Insert(iter->c_str(), bytesPerStr);

    /*accuracy test*/
    ofstream frequency_file;
    frequency_file.open(string(ROOT_DIR)+"experiment/output/"+frequency_file_name);
    for (const auto& p: item2freq)
    {
        frequency_file << p.second << " ";
        frequency_file << sketch.frequencyQuery(p.first.c_str(), bytesPerStr);
        frequency_file << endl;
    }
    frequency_file.close();
}
void topkTest(vector<string> & v,vector<itemType> & frequentItem,int k,SketchBase& sketch,const int bytesPerStr, string topk_file_name, unordered_map<string, int> & item2idx)
{

    for(auto iter = v.begin();iter!=v.end();iter++){
        sketch.Insert(iter->c_str(), bytesPerStr);
    }
    /*accuracy test*/
    std::vector<std::pair <std::string, int> > queryResult = sketch.topkQuery(k);
    ofstream topk_file;
    topk_file.open(string(ROOT_DIR)+"experiment/output/"+topk_file_name);
    if(queryResult.size()<k)
    {
        cout<<queryResult.size()<<" "<<k<<endl;
        cout<<"top-k k is too big!!!"<<endl;
        return;
    }
    for(int i =0;i<k;++i)
    {
        topk_file << item2idx[frequentItem[i].id] << " " << frequentItem[i].frequency << " " << item2idx[queryResult[i].first] << " " << queryResult[i].second << endl;
    }
    topk_file.close();

}
void insertionSpeedTest(vector<string> & v,SketchBase& sketch,const int bytesPerStr,string throughput_file_name)
{
    /*throuput test*/

    clock_t start,finish;
    start = clock();
    for(auto iter = v.begin();iter!=v.end();iter++)
        sketch.Insert(iter->c_str(), bytesPerStr);
    finish = clock();

    ofstream throughput_file;
    throughput_file.open(string(ROOT_DIR)+"experiment/output/"+throughput_file_name);
    // throughput_file << "TotalNum\tTime(s)" << endl;
    throughput_file << v.size() << " ";
    throughput_file << double(finish-start)/CLOCKS_PER_SEC << endl;
    throughput_file.close();

}
