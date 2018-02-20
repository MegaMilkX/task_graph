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
    std::vector<typeindex>& GetInputs() { return inputs; }
    std::vector<typeindex>& GetOutputs() { return outputs; }
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
        tasks.push_back(new TaskWrap<Arg1>(fn));
        Sort();
    }
    template<typename Arg1, typename Arg2>
    void operator+=(void(*fn)(Arg1, Arg2)){
        tasks.push_back(new TaskWrap<Arg1, Arg2>(fn));
        Sort();
    }
    
    void Sort()
    {
        std::vector<TaskWrapBase*> tmpTasks = tasks;
        tasks.clear();
        
        int sortPassCount;
        do
        {
            sortPassCount = 0;
            for(unsigned i = 0; i < tmpTasks.size(); ++i)
            {
                TaskWrapBase* task = tmpTasks[i];
                if(!task)
                    continue;
                tmpTasks[i] = 0;
                if(IsAnyInputConnected(task, tmpTasks))
                {
                    tmpTasks[i] = task;
                }
                else
                {
                    tasks.push_back(task);
                    sortPassCount++;
                }
            }
        }while(sortPassCount != 0);
        for(unsigned i = 0; i < tmpTasks.size(); ++i)
        {
            if(tmpTasks[i] != 0)
            {
                std::cout << "Task graph is not acyclic" << std::endl;
                break;
            }
        }
    }
    
    void Run()
    {
        for(unsigned i = 0; i < tasks.size(); ++i)
        {
            tasks[i]->Run();
        }
    }
private:
    bool IsAnyInputConnected(TaskWrapBase* task, const std::vector<TaskWrapBase*> other)
    {
        std::vector<typeindex>& inputs = task->GetInputs();
        for(typeindex in : inputs)
        {
            for(unsigned i = 0; i < other.size(); ++i)
            {
                if(!other[i])
                    continue;
                std::vector<typeindex>& outputs = other[i]->GetOutputs();
                for(typeindex out : outputs)
                {
                    if(in == out)
                        return true;
                }
            }
        }
        return false;
    }
    std::vector<TaskWrapBase*> tasks;
};

#endif
