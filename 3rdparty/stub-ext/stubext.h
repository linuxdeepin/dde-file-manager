#ifndef STUBEXT_H
#define STUBEXT_H
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

//需修改Stub的私用成员函数和成员变量为保护类型
#include "stub.h"

#include "stub-shadow.h"

#define VADDR(CLASS_NAME,MEMBER_NAME) (typename stub_ext::VFLocator<decltype(&CLASS_NAME::MEMBER_NAME)>::Func)(&CLASS_NAME::MEMBER_NAME)

namespace stub_ext {

class StubExt : public Stub
{
public:
    StubExt() : Stub() {}

    template<typename T, class Lamda>
    bool set_lamda(T addr, Lamda lamda)
    {
        char *fn = addrof(addr);
        if (m_result.find(fn) != m_result.end())
            reset(addr);

        Wrapper *wrapper = nullptr;
        auto addr_stub = depictShadow(&wrapper,addr,lamda);
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
        for (auto iter = m_wrappers.begin(); iter != m_wrappers.end(); ++iter) {
            freeWrapper(iter->second);
        }
    }
protected:
    std::map<char*, Wrapper*> m_wrappers;
};

}

#endif // STUBEXT_H
