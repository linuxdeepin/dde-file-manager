/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-28

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

