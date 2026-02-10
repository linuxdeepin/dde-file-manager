// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "files/tagdiriterator.h"
#include "files/private/tagdiriterator_p.h"
#include "utils/tagmanager.h"
#include "utils/taghelper.h"

#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QDir>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class TagDirIteratorTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {

        stub.set_lamda(&TagManager::getFilesByTag, []() {
            __DBG_STUB_INVOKE__
            return QStringList() << QString("红色");
        });
        iter = new TagDirIterator(QUrl::fromLocalFile("/home/"), {}, QDir::AllEntries);
        d = iter->d.data();
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete iter;
        iter = nullptr;
    }

private:
    stub_ext::StubExt stub;
    TagDirIterator *iter { nullptr };
    TagDirIteratorPrivate *d { nullptr };
};

TEST_F(TagDirIteratorTest, Next)
{
    EXPECT_FALSE(iter->next().isValid());
}

TEST_F(TagDirIteratorTest, HasNext)
{
    EXPECT_FALSE(iter->hasNext());
}

TEST_F(TagDirIteratorTest, FileName)
{
    EXPECT_TRUE(iter->fileName() == "");
}

TEST_F(TagDirIteratorTest, FileUrl)
{
    EXPECT_FALSE(iter->fileUrl().isValid());
}

TEST_F(TagDirIteratorTest, FileInfo)
{
    EXPECT_TRUE(iter->fileInfo() == nullptr);
}

TEST_F(TagDirIteratorTest, Url)
{
    EXPECT_TRUE(iter->url().isValid());
}

TEST_F(TagDirIteratorTest, loadTagsUrls)
{
    stub.set_lamda(&TagManager::getAllTags, []() {
        __DBG_STUB_INVOKE__
        QMap<QString, QColor> map;
        map.insert("红色", QColor("red"));
        return map;
    });
    d->loadTagsUrls(TagHelper::rootUrl());
}
