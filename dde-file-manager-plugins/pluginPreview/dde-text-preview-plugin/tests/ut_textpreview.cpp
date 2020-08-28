/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-27

 */


#include <gtest/gtest.h>

#include "textpreview.h"
#include "durl.h"

DFM_BEGIN_NAMESPACE

class TestTextPreview :public testing::Test
{
public:
     void SetUp() override
     {
        m_testPreview = new TextPreview(nullptr);
        DUrl url("file:./../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-text-preview-plugin/tests/test.txt");
        m_url = DUrl("file://" + url.toAbsolutePathUrl().path());
     }

     void TearDown() override
     {
        delete m_testPreview;
         m_testPreview = nullptr;
     }

public:
     TextPreview * m_testPreview;
     DUrl m_url;
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
    EXPECT_TRUE(m_testPreview->contentWidget());
}

TEST_F(TestTextPreview, get_title)
{
    EXPECT_TRUE(m_testPreview->setFileUrl(m_url));
    EXPECT_TRUE(!m_testPreview->title().isEmpty());
}

TEST_F(TestTextPreview, get_show_statusbar_separator)
{
    EXPECT_TRUE(m_testPreview->setFileUrl(m_url));
    EXPECT_TRUE(m_testPreview->showStatusBarSeparator());
}

DFM_END_NAMESPACE
