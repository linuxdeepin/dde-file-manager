/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "stubext.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/iterator/smbshareiterator.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/private/smbshareiterator_p.h"

#include <QUrl>

#include <dfm-io/local/dlocalenumerator.h>

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
