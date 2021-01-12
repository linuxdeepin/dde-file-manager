/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FILEMANAGERLIBRARY_H
#define FILEMANAGERLIBRARY_H

#include <QString>

class FileManagerLibrary
{
public:

    static FileManagerLibrary* instance(){
          static FileManagerLibrary instance;
          return &instance;
    }

    inline static QString LibraryName(){
        return "/usr/lib/sw_64-linux-gnu/dde-file-manager/libfilemanager.so";
    }

    void loadLibrary(const QString& fileName);

    typedef char *(*func_auto_add_rightmenu)(char *path);
    typedef void *(*func_auto_operation)(char *path,char *strid);
    typedef char *(*func_auto_add_emblem)(char *path);
    typedef void (*func_InitLLSInfo)();
    typedef void (*func_free_memory)(void*);

    func_auto_add_rightmenu auto_add_rightmenu() const;
    void setAuto_add_rightmenu(const func_auto_add_rightmenu &auto_add_rightmenu);

    func_auto_operation auto_operation() const;
    void setAuto_operation(const func_auto_operation &auto_operation);

    func_auto_add_emblem auto_add_emblem() const;
    void setAuto_add_emblem(const func_auto_add_emblem &auto_add_emblem);

    func_InitLLSInfo InitLLSInfo() const;
    void setInitLLSInfo(const func_InitLLSInfo &InitLLSInfo);

    func_free_memory free_memory() const;
    void setFree_memory(const func_free_memory &free_memory);

    bool isCompletion() const;

private:
    FileManagerLibrary();
    FileManagerLibrary(const FileManagerLibrary &);
    FileManagerLibrary & operator = (const FileManagerLibrary &);

    bool m_isCompletion = false;
    func_auto_add_rightmenu m_auto_add_rightmenu = NULL;
    func_auto_operation m_auto_operation = NULL;
    func_auto_add_emblem m_auto_add_emblem = NULL;
    func_InitLLSInfo m_InitLLSInfo = NULL;
    func_free_memory m_free_memory = NULL;
};

#endif // FILEMANAGERLIBRARY_H
