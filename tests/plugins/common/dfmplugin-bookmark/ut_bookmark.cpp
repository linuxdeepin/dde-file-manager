// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "bookmark.h"
#include "controller/bookmarkmanager.h"
#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>
#include "qdbusabstractinterface.h"
#include <QDBusPendingCall>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_bookmark;
using namespace dpf;
using namespace QDBus;
Q_DECLARE_METATYPE(CustomViewExtensionView)

class BookMarkTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    BookMark ins;
};

TEST_F(BookMarkTest, Start)
{
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, CustomViewExtensionView, int &);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, CustomViewExtensionView, QString &, int &);
    typedef QVariant (EventChannelManager::*Push3)(const QString &, const QString &, QString, QStringList &);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    auto push3 = static_cast<Push3>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push3, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_TRUE(ins.start());
    ins.initialize();
}

TEST_F(BookMarkTest, onWindowOpened)
{
    bool isRun = false;
    FileManagerWindow w(QUrl::fromLocalFile("/home"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&w, &isRun] {
        isRun = true;
        return &w;
    });
    ins.onWindowOpened(1);
    EXPECT_TRUE(isRun);
}

TEST_F(BookMarkTest, onSideBarInstallFinished)
{
    bool isRun = false;
    stub.set_lamda(&BookMarkManager::addQuickAccessItemsFromConfig, [&isRun] {  isRun = true;__DBG_STUB_INVOKE__ });
    ins.onSideBarInstallFinished();
    EXPECT_TRUE(isRun);
}

TEST_F(BookMarkTest, onMenuSceneAdded)
{
    stub.set_lamda(&BookMarkManager::addQuickAccessItemsFromConfig, [] { __DBG_STUB_INVOKE__ });
    ins.menuScenes.insert("hello");
    EXPECT_NO_FATAL_FAILURE(ins.onMenuSceneAdded("hello"));
}
