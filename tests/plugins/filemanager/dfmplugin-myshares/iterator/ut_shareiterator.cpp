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

#include "plugins/filemanager/dfmplugin-myshares/iterator/shareiterator.h"
#include "plugins/filemanager/dfmplugin-myshares/private/shareiterator_p.h"
#include "dfm-base/base/schemefactory.h"

#include <gtest/gtest.h>

using namespace dfmplugin_myshares;
class UT_ShareIterator : public testing::Test
{
public:
    virtual void SetUp() override
    {
        it = new ShareIterator(QUrl("usershare:///"), {}, QDir::AllDirs, QDirIterator::NoIteratorFlags);
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete it;
        it = nullptr;
    }

    stub_ext::StubExt stub;
    ShareIterator *it { nullptr };
};

DFMBASE_USE_NAMESPACE

TEST_F(UT_ShareIterator, Next)
{
    EXPECT_FALSE(it->next().isValid());

    it->d->shares.append(QVariantMap());
    EXPECT_TRUE(it->next().path().isEmpty());

    it->d->shares.append({ { "path", "/hello/world" } });
    EXPECT_TRUE(it->next().path() == "/hello/world");
}

TEST_F(UT_ShareIterator, HasNext)
{
    it->d->shares.clear();
    EXPECT_FALSE(it->hasNext());
    it->d->shares.append(QVariantMap());
    EXPECT_TRUE(it->hasNext());
}

TEST_F(UT_ShareIterator, FileName)
{
    it->d->currentInfo.insert("shareName", "Test");
    EXPECT_TRUE(it->fileName() == "Test");
}

TEST_F(UT_ShareIterator, FileUrl)
{
    it->d->currentInfo = { { "", "" } };
    EXPECT_TRUE(it->fileUrl().path().isEmpty());

    it->d->currentInfo = { { "path", "/hello/world" } };
    EXPECT_TRUE(it->fileUrl().isValid());
    EXPECT_EQ(it->fileUrl().path(), QString("/hello/world"));
}

TEST_F(UT_ShareIterator, FileInfo)
{
    stub.set_lamda(InfoFactory::create<AbstractFileInfo>, [] { __DBG_STUB_INVOKE__ return nullptr; });
    EXPECT_TRUE(it->fileInfo() == nullptr);
}

TEST_F(UT_ShareIterator, Url)
{
    EXPECT_TRUE(it->url().path() == "/");
    EXPECT_TRUE(it->url().scheme() == "usershare");
}
