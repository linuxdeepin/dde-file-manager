/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#include <QFile>

#include <gtest/gtest.h>

#include "taghandle.h"

namespace  {
class TestTagHandle : public testing::Test
{
public:
    void SetUp() override
    {
        m_tagHandle = new TagHandle(nullptr);
        QFile file("./test.txt");
        file.open(QIODevice::ReadWrite|QIODevice::Text);
        file.write("I am writing file");
        file.close();
        filelist << "./test.txt";
    }

    void TearDown() override
    {
        delete m_tagHandle;
        m_tagHandle = nullptr;
        QFile fp("./test.txt");
        fp.remove();
    }

public:
    TagHandle *m_tagHandle;
    QByteArrayList filelist;
};

TEST_F(TestTagHandle, on_file_create)
{
    m_tagHandle->onFileCreate(filelist);
}

TEST_F(TestTagHandle, on_file_delete)
{
    m_tagHandle->onFileDelete(filelist);
}

TEST_F(TestTagHandle, on_file_rename)
{
    QList<QPair<QByteArray, QByteArray>> files;
    QPair<QByteArray, QByteArray> temp("name", "ttttt");
    files << temp;
    m_tagHandle->onFileRename(files);
}
}

