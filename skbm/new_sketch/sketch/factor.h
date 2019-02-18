#ifndef FACTOR_H
#define FACTOR_H
#define REGISTER(className)                                             \
    className* objectCreator##className(){                              \
        return new className;                                           \
    }                                                                   \
    RegisterAction g_creatorRegister##className(                        \
        #className,(PTRCreateObject)objectCreator##className)
#include <cstdio>
#include <string>
#include <map>
#include <iostream>
using namespace std;

typedef void* (*PTRCreateObject)(void);

class ClassFactory{
private:
    map<string, PTRCreateObject> m_classMap ;
    ClassFactory(){}; //构造函数私有化

public:
    void* getClassByName(string className);
    void registClass(string name, PTRCreateObject method) ;
    static ClassFactory& getInstance() ;
};
class RegisterAction{
public:
    RegisterAction(string className,PTRCreateObject ptrCreateFn){
        ClassFactory::getInstance().registClass(className,ptrCreateFn);
    }
};
#endif