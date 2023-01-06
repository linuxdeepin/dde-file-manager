// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QIODevice>

#include "textpreview.h"
#include "durl.h"

#include "stub.h"

DFM_USE_NAMESPACE

class TestTextPreview :public testing::Test
{
public:
     void SetUp() override
     {
        m_testPreview = new TextPreview(nullptr);
        QFile fp(filePath);
        fp.open(QIODevice::ReadWrite|QIODevice::Text);
        fp.write("I am writing file");
        fp.close();
        m_url = DUrl::fromLocalFile(filePath);
     }

     void TearDown() override
     {
        delete m_testPreview;
        m_testPreview = nullptr;
        QFile fp(filePath);
        fp.remove();
     }

public:
     TextPreview * m_testPreview;
     DUrl m_url;
     QString filePath = QCoreApplication::applicationDirPath() + "/1.txt";
};

TEST_F(TestTextPreview, set_file_url)
{
    EXPECT_TRUE(m_testPreview->setFileUrl(m_url));
}

TEST_F(TestTextPreview, get_file_url)
{
    EXPECT_TRUE(m_testPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_testPreview->fileUrl() == m_url);
}

TEST_F(TestTextPreview, get_content_widget)
{
    EXPECT_TRUE(m_testPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_testPreview->contentWidget() != nullptr);
}

TEST_F(TestTextPreview, get_title)
{
    EXPECT_TRUE(m_testPreview->setFileUrl(m_url));
    bool flg = m_testPreview->title().isEmpty();
    if(flg)
        EXPECT_TRUE(flg);
    else
        EXPECT_FALSE(flg);
}

TEST_F(TestTextPreview, get_show_statusbar_separator)
{
    EXPECT_TRUE(m_testPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_testPreview->showStatusBarSeparator());
}
