// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: MIT

#include "stub-shadow.h"

namespace stub_ext {

WrapperMap stub_wrappers;

Wrapper::Wrapper()
{

}

Wrapper::~Wrapper()
{

}

void freeWrapper(Wrapper *wrapper)
{
    if (!wrapper)
        return;

    for (auto iter = stub_wrappers.begin(); iter != stub_wrappers.end();) {
        if (iter->second == wrapper)
            iter = stub_wrappers.erase(iter);
        else
            ++iter;
    }

    delete wrapper;
}
}
