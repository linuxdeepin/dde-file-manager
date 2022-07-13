#ifndef STUBSHADOW_H
#define STUBSHADOW_H
/* 
 * Author:     Zhang Yu <clauszy@163.com>
 * Maintainer: Zhang Yu <clauszy@163.com>
 * 
 * MIT License
 * 
 * Copyright (c) 2020 Zhang Yu
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <unordered_map>
#include <assert.h>

namespace stub_ext {

#define LAMDA_FUNCTION_TYPE decltype(&Lamda::operator())

class Wrapper
{
public:
    Wrapper();
    virtual ~Wrapper();
};

typedef std::unordered_map<long, Wrapper* > WrapperMap;
extern WrapperMap stub_wrappers;

template<class Lamda>
class LamdaWrapper : public Wrapper
{
public:
    LamdaWrapper(Lamda func): Wrapper(),_func(func){}
    ~LamdaWrapper(){}
    Lamda _func;
};

template <typename Func>
struct VFLocator
{

};

template <class Obj, typename Ret, typename... Args>
struct VFLocator<Ret (Obj::*)(Args...)>
{
    typedef Ret (*Func)(Obj*, Args...);
};

template <class Obj, typename Ret, typename... Args>
struct VFLocator<Ret (Obj::*)(Args...) const>
{
    typedef Ret (*Func)(Obj*, Args...);
};

template <typename Func>
struct LamdaCaller
{

};

template <class Obj, typename Ret, typename... Args>
struct LamdaCaller<Ret (Obj::*)(Args...) const>
{
    template<class Lamda, typename ...OrgArgs>
    static Ret call(LamdaWrapper<Lamda> *wrapper, OrgArgs... args)
    {
        return wrapper->_func(args...);
    }
};

template <class Obj, typename Ret>
struct LamdaCaller<Ret (Obj::*)() const>
{
    template<class Lamda, typename ...OrgArgs>
    static Ret call(LamdaWrapper<Lamda> *wrapper, OrgArgs... args)
    {
        return wrapper->_func();
    }
};

template<typename Func, class Lamda>
struct FuncShadow
{

};

template<typename Ret, typename... Args, class Lamda>
struct FuncShadow<Ret (*)(Args...), Lamda>
{
    typedef Ret (*Shadow)(Args...);
    typedef Ret RetType;

    static Ret call(Args ...args)
    {
       Shadow shadow = &call;
       long id = (long)shadow;
       auto iter = stub_wrappers.find(id);
       assert(stub_wrappers.find(id) != stub_wrappers.end());
       LamdaWrapper<Lamda> *wrapper = dynamic_cast<LamdaWrapper<Lamda> *>(iter->second);
       return LamdaCaller<LAMDA_FUNCTION_TYPE>::call(wrapper, args...);
    }
};

template<typename Ret, class Obj,typename... Args, class Lamda>
struct FuncShadow<Ret (Obj::*)(Args...), Lamda>
{
    typedef Ret (*Shadow)(Obj *,Args...);
    typedef Ret RetType;
    static Ret call(Obj *obj, Args ...args)
    {
        Shadow shadow = &call;
        long id = (long)shadow;
        auto iter = stub_wrappers.find(id);
        assert(stub_wrappers.find(id) != stub_wrappers.end());
        LamdaWrapper<Lamda> *wrapper = dynamic_cast<LamdaWrapper<Lamda> *>(iter->second);
        return LamdaCaller<LAMDA_FUNCTION_TYPE>::call(wrapper, obj, args...);
    }
};


template<typename Ret, class Obj,typename... Args, class Lamda>
struct FuncShadow<Ret (Obj::*)(Args...) const, Lamda>
{
    typedef Ret (*Shadow)(Obj *,Args...);
    typedef Ret RetType;
    static Ret call(Obj *obj, Args ...args)
    {
        Shadow shadow = &call;
        long id = (long)shadow;
        auto iter = stub_wrappers.find(id);
        assert(stub_wrappers.find(id) != stub_wrappers.end());
        LamdaWrapper<Lamda> *wrapper = dynamic_cast<LamdaWrapper<Lamda> *>(iter->second);
        return LamdaCaller<LAMDA_FUNCTION_TYPE>::call(wrapper, obj, args...);
    }
};

template<typename Func, class Lamda>
typename FuncShadow<Func, Lamda>::Shadow depictShadow(Wrapper **wrapper, Func func, Lamda lamda)
{
    *wrapper = new LamdaWrapper<Lamda>(lamda);
    typename FuncShadow<Func,Lamda>::Shadow shadow = &FuncShadow<Func,Lamda>::call;
    long id = (long)shadow;
    assert(stub_wrappers.find(id) == stub_wrappers.end());
    stub_wrappers.insert(std::make_pair(id,*wrapper));
    return shadow;
}

void freeWrapper(Wrapper *wrapper);

}

#endif // STUBSHADOW_H
