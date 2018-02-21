#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include <iostream>
#include <type_traits>

#include "typeinfo.h"

namespace task_graph
{

template<typename T>
class task_data_storage
{
public:
    static T& Get() { return data; }
private:
    static T data;
};
template<typename T>
T task_data_storage<T>::data;

class task_wrap_base
{
public:
    virtual ~task_wrap_base() {}
    virtual void run() = 0;
    void print()
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
    std::vector<typeindex>& get_inputs() { return inputs; }
    std::vector<typeindex>& get_outputs() { return outputs; }
protected:
    std::vector<typeindex> inputs;
    std::vector<typeindex> outputs;
};

template<typename Arg1, typename Arg2 = void>
class task_wrap : public task_wrap_base
{
public:
    task_wrap(void(*f)(Arg1, Arg2))
    : func(f) 
    {
        typeindex a1 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Index();
        typeindex a2 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Index();
        std::is_const<typename std::remove_reference<Arg1>::type>::value ?
            inputs.push_back(a1) : outputs.push_back(a1);
        std::is_const<typename std::remove_reference<Arg2>::type>::value ?
            inputs.push_back(a2) : outputs.push_back(a2);
    }
    virtual void run()
    {
        Arg1& a1 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Get();
        Arg2& a2 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Get();
        func(a1, a2);
    }
private:
    void(*func)(Arg1, Arg2);
};

template<typename Arg1>
class task_wrap<Arg1, void> : public task_wrap_base
{
public:
    task_wrap(void(*f)(Arg1))
    : func(f) 
    {
        typeindex a1 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Index();
        std::is_const<typename std::remove_reference<Arg1>::type>::value ?
            inputs.push_back(a1) : outputs.push_back(a1);
    }
    virtual void run()
    {
        Arg1& a1 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Get();
        func(a1);
    }
    void (*func)(Arg1);
};

class graph
{
public:
    template<typename Arg1>
    void operator+=(void(*fn)(Arg1)){
        tasks.push_back(new task_wrap<Arg1>(fn));
        sort();
    }
    template<typename Arg1, typename Arg2>
    void operator+=(void(*fn)(Arg1, Arg2)){
        tasks.push_back(new task_wrap<Arg1, Arg2>(fn));
        sort();
    }
    
    void sort()
    {
        std::vector<task_wrap_base*> tmp_tasks = tasks;
        tasks.clear();
        
        int sorted_tasks_count;
        do
        {
            sorted_tasks_count = 0;
            for(unsigned i = 0; i < tmp_tasks.size(); ++i)
            {
                task_wrap_base* task = tmp_tasks[i];
                if(!task)
                    continue;
                tmp_tasks[i] = 0;
                if(is_any_input_connected(task, tmp_tasks))
                {
                    tmp_tasks[i] = task;
                }
                else
                {
                    tasks.push_back(task);
                    sorted_tasks_count++;
                }
            }
        }while(sorted_tasks_count != 0);
        for(unsigned i = 0; i < tmp_tasks.size(); ++i)
        {
            if(tmp_tasks[i] != 0)
            {
                std::cout << "Task graph is not acyclic" << std::endl;
                break;
            }
        }
    }
    
    void run()
    {
        for(unsigned i = 0; i < tasks.size(); ++i)
        {
            tasks[i]->run();
        }
    }
private:
    bool is_any_input_connected(task_wrap_base* task, const std::vector<task_wrap_base*> other)
    {
        std::vector<typeindex>& inputs = task->get_inputs();
        for(typeindex in : inputs)
        {
            for(unsigned i = 0; i < other.size(); ++i)
            {
                if(!other[i])
                    continue;
                std::vector<typeindex>& outputs = other[i]->get_outputs();
                for(typeindex out : outputs)
                {
                    if(in == out)
                        return true;
                }
            }
        }
        return false;
    }
    std::vector<task_wrap_base*> tasks;
};

}

#endif
