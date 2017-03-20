/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once
#include <memory>

using namespace std;

template <class T>
class Singleton
{
public:
    static inline T *instance() {
        static T*  _instance = new T;
        return _instance;
    }

protected:
    Singleton(void) {}
    ~Singleton(void) {}
    Singleton(const Singleton &) {}
    Singleton &operator= (const Singleton &) {}
};


