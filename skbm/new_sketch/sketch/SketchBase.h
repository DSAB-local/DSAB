#ifndef SKETCH_BASE_H
#define SKETCH_BASE_H
#include <string>
#include <vector>
class SketchBase
{
    public:
        std::string sketch_name;// store sketch name
        virtual ~SketchBase(){}
        virtual void Insert(const char * str, const int & len) = 0;
        virtual int frequencyQuery(const char * str, const int & len) = 0;
        virtual std::vector<std::pair <std::string, int> > topkQuery(const int & k) = 0;
        virtual void parameterSet(const std::string& parameterName, double  parameterValue)=0;
        virtual void init() = 0;
        virtual void reset() = 0;//reset sketch to the initial state
};
#endif