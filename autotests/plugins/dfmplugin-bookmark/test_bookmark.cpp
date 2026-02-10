// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "bookmark.h"
#include "controller/bookmarkmanager.h"
#include "controller/defaultitemmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/dpf.h>

#include <QSet>
#include <QString>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_bookmark;
using namespace dpf;
Q_DECLARE_METATYPE(CustomViewExtensionView)

class UT_BookMark : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        plugin = new BookMark();
    }

    virtual void TearDown() override
    {
        delete plugin;
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    BookMark *plugin = nullptr;
};

TEST_F(UT_BookMark, Initialize)
{
    EXPECT_NO_FATAL_FAILURE(plugin->initialize());
}

TEST_F(UT_BookMark, Start)
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
    EXPECT_TRUE(plugin->start());
}

TEST_F(UT_BookMark, OnWindowOpened)
{
    bool isRun = false;
    FileManagerWindow w(QUrl::fromLocalFile("/home"));
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&w, &isRun] {
        isRun = true;
        return &w;
    });
    plugin->onWindowOpened(1);
    EXPECT_TRUE(isRun);
}

TEST_F(UT_BookMark, OnSideBarInstallFinished)
{
    bool isRun = false;
    stub.set_lamda(&BookMarkManager::addQuickAccessItemsFromConfig, [&isRun] {  isRun = true;__DBG_STUB_INVOKE__ });
    plugin->onSideBarInstallFinished();
    EXPECT_TRUE(isRun);
}

TEST_F(UT_BookMark, OnMenuSceneAdded)
{
    stub.set_lamda(&BookMarkManager::addQuickAccessItemsFromConfig, [] { __DBG_STUB_INVOKE__ });
    plugin->menuScenes.insert("hello");
    EXPECT_NO_FATAL_FAILURE(plugin->onMenuSceneAdded("hello"));
}
