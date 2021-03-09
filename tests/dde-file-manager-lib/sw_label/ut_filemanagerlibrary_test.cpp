/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#define private public
#define protected public

#include "sw_label/filemanagerlibrary.h"
#include "sw_label/filemanagerlibrary.cpp"

using namespace testing;
namespace  {
    class FileManagerLibraryTest : public Test
    {
    public:
        FileManagerLibraryTest():Test()
        {
            p_library = FileManagerLibrary::instance();
        }

        virtual void SetUp() override {
        }

        virtual void TearDown() override {
        }

        FileManagerLibrary * p_library;
    };
}

TEST_F(FileManagerLibraryTest, get_library_name)
{
    QString result = p_library->LibraryName();
    QString expectResult("/usr/lib/sw_64-linux-gnu/dde-file-manager/libfilemanager.so");

    EXPECT_EQ(result, expectResult);
}

TEST_F(FileManagerLibraryTest, load_library)
{
    QString name("/usr/lib/sw_64-linux-gnu/dde-file-manager/libfilemanager.so");
    p_library->loadLibrary(name);

    name = "libz";
    p_library->loadLibrary(name);
}

TEST_F(FileManagerLibraryTest, auto_add_rightmenu)
{
    FileManagerLibrary::func_auto_add_rightmenu result = p_library->auto_add_rightmenu();
    EXPECT_EQ(result, p_library->m_auto_add_rightmenu);
}

TEST_F(FileManagerLibraryTest, set_auto_rightmenu)
{
    FileManagerLibrary::func_auto_add_rightmenu result = p_library->auto_add_rightmenu();
    p_library->setAuto_add_rightmenu(nullptr);
    EXPECT_EQ(nullptr, p_library->m_auto_add_rightmenu);
    p_library->setAuto_add_rightmenu(result);
}

TEST_F(FileManagerLibraryTest, auto_operation)
{
    FileManagerLibrary::func_auto_operation result = p_library->auto_operation();
    EXPECT_EQ(result, p_library->m_auto_operation);
}

TEST_F(FileManagerLibraryTest, set_auto_operation)
{
    FileManagerLibrary::func_auto_operation result = p_library->auto_operation();
    p_library->setAuto_operation(nullptr);
    EXPECT_EQ(nullptr, p_library->m_auto_operation);
    p_library->setAuto_operation(result);
}

TEST_F(FileManagerLibraryTest, auto_add_emblem)
{
    FileManagerLibrary::func_auto_add_emblem result = p_library->auto_add_emblem();
    EXPECT_EQ(result, p_library->m_auto_add_emblem);
}

TEST_F(FileManagerLibraryTest, set_auto_emblem)
{
    FileManagerLibrary::func_auto_add_emblem result = p_library->auto_add_emblem();
    p_library->setAuto_add_emblem(nullptr);
    EXPECT_EQ(nullptr, p_library->m_auto_add_emblem);
    p_library->setAuto_add_emblem(result);
}

TEST_F(FileManagerLibraryTest, init_LLS_info)
{
    FileManagerLibrary::func_InitLLSInfo result = p_library->InitLLSInfo();
    EXPECT_EQ(result, p_library->m_InitLLSInfo);
}

TEST_F(FileManagerLibraryTest, set_init_info)
{
    FileManagerLibrary::func_InitLLSInfo result = p_library->InitLLSInfo();
    p_library->setInitLLSInfo(nullptr);
    EXPECT_EQ(nullptr, p_library->m_InitLLSInfo);
    p_library->setInitLLSInfo(result);
}

TEST_F(FileManagerLibraryTest, free_memory)
{
    FileManagerLibrary::func_free_memory result = p_library->free_memory();
    EXPECT_EQ(result, p_library->m_free_memory);
}

TEST_F(FileManagerLibraryTest, set_free_memory)
{
    FileManagerLibrary::func_free_memory result = p_library->free_memory();
    p_library->setFree_memory(nullptr);
    EXPECT_EQ(nullptr, p_library->m_free_memory);
    p_library->setFree_memory(result);
}

TEST_F(FileManagerLibraryTest, get_is_completion)
{
    bool result = p_library->isCompletion();
    EXPECT_EQ(result, p_library->m_isCompletion);
}
