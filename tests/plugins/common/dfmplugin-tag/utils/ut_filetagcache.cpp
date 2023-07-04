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

protected:
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
    EXPECT_TRUE(ins->getTagsByFiles({ QString("tag") }).contains(QString("tag1")));
}

TEST_F(FileTagCacheTest, untaggeFiles)
{
    QVariantMap map;
    map["tag"] = QString("tag1");
    ins->untaggeFiles(map);
    EXPECT_TRUE(!ins->getTagsByFiles({ QString("tag") }).contains(QString("tag1")));
}

TEST_F(FileTagCacheTest, onTagAdded)
{
    bool isRun = false;
    stub.set_lamda(&FileTagCache::addTags, [&isRun]() {
        isRun = true;
    });
    FileTagCacheController::instance().cacheWorker->onTagAdded(QVariantMap());
    EXPECT_TRUE(isRun);
}

TEST_F(FileTagCacheTest, onTagsNameChanged)
{
    bool isRun = false;
    stub.set_lamda(&FileTagCache::changeTagName, []() {});
    stub.set_lamda(&FileTagCache::changeFilesTagName, [&isRun]() {
        isRun = true;
    });
    QVariantMap map;
    map["tag"] = QString("tag1");
    FileTagCacheController::instance().cacheWorker->onTagsNameChanged(map);
    EXPECT_TRUE(isRun);
}

TEST_F(FileTagCacheTest, onTagDeleted)
{
    bool isRun = false;
    stub.set_lamda(&FileTagCache::deleteTags, [&isRun]() {
        isRun = true;
    });
    FileTagCacheController::instance().cacheWorker->onTagDeleted(QVariant());
    EXPECT_TRUE(isRun);
}

TEST_F(FileTagCacheTest, onTagsColorChanged)
{
    bool isRun = false;
    stub.set_lamda(&FileTagCache::changeTagColor, [&isRun]() {
        isRun = true;
    });
    FileTagCacheController::instance().cacheWorker->onTagsColorChanged(QVariantMap());
    EXPECT_TRUE(isRun);
}

TEST_F(FileTagCacheTest, onFilesTagged)
{
    bool isRun = false;
    stub.set_lamda(&FileTagCache::taggeFiles, [&isRun]() {
        isRun = true;
    });
    FileTagCacheController::instance().cacheWorker->onFilesTagged(QVariantMap());
    EXPECT_TRUE(isRun);
}

TEST_F(FileTagCacheTest, onFilesUntagged)
{
    bool isRun = false;
    stub.set_lamda(&FileTagCache::untaggeFiles, [&isRun]() {
        isRun = true;
    });
    FileTagCacheController::instance().cacheWorker->onFilesUntagged(QVariantMap());
    EXPECT_TRUE(isRun);
}
