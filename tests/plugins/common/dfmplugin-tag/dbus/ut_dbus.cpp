// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "dfmplugin_tag_global.h"
#include "plugins/common/dfmplugin-tag/dbus/tagdbus.h"
#include "plugins/common/dfmplugin-tag/data/tagdbhandle.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"

#include <gtest/gtest.h>

DPTAG_USE_NAMESPACE

class UT_TagDBus : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    TagDBus tagDbus;
};

TEST_F(UT_TagDBus, Query)
{
    stub_ext::StubExt stub;
    bool in = false;
    stub.set_lamda(&TagDBus::getAllTags, [&in] {
        in = true;
        return QVariantMap();
    });

    tagDbus.Query(static_cast<quint8>(TagActionType::kGetAllTags));
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::getTagsThroughFile, [&in] {
        in = true;
        return QVariantMap();
    });
    tagDbus.Query(static_cast<quint8>(TagActionType::kGetTagsThroughFile), { "test" });
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::getSameTagsOfDiffFiles, [&in] {
        in = true;
        return QVariant();
    });
    tagDbus.Query(static_cast<quint8>(TagActionType::kGetSameTagsOfDiffFiles), { "test" });
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::getFilesThroughTag, [&in] {
        in = true;
        return QVariantMap();
    });
    tagDbus.Query(static_cast<quint8>(TagActionType::kGetFilesThroughTag), { "test" });
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::getTagsColor, [&in] {
        in = true;
        return QVariantMap();
    });
    tagDbus.Query(static_cast<quint8>(TagActionType::kGetTagsColor), { "test" });
    EXPECT_TRUE(in);
}

TEST_F(UT_TagDBus, Insert)
{
    stub_ext::StubExt stub;
    bool in = false;
    stub.set_lamda(&TagDbHandle::addTagProperty, [&in] {
        in = true;
        return in;
    });
    QVariantMap map;
    map.insert("testKey", QVariant("testValue"));
    tagDbus.Insert(static_cast<quint8>(TagActionType::kAddTags), map);
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDbHandle::addTagsForFiles, [&in] {
        in = true;
        return in;
    });
    tagDbus.Insert(static_cast<quint8>(TagActionType::kMakeFilesTags), map);
    EXPECT_TRUE(in);
}

TEST_F(UT_TagDBus, Delete)
{
    stub_ext::StubExt stub;
    bool in = false;
    QVariantMap map;
    map.insert("testKey", QVariant("testValue"));
    stub.set_lamda(&TagDBus::deleteFileTags, [&in] {
        in = true;
        return in;
    });
    tagDbus.Delete(static_cast<quint8>(TagActionType::kRemoveTagsOfFiles), map);
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::deleteTags, [&in] {
        in = true;
        return in;
    });
    tagDbus.Delete(static_cast<quint8>(TagActionType::kDeleteTags), map);
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::deleteFiles, [&in] {
        in = true;
        return in;
    });
    tagDbus.Delete(static_cast<quint8>(TagActionType::kDeleteFiles), map);
    EXPECT_TRUE(in);
}

TEST_F(UT_TagDBus, Update)
{
    stub_ext::StubExt stub;
    bool in = false;
    QVariantMap map;
    map.insert("testKey", QVariant("testValue"));
    stub.set_lamda(&TagDBus::changeTagColors, [&in] {
        in = true;
        return in;
    });
    tagDbus.Update(static_cast<quint8>(TagActionType::kChangeTagsColor), map);
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::changeTagNamesWithFiles, [&in] {
        in = true;
        return in;
    });
    tagDbus.Update(static_cast<quint8>(TagActionType::kChangeTagsNameWithFiles), map);
    EXPECT_TRUE(in);

    in = false;
    stub.set_lamda(&TagDBus::changeFilePaths, [&in] {
        in = true;
        return in;
    });
    tagDbus.Update(static_cast<quint8>(TagActionType::kChangeFilesPaths), map);
    EXPECT_TRUE(in);
}

TEST_F(UT_TagDBus, CanTagFile)
{
    stub_ext::StubExt stub;
    bool in = false;


    stub.set_lamda((bool(TagManager::*)(const QUrl&) const)ADDR(TagManager, canTagFile), [&in]() {
        in = true;
        return in;
    });
    tagDbus.CanTagFile({});
    EXPECT_TRUE(in);
}
