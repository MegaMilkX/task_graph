#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include <iostream>
#include <type_traits>

#include "typeinfo.h"

template<typename T>
class TaskDataStorage
{
public:
    static T& Get() { return data; }
private:
    static T data;
};
template<typename T>
T TaskDataStorage<T>::data;

class TaskWrapBase
{
public:
    virtual ~TaskWrapBase() {}
    virtual void Run() = 0;
    void Print()
    {
        std::cout << "Inputs:";
        for(typeindex type : inputs)
        {
            std::cout << " " << type;
        }
        std::cout << std::endl;
        std::cout << "Outputs:";
        for(typeindex type : outputs)
        {
            std::cout << " " << type;
        }
        std::cout << std::endl;
    }
protected:
    std::vector<typeindex> inputs;
    std::vector<typeindex> outputs;
};

template<typename Arg1, typename Arg2 = void>
class TaskWrap : public TaskWrapBase
{
public:
    TaskWrap(void(*f)(Arg1, Arg2))
    : func(f) 
    {
        typeindex a1 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Index();
        typeindex a2 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Index();
        std::is_const<typename std::remove_reference<Arg1>::type>::value ?
            inputs.push_back(a1) : outputs.push_back(a1);
        std::is_const<typename std::remove_reference<Arg2>::type>::value ?
            inputs.push_back(a2) : outputs.push_back(a2);
    }
    virtual void Run()
    {
        Arg1& a1 = TaskDataStorage<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Get();
        Arg2& a2 = TaskDataStorage<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Get();
        func(a1, a2);
    }
private:
    void(*func)(Arg1, Arg2);
};

template<typename Arg1>
class TaskWrap<Arg1, void> : public TaskWrapBase
{
public:
    TaskWrap(void(*f)(Arg1))
    : func(f) 
    {
        typeindex a1 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Index();
        std::is_const<typename std::remove_reference<Arg1>::type>::value ?
            inputs.push_back(a1) : outputs.push_back(a1);
    }
    virtual void Run()
    {
        Arg1& a1 = TaskDataStorage<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Get();
        func(a1);
    }
    void (*func)(Arg1);
};

class TaskGraph
{
public:
    template<typename Arg1>
    void operator+=(void(*fn)(Arg1)){
        passes.push_back(new TaskWrap<Arg1>(fn));
    }
    template<typename Arg1, typename Arg2>
    void operator+=(void(*fn)(Arg1, Arg2)){
        passes.push_back(new TaskWrap<Arg1, Arg2>(fn));
    }
    
    void Run()
    {
        for(unsigned i = 0; i < passes.size(); ++i)
        {
            passes[i]->Print();
            passes[i]->Run();
        }
    }
private:
    std::vector<TaskWrapBase*> passes;
};

#endif
