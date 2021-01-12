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

#include "filemanagerlibrary.h"
#include <QLibrary>
#include <QDebug>

FileManagerLibrary::FileManagerLibrary()
{
    loadLibrary(LibraryName());
}

void FileManagerLibrary::loadLibrary(const QString &fileName)
{
     QLibrary lib(fileName);
     bool isLoaded = lib.load();
     if (isLoaded){
         m_auto_add_rightmenu = (func_auto_add_rightmenu) lib.resolve("auto_add_rightmenu");
         m_auto_operation = (func_auto_operation) lib.resolve("auto_operation");
         m_auto_add_emblem = (func_auto_add_emblem) lib.resolve("auto_add_emblem");
         m_InitLLSInfo = (func_InitLLSInfo) lib.resolve("InitLLSInfo");
         m_free_memory = (func_free_memory) lib.resolve("free_memory");

         if (m_auto_add_rightmenu && m_auto_operation && m_auto_add_emblem && m_InitLLSInfo && m_free_memory){
             m_isCompletion = true;
         }
     }else{
        qDebug() << "load library " << LibraryName() << "failed: " << lib.errorString();
     }
}

FileManagerLibrary::func_auto_add_rightmenu FileManagerLibrary::auto_add_rightmenu() const
{
    return m_auto_add_rightmenu;
}

void FileManagerLibrary::setAuto_add_rightmenu(const func_auto_add_rightmenu &auto_add_rightmenu)
{
    m_auto_add_rightmenu = auto_add_rightmenu;
}

FileManagerLibrary::func_auto_operation FileManagerLibrary::auto_operation() const
{
    return m_auto_operation;
}

void FileManagerLibrary::setAuto_operation(const func_auto_operation &auto_operation)
{
    m_auto_operation = auto_operation;
}

FileManagerLibrary::func_auto_add_emblem FileManagerLibrary::auto_add_emblem() const
{
    return m_auto_add_emblem;
}

void FileManagerLibrary::setAuto_add_emblem(const func_auto_add_emblem &auto_add_emblem)
{
    m_auto_add_emblem = auto_add_emblem;
}

FileManagerLibrary::func_InitLLSInfo FileManagerLibrary::InitLLSInfo() const
{
    return m_InitLLSInfo;
}

void FileManagerLibrary::setInitLLSInfo(const func_InitLLSInfo &InitLLSInfo)
{
    m_InitLLSInfo = InitLLSInfo;
}

FileManagerLibrary::func_free_memory FileManagerLibrary::free_memory() const
{
    return m_free_memory;
}

void FileManagerLibrary::setFree_memory(const func_free_memory &free_memory)
{
    m_free_memory = free_memory;
}

bool FileManagerLibrary::isCompletion() const
{
    return m_isCompletion;
}

