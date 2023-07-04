// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "events/tageventreceiver.h"
#include "utils/tagmanager.h"
#include <dfm-base/base/application/application.h>
#include <dfm-framework/event/event.h>
#include <gtest/gtest.h>

DPF_USE_NAMESPACE
using namespace dfmplugin_tag;
Q_DECLARE_METATYPE(QDir::Filters);
class TagEventReceiverTest : public testing::Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TagEventReceiverTest, handleFileCutResult)
{
    bool isRun = false;
    stub.set_lamda(&TagManager::removeTagsOfFiles, []() { return true; });
    auto func = static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() -> bool { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagManager::addTagsForFiles, []() { return true; });

    stub.set_lamda(&TagManager::getTagsByUrls, [&isRun]() {
        __DBG_STUB_INVOKE__
        isRun = true;
        return QStringList() << "red";
    });
    TagEventReceiver::instance()->handleFileCutResult(QList<QUrl>() << QUrl("/"), QList<QUrl>(), true, QString());
    TagEventReceiver::instance()->handleFileCutResult(QList<QUrl>() << QUrl("/"), QList<QUrl>() << QUrl("/"), true, QString());
    EXPECT_TRUE(isRun);
}

TEST_F(TagEventReceiverTest, handleHideFilesResult)
{
    bool isRun = false;
    stub.set_lamda(&TagManager::hideFiles, []() {});

    stub.set_lamda(&TagManager::getTagsByUrls, [&isRun]() {
        __DBG_STUB_INVOKE__
        isRun = true;
        return QStringList() << "red";
    });
    TagEventReceiver::instance()->handleHideFilesResult(1, QList<QUrl>() << QUrl("/"), true);
    EXPECT_TRUE(isRun);
}

TEST_F(TagEventReceiverTest, handleFileRemoveResult)
{
    bool isRun = false;
    stub.set_lamda(&TagManager::removeTagsOfFiles, []() { return true; });

    stub.set_lamda(&TagManager::getTagsByUrls, [&isRun]() {
        __DBG_STUB_INVOKE__
        isRun = true;
        return QStringList() << "red";
    });
    TagEventReceiver::instance()->handleWindowUrlChanged(1, QUrl("tag:/"));
    TagEventReceiver::instance()->handleFileRemoveResult(QList<QUrl>() << QUrl("/"), true, QString());
    EXPECT_TRUE(isRun);
}

TEST_F(TagEventReceiverTest, handleFileRenameResult)
{
    bool isRun = false;
    stub.set_lamda(&TagManager::removeTagsOfFiles, []() { return true; });
    stub.set_lamda(&TagManager::addTagsForFiles, []() { return true; });

    stub.set_lamda(&TagManager::getTagsByUrls, [&isRun]() {
        __DBG_STUB_INVOKE__
        isRun = true;
        return QStringList() << "red";
    });
    QMap<QUrl, QUrl> map;
    map.insert(QUrl("/"), QUrl("/"));
    TagEventReceiver::instance()->handleFileRenameResult(1, map, true, QString());
    EXPECT_TRUE(isRun);
}

TEST_F(TagEventReceiverTest, handleGetTags)
{
    stub.set_lamda(&TagManager::getTagsByUrls, []() {
        __DBG_STUB_INVOKE__
        return QStringList() << "red";
    });
    EXPECT_TRUE(!TagEventReceiver::instance()->handleGetTags(QUrl("/")).isEmpty());
}

TEST_F(TagEventReceiverTest, handleSidebarOrderChanged)
{
    bool isRun = false;
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, quint64, const QString &);
    auto push = static_cast<Push>(&dpf::EventChannelManager::push);
    stub.set_lamda(push, [&isRun] {
        isRun = true;
        return QVariant();
    });
    TagEventReceiver::instance()->handleSidebarOrderChanged(1, QString("Tag"));
    EXPECT_TRUE(isRun);
}
