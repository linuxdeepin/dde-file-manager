// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "data/tagproxyhandle.h"
#include "data/private/tagproxyhandle_p.h"
#include "tagmanager_interface.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>
#include <QRect>
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_tag;

TEST(UT_TagProxyHandle, connectToService)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&TagProxyHandlePrivate::isDBusRuning, []() { return true; });

    EXPECT_TRUE(TagProxyHandle::instance()->connectToService());
}

TEST(UT_TagProxyHandle, deleteFileTags)
{
    bool isRun { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&OrgDeepinFilemanagerServerTagManagerInterface::Delete, [&isRun]() {
        isRun = true;
        return QDBusPendingReply<bool>();
    });
    QMap<QString, QVariant> fileWithTag;
    fileWithTag["1"] = QVariant(QStringList() << "red");
    stub.set_lamda(&QDBusPendingCall::isValid, []() {
        return true;
    });
    TagProxyHandle::instance()->deleteFileTags(fileWithTag);

    EXPECT_TRUE(isRun);
}

TEST(UT_TagProxyHandle, deleteFiles)
{
    bool isRun { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&OrgDeepinFilemanagerServerTagManagerInterface::Delete, [&isRun]() {
        isRun = true;
        return QDBusPendingReply<bool>();
    });
    QMap<QString, QVariant> fileWithTag;
    fileWithTag["1"] = QVariant(QStringList() << "red");
    stub.set_lamda(&QDBusPendingCall::isValid, []() {
        return true;
    });
    TagProxyHandle::instance()->deleteFiles(fileWithTag);

    EXPECT_TRUE(isRun);
}

TEST(UT_TagProxyHandle, deleteTags)
{
    bool isRun { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&OrgDeepinFilemanagerServerTagManagerInterface::Delete, [&isRun]() {
        isRun = true;
        return QDBusPendingReply<bool>();
    });
    QMap<QString, QVariant> fileWithTag;
    fileWithTag["1"] = QVariant(QStringList() << "red");
    stub.set_lamda(&QDBusPendingCall::isValid, []() {
        return true;
    });
    TagProxyHandle::instance()->deleteTags(fileWithTag);

    EXPECT_TRUE(isRun);
}

TEST(UT_TagProxyHandle, update)
{
    int isRun = 0;
    stub_ext::StubExt stub;
    stub.set_lamda(&OrgDeepinFilemanagerServerTagManagerInterface::Update, [&isRun]() {
        isRun++;
        return QDBusPendingReply<bool>();
    });
    QMap<QString, QVariant> fileWithTag;
    fileWithTag["1"] = QVariant(QStringList() << "red");
    stub.set_lamda(&QDBusPendingCall::isValid, []() {
        return true;
    });
    TagProxyHandle::instance()->changeFilePaths(fileWithTag);
    TagProxyHandle::instance()->changeTagNamesWithFiles(fileWithTag);
    TagProxyHandle::instance()->changeTagsColor(fileWithTag);
    EXPECT_TRUE(isRun == 3);
}

TEST(UT_TagProxyHandle, Insert)
{
    int isRun = 0;
    stub_ext::StubExt stub;
    stub.set_lamda(&OrgDeepinFilemanagerServerTagManagerInterface::Insert, [&isRun]() {
        isRun++;
        return QDBusPendingReply<bool>();
    });
    stub.set_lamda(&QDBusPendingCall::isValid, []() {
        return true;
    });
    QMap<QString, QVariant> fileWithTag;
    fileWithTag["1"] = QVariant(QStringList() << "red");

    TagProxyHandle::instance()->addTags(fileWithTag);
    TagProxyHandle::instance()->addTagsForFiles(fileWithTag);
    EXPECT_TRUE(isRun == 2);
}

TEST(UT_TagProxyHandle, Query)
{
    int isRun = 0;
    stub_ext::StubExt stub;
    auto func = static_cast<QDBusPendingReply<QDBusVariant> (OrgDeepinFilemanagerServerTagManagerInterface::*)(int opt, const QStringList &)>(&OrgDeepinFilemanagerServerTagManagerInterface::Query);
    stub.set_lamda(func, [&isRun]() {
        isRun++;
        return QDBusPendingReply<QDBusVariant>();
    });
    stub.set_lamda(&QDBusPendingCall::isValid, []() {
        return true;
    });
    TagProxyHandle::instance()->getTagsColor(QStringList());
    TagProxyHandle::instance()->getFilesThroughTag(QStringList());
    TagProxyHandle::instance()->getSameTagsOfDiffFiles(QStringList());
    TagProxyHandle::instance()->getTagsThroughFile(QStringList());
    EXPECT_TRUE(isRun == 4);
}

TEST(UT_TagProxyHandle, Query2)
{
    int isRun = 0;
    stub_ext::StubExt stub;
    auto func = static_cast<QDBusPendingReply<QDBusVariant> (OrgDeepinFilemanagerServerTagManagerInterface::*)(int opt)>(&OrgDeepinFilemanagerServerTagManagerInterface::Query);
    stub.set_lamda(func, [&isRun]() {
        isRun++;
        return QDBusPendingReply<QDBusVariant>();
    });
    stub.set_lamda(&QDBusArgument::currentType, []() {
        return QDBusArgument::ElementType::MapType;
    });
    stub.set_lamda(&QDBusPendingCall::isValid, []() {
        return true;
    });

    TagProxyHandle::instance()->getAllFileWithTags();
    TagProxyHandle::instance()->getAllTags();
    EXPECT_TRUE(isRun == 2);
}
