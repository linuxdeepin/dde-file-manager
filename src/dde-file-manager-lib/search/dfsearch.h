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
#ifndef __DFSEARCH_H__
#define __DFSEARCH_H__

#include <QObject>

extern "C"
{
#include "fsearch.h"
}

class DFSearch
{
    typedef void (*callbackFunc)(void *, void *);
public:
    DFSearch(const QString &searchPath, void *parent = nullptr);

    ~DFSearch();

    DFSearch(DFSearch &) = delete;
    DFSearch &operator=(DFSearch &) = delete;

    void searchByKeyWord(const QString &key, void (*callback)(void *, void *));

    void stop();

    static bool isSupportFSearch(const QString &path);

private:
    static void fsearch_application_window_update_results(void *data, void *sender);

    static gboolean update_model_cb(gpointer user_data, gpointer sender);
private:
    FsearchApplication *app = nullptr;
    callbackFunc cbFunc = nullptr;
    GPtrArray *results = nullptr;
    uint  num_results = 0;
    void *caller = nullptr;
    /**
     * @brief state 搜索状态值，用于打断后续遍历文件夹
     */
    bool state = true;
    QString pathForSearching;
};

#endif
