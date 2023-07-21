// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/filetagcache.h"
#include "data/tagproxyhandle.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/clipboard.h>

#include <gtest/gtest.h>

#include <DCrumbEdit>
#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class FileTagCacheTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = &FileTagCache::instance();
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    FileTagCache *ins;
};

TEST_F(FileTagCacheTest, loadFileTagsFromDatabase)
{
    stub.set_lamda(&TagProxyHandle::getAllTags, []() {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["test"] = QColor("red");
        return map;
    });
    stub.set_lamda(&TagProxyHandle::getAllFileWithTags, []() {
        __DBG_STUB_INVOKE__
        QVariantHash hash;
        hash["test"] = QColor("red");
        return hash;
    });
    stub.set_lamda(&TagProxyHandle::isValid, []() { __DBG_STUB_INVOKE__ return true; });
    ins->loadFileTagsFromDatabase();
    EXPECT_TRUE(ins->getTagsColor(QStringList() << QString("test")).value("test") == QColor("red"));
}

TEST_F(FileTagCacheTest, addTags)
{
    QVariantMap map;
    map["test2"] = QColor("green");
    ins->addTags(map);
    EXPECT_TRUE(!ins->getTagsColor(QStringList() << QString("test2")).isEmpty());
    ins->deleteTags(QStringList() << QString("test2"));
    EXPECT_TRUE(ins->getTagsColor(QStringList() << QString("test2")).isEmpty());
}

TEST_F(FileTagCacheTest, changeTagColor)
{
    QVariantMap map;
    map["test"] = QColor("green");
    ins->changeTagColor(map);
    EXPECT_TRUE(ins->getTagsColor(QStringList() << QString("test")).value("test") == QColor("green"));
}

TEST_F(FileTagCacheTest, changeTagName)
{
    QVariantMap map;
    map["test"] = QString("test3");
    ins->changeTagName(map);
    EXPECT_TRUE(ins->getTagsColor(QStringList() << QString("test3")).value("test3") == QColor("green"));
}

TEST_F(FileTagCacheTest, taggeFiles)
{
    QVariantMap map;
    map["tag"] = QString("tag1");
    ins->taggeFiles(map);
    EXPECT_TRUE(ins->getCacheFileTags(QString("tag")).contains(QString("tag1")));
}

TEST_F(FileTagCacheTest, untaggeFiles)
{
    QVariantMap map;
    map["tag"] = QString("tag1");
    ins->untaggeFiles(map);
    EXPECT_TRUE(!ins->getCacheFileTags(QString("tag")).contains(QString("tag1")));
}
