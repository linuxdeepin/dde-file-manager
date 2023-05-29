// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: MIT

#ifndef STUBEXT_H
#define STUBEXT_H

//需修改Stub的私用成员函数和成员变量为保护类型
#include "stub.h"

#include "stub-shadow.h"

#ifdef DEBUG_STUB_INVOKE
// use to make sure the stub function is invoked.
#    define __DBG_STUB_INVOKE__ printf("stub at %s:%d is invoked.\n", __FILE__, __LINE__);
#else
#    define __DBG_STUB_INVOKE__
#endif

#define VADDR(CLASS_NAME, MEMBER_NAME) (typename stub_ext::VFLocator<decltype(&CLASS_NAME::MEMBER_NAME)>::Func)(&CLASS_NAME::MEMBER_NAME)

namespace stub_ext {

class StubExt : public Stub
{
public:
    StubExt()
        : Stub() { }

    template<typename T, class Lamda>
    bool set_lamda(T addr, Lamda lamda)
    {
        char *fn = addrof(addr);
        if (m_result.find(fn) != m_result.end())
            reset(addr);

        Wrapper *wrapper = nullptr;
        auto addr_stub = depictShadow(&wrapper, addr, lamda);
        if (set(addr, addr_stub)) {
            m_wrappers.insert(std::make_pair(fn, wrapper));
            return true;
        } else {
            freeWrapper(wrapper);
        }
        return false;
    }

    template<typename T>
    void reset(T addr)
    {
        Stub::reset(addr);
        char *fn = addrof(addr);
        auto iter = m_wrappers.find(fn);
        if (iter != m_wrappers.end()) {
            freeWrapper(iter->second);
            m_wrappers.erase(iter);
        }
    }

    ~StubExt()
    {
        clear();
    }

    void clear() override
    {
        Stub::clear();
        for (auto iter = m_wrappers.begin(); iter != m_wrappers.end(); ++iter) {
            freeWrapper(iter->second);
        }
        m_wrappers.clear();
    }

    template<class T>
    static void *get_ctor_addr(bool start = true)
    {
        // the start vairable must be true, or the compiler will optimize out.
        if (start) goto Start;
    Call_Constructor:
        // This line of code will not be executed.
        // The purpose of the code is to allow the compiler to generate the assembly code that calls the constructor.
        T();
    Start:
        // The address of the line of code T() obtained by assembly
        char *p = (char *)&&Call_Constructor;   // https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
        // CALL rel32
        void *ret = 0;
        char pos;
        char call = 0xe8;
        do {
            pos = *p;
            if (pos == call) {
                ret = p + 5 + (*(int *)(p + 1));
            }

        } while (!ret && (++p));

        return ret;
    }

protected:
    std::map<char *, Wrapper *> m_wrappers;
};

}

#endif   // STUBEXT_H
