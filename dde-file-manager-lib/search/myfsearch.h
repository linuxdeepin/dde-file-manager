/*
* Copyright (C) 2019 ~ 2020 Uniontech Technology Co., Ltd.
*
* Author:     hujianzhong <hujianhzong@uniontech.com>
*
* Maintainer: hujianzhong <hujianhzong@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MYFSEARCH_H__
#define __MYFSEARCH_H__
extern "C"
{
#include "database_search.h"
    void fsearch_Init(const char*path);
//    void fsearch_DeInit(char*path);
    void fsearch_Find(const char*text,void (*callback)(void *));
//    void fsearch_Find(const char*text);
    void fsearch_Close(void);
    void fsearch_UpdateDb(char*path);
    GPtrArray * get_Result(void);
    unsigned int get_ResultLen(void);
}
#endif
