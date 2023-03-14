// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/iterator/smbshareiterator.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/private/smbshareiterator_p.h"

#include <QUrl>

#include <dfm-io/denumerator.h>

#include <gtest/gtest.h>

using namespace dfmplugin_smbbrowser;

class UT_SmbShareIterator : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        iter = new SmbShareIterator(QUrl::fromLocalFile("/"), {}, QDir::AllEntries);
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete iter;
        iter = nullptr;
    }

private:
    stub_ext::StubExt stub;
    SmbShareIterator *iter { nullptr };
};

TEST_F(UT_SmbShareIterator, Next)
{
    iter->d->rootUrl.setPort(448);
    EXPECT_NO_FATAL_FAILURE(iter->next());
    EXPECT_FALSE(iter->next().isValid());
}

TEST_F(UT_SmbShareIterator, HasNext)
{
    EXPECT_NO_FATAL_FAILURE(iter->hasNext());
    EXPECT_TRUE(iter->hasNext());
}

TEST_F(UT_SmbShareIterator, FileName)
{
    EXPECT_NO_FATAL_FAILURE(iter->fileName());
    EXPECT_TRUE(iter->fileName() == "");
}

TEST_F(UT_SmbShareIterator, FileUrl)
{
    EXPECT_NO_FATAL_FAILURE(iter->fileUrl());
    EXPECT_FALSE(iter->fileUrl().isValid());
}

TEST_F(UT_SmbShareIterator, FileInfo)
{
    EXPECT_NO_FATAL_FAILURE(iter->fileInfo());
    EXPECT_TRUE(iter->fileInfo() == nullptr);
}

TEST_F(UT_SmbShareIterator, Url)
{
    EXPECT_NO_FATAL_FAILURE(iter->url());
    EXPECT_FALSE(iter->url().isValid());
}
