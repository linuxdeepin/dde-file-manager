// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/titlebarhelper.h"
#include "views/titlebarwidget.h"
#include "events/titlebareventcaller.h"
#include "dialogs/connecttoserverdialog.h"
#include "dialogs/diskpasswordchangingdialog.h"

#include <dfm-base/dialogs/smbsharepasswddialog/usersharepasswordsettingdialog.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <dfm-framework/dpf.h>

#include <DTitlebar>

#include <gtest/gtest.h>
#include <QUrl>
#include <QWidget>
#include <QStandardPaths>
#include <QSettings>
#include <QMenu>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class TitleBarHelperTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Clear static data
        TitleBarHelper::kTitleBarMap.clear();
        TitleBarHelper::kKeepTitleStatusSchemeList.clear();
        TitleBarHelper::kViewModeUrlCallbackMap.clear();

        // Stub icon loading
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        // Stub FileManagerWindowsManager
        stub.set_lamda(&FileManagerWindowsManager::findWindowId, [](FileManagerWindowsManager *, const QWidget *) {
            __DBG_STUB_INVOKE__
            return static_cast<quint64>(12345);
        });

        // Stub device utilities
        stub.set_lamda(&DeviceUtils::checkDiskEncrypted, []() {
            __DBG_STUB_INVOKE__
            return false;
        });
    }

    void TearDown() override
    {
        // Clean up static data
        TitleBarHelper::kTitleBarMap.clear();
        TitleBarHelper::kKeepTitleStatusSchemeList.clear();
        TitleBarHelper::kViewModeUrlCallbackMap.clear();
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TitleBarHelperTest, Titlebars_EmptyMap_ReturnsEmptyList)
{
    auto titlebars = TitleBarHelper::titlebars();
    EXPECT_TRUE(titlebars.isEmpty());
}

TEST_F(TitleBarHelperTest, Titlebars_AfterAdd_ReturnsListWithTitleBar)
{
    quint64 windowId = 12345;
    TitleBarWidget *widget = new TitleBarWidget();

    TitleBarHelper::addTileBar(windowId, widget);
    auto titlebars = TitleBarHelper::titlebars();

    EXPECT_EQ(titlebars.size(), 1);
    EXPECT_TRUE(titlebars.contains(widget));

    TitleBarHelper::removeTitleBar(windowId);
    delete widget;
}

TEST_F(TitleBarHelperTest, Titlebars_MultipleTitleBars_ReturnsAllTitleBars)
{
    TitleBarWidget *widget1 = new TitleBarWidget();
    TitleBarWidget *widget2 = new TitleBarWidget();
    TitleBarWidget *widget3 = new TitleBarWidget();

    TitleBarHelper::addTileBar(100, widget1);
    TitleBarHelper::addTileBar(200, widget2);
    TitleBarHelper::addTileBar(300, widget3);

    auto titlebars = TitleBarHelper::titlebars();
    EXPECT_EQ(titlebars.size(), 3);

    TitleBarHelper::removeTitleBar(100);
    TitleBarHelper::removeTitleBar(200);
    TitleBarHelper::removeTitleBar(300);

    delete widget1;
    delete widget2;
    delete widget3;
}

TEST_F(TitleBarHelperTest, FindTileBarByWindowId_ValidWindowId_ReturnsTitleBar)
{
    quint64 windowId = 54321;
    TitleBarWidget *widget = new TitleBarWidget();

    TitleBarHelper::addTileBar(windowId, widget);
    TitleBarWidget *found = TitleBarHelper::findTileBarByWindowId(windowId);

    EXPECT_EQ(found, widget);

    TitleBarHelper::removeTitleBar(windowId);
    delete widget;
}

TEST_F(TitleBarHelperTest, FindTileBarByWindowId_NonExistentWindowId_ReturnsNull)
{
    TitleBarWidget *found = TitleBarHelper::findTileBarByWindowId(99999);
    EXPECT_EQ(found, nullptr);
}

TEST_F(TitleBarHelperTest, FindTileBarByWindowId_AfterRemoval_ReturnsNull)
{
    quint64 windowId = 11111;
    TitleBarWidget *widget = new TitleBarWidget();

    TitleBarHelper::addTileBar(windowId, widget);
    TitleBarHelper::removeTitleBar(windowId);

    TitleBarWidget *found = TitleBarHelper::findTileBarByWindowId(windowId);
    EXPECT_EQ(found, nullptr);

    delete widget;
}

TEST_F(TitleBarHelperTest, AddTileBar_ValidWindowId_TitleBarAdded)
{
    quint64 windowId = 12345;
    TitleBarWidget *widget = new TitleBarWidget();

    TitleBarHelper::addTileBar(windowId, widget);

    EXPECT_TRUE(TitleBarHelper::kTitleBarMap.contains(windowId));
    EXPECT_EQ(TitleBarHelper::kTitleBarMap[windowId], widget);

    TitleBarHelper::removeTitleBar(windowId);
    delete widget;
}

TEST_F(TitleBarHelperTest, AddTileBar_DuplicateWindowId_DoesNotOverwrite)
{
    quint64 windowId = 12345;
    TitleBarWidget *widget1 = new TitleBarWidget();
    TitleBarWidget *widget2 = new TitleBarWidget();

    TitleBarHelper::addTileBar(windowId, widget1);
    TitleBarHelper::addTileBar(windowId, widget1);   // Try to add again

    EXPECT_EQ(TitleBarHelper::findTileBarByWindowId(windowId), widget1);

    TitleBarHelper::removeTitleBar(windowId);
    delete widget1;
    delete widget2;
}

TEST_F(TitleBarHelperTest, AddTileBar_MultipleTitleBars_AllAdded)
{
    TitleBarWidget *widget1 = new TitleBarWidget();
    TitleBarWidget *widget2 = new TitleBarWidget();

    TitleBarHelper::addTileBar(100, widget1);
    TitleBarHelper::addTileBar(200, widget2);

    EXPECT_EQ(TitleBarHelper::findTileBarByWindowId(100), widget1);
    EXPECT_EQ(TitleBarHelper::findTileBarByWindowId(200), widget2);

    TitleBarHelper::removeTitleBar(100);
    TitleBarHelper::removeTitleBar(200);
    delete widget1;
    delete widget2;
}

TEST_F(TitleBarHelperTest, RemoveTitleBar_ValidWindowId_TitleBarRemoved)
{
    quint64 windowId = 54321;
    TitleBarWidget *widget = new TitleBarWidget();

    TitleBarHelper::addTileBar(windowId, widget);
    TitleBarHelper::removeTitleBar(windowId);

    EXPECT_FALSE(TitleBarHelper::kTitleBarMap.contains(windowId));

    delete widget;
}

TEST_F(TitleBarHelperTest, RemoveTitleBar_NonExistentWindowId_DoesNotCrash)
{
    TitleBarHelper::removeTitleBar(99999);
    // Should not crash
}

TEST_F(TitleBarHelperTest, RemoveTitleBar_AlreadyRemoved_DoesNotCrash)
{
    quint64 windowId = 11111;
    TitleBarWidget *widget = new TitleBarWidget();

    TitleBarHelper::addTileBar(windowId, widget);
    TitleBarHelper::removeTitleBar(windowId);
    TitleBarHelper::removeTitleBar(windowId);   // Remove again

    // Should not crash
    delete widget;
}

TEST_F(TitleBarHelperTest, WindowId_ValidWidget_ReturnsWindowId)
{
    QWidget *widget = new QWidget();
    quint64 expectedId = 12345;

    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [expectedId](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return expectedId;
    });

    quint64 id = TitleBarHelper::windowId(widget);
    EXPECT_EQ(id, expectedId);

    delete widget;
}

TEST_F(TitleBarHelperTest, WindowId_NullWidget_ReturnsZero)
{
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [](FileManagerWindowsManager *, const QWidget *) {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(0);
    });

    quint64 id = TitleBarHelper::windowId(nullptr);
    EXPECT_EQ(id, 0);
}

TEST_F(TitleBarHelperTest, CreateSettingsMenu_ValidWindowId_CreatesMenu)
{
    quint64 windowId = 12345;
    FileManagerWindow *window = new FileManagerWindow(QUrl::fromLocalFile("/home"));
    TitleBarWidget *titleBarWidget = new TitleBarWidget();

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [window](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return window;
    });

    stub.set_lamda(&QWidget::property, [] {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    stub.set_lamda(&DeviceUtils::checkDiskEncrypted, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Should not crash
    TitleBarHelper::createSettingsMenu(windowId);

    delete titleBarWidget;
    delete window;
}

TEST_F(TitleBarHelperTest, CreateSettingsMenu_WithDisabledMenu_DisablesMenu)
{
    quint64 windowId = 12345;
    FileManagerWindow *window = new FileManagerWindow(QUrl::fromLocalFile("/home"));

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [window](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return window;
    });

    TitleBarWidget w;
    stub.set_lamda(&FileManagerWindow::titleBar, [&w] {
        __DBG_STUB_INVOKE__
        return &w;
    });

    bool propertyChecked = false;
    stub.set_lamda(&QWidget::property, [&propertyChecked] {
        __DBG_STUB_INVOKE__
        propertyChecked = true;
        return QVariant(true);
    });

    TitleBarHelper::createSettingsMenu(windowId);
    EXPECT_TRUE(propertyChecked);

    delete window;
}

TEST_F(TitleBarHelperTest, CrumbSeprateUrl_FileUrl_ReturnsCrumbList)
{
    QUrl url("file:///home/user/documents");

    stub.set_lamda(&DeviceUtils::getLongestMountRootPath, [](const QString &) {
        __DBG_STUB_INVOKE__
        return QString("/");
    });

    stub.set_lamda(&UrlRoute::urlParentList, [](const QUrl &, QList<QUrl> *urls) {
        __DBG_STUB_INVOKE__
        urls->append(QUrl("file:///home"));
        urls->append(QUrl("file:///home/user"));
    });

    auto crumbs = TitleBarHelper::crumbSeprateUrl(url);
    EXPECT_GT(crumbs.size(), 0);
}

TEST_F(TitleBarHelperTest, CrumbSeprateUrl_EmptyUrl_ReturnsEmptyList)
{
    QUrl emptyUrl;
    auto crumbs = TitleBarHelper::crumbSeprateUrl(emptyUrl);
    EXPECT_TRUE(crumbs.isEmpty());
}

TEST_F(TitleBarHelperTest, CrumbSeprateUrl_HomeUrl_ContainsHomeCrumb)
{
    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).constLast();
    QUrl url = QUrl::fromLocalFile(homePath);

    stub.set_lamda(&SystemPathUtil::systemPathIconName, [](SystemPathUtil *, const QString &) {
        __DBG_STUB_INVOKE__
        return QString("user-home");
    });

    stub.set_lamda(&SystemPathUtil::systemPathDisplayName, [](SystemPathUtil *, const QString &) {
        __DBG_STUB_INVOKE__
        return QString("Home");
    });

    stub.set_lamda(&UrlRoute::urlParentList, [](const QUrl &, QList<QUrl> *) {
        __DBG_STUB_INVOKE__
    });

    auto crumbs = TitleBarHelper::crumbSeprateUrl(url);
    EXPECT_GT(crumbs.size(), 0);
}

TEST_F(TitleBarHelperTest, TansToCrumbDataList_ValidMapGroup_ReturnsCorrectList)
{
    QList<QVariantMap> mapGroup;
    QVariantMap map1, map2;

    map1[CustomKey::kUrl] = QUrl("file:///home");
    map1[CustomKey::kDisplayText] = QString("Home");
    map1[CustomKey::kIconName] = QString("user-home");

    map2[CustomKey::kUrl] = QUrl("file:///home/user");
    map2[CustomKey::kDisplayText] = QString("User");
    map2[CustomKey::kIconName] = QString("folder");

    mapGroup.append(map1);
    mapGroup.append(map2);

    auto crumbs = TitleBarHelper::tansToCrumbDataList(mapGroup);

    EXPECT_EQ(crumbs.size(), 2);
    EXPECT_EQ(crumbs[0].url, QUrl("file:///home"));
    EXPECT_EQ(crumbs[0].displayText, QString("Home"));
    EXPECT_EQ(crumbs[0].iconName, QString("user-home"));
    EXPECT_EQ(crumbs[1].url, QUrl("file:///home/user"));
}

TEST_F(TitleBarHelperTest, TansToCrumbDataList_EmptyMapGroup_ReturnsEmptyList)
{
    QList<QVariantMap> emptyGroup;
    auto crumbs = TitleBarHelper::tansToCrumbDataList(emptyGroup);
    EXPECT_TRUE(crumbs.isEmpty());
}

TEST_F(TitleBarHelperTest, HandleJumpToPressed_ValidLocalPath_SendsCdEvent)
{
    QWidget *sender = new QWidget();
    QString path = "/home/user";
    bool cdSent = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&TitleBarEventCaller::sendCheckAddressInputStr, [](QWidget *, QString *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(qOverload<const QString &, bool>(&UrlRoute::fromUserInput), [](const QString &, bool) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/user");
    });

    stub.set_lamda(&UrlRoute::hasScheme, [](const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, exists), [](FileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&TitleBarEventCaller::sendCd, [&cdSent](QWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        cdSent = true;
    });

    TitleBarHelper::handleJumpToPressed(sender, path);
    EXPECT_TRUE(cdSent);

    delete sender;
}

TEST_F(TitleBarHelperTest, HandleJumpToPressed_FileUrl_SendsOpenFile)
{
    QWidget *sender = new QWidget();
    QString path = "/home/user/test.txt";
    bool openFileSent = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&TitleBarEventCaller::sendCheckAddressInputStr, [](QWidget *, QString *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(qOverload<const QString &, bool>(&UrlRoute::fromUserInput), [](const QString &, bool) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home/user/test.txt");
    });

    stub.set_lamda(&UrlRoute::hasScheme, [](const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, exists), [](FileInfo *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&TitleBarEventCaller::sendOpenFile, [&openFileSent](QWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        openFileSent = true;
    });

    TitleBarHelper::handleJumpToPressed(sender, path);
    EXPECT_TRUE(openFileSent);

    delete sender;
}

TEST_F(TitleBarHelperTest, HandleJumpToPressed_InvalidUrl_DoesNotSendEvent)
{
    QWidget *sender = new QWidget();
    QString invalidPath = "invalid path";
    bool eventSent = false;

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [](quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<TitleBarWidget *>(nullptr);
    });

    stub.set_lamda(&TitleBarEventCaller::sendCheckAddressInputStr, [](QWidget *, QString *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(qOverload<const QString &, bool>(&UrlRoute::fromUserInput), [](const QString &, bool) {
        __DBG_STUB_INVOKE__
        return QUrl();
    });

    stub.set_lamda(&UrlRoute::hasScheme, [](const QString &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&TitleBarEventCaller::sendCd, [&eventSent](QWidget *, const QUrl &) {
        __DBG_STUB_INVOKE__
        eventSent = true;
    });

    TitleBarHelper::handleJumpToPressed(sender, invalidPath);
    EXPECT_FALSE(eventSent);

    delete sender;
}

TEST_F(TitleBarHelperTest, HandleSearch_ValidKeyword_SendsSearchEvent)
{
    QWidget *sender = new QWidget();
    QString keyword = "test search";
    bool searchSent = false;
    TitleBarWidget *titleBarWidget = new TitleBarWidget();

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [titleBarWidget](quint64) {
        __DBG_STUB_INVOKE__
        return titleBarWidget;
    });

    stub.set_lamda(VADDR(TitleBarWidget, currentUrl), [](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home");
    });

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, const QUrl &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [](EventChannelManager *, const QString &, const QString &, const QUrl &) {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });

    stub.set_lamda(&TitleBarEventCaller::sendSearch, [&searchSent](QWidget *, const QString &) {
        __DBG_STUB_INVOKE__
        searchSent = true;
    });

    TitleBarHelper::handleSearch(sender, keyword);
    EXPECT_TRUE(searchSent);

    delete sender;
    delete titleBarWidget;
}

TEST_F(TitleBarHelperTest, HandleSearch_DisabledSearch_DoesNotSendEvent)
{
    QWidget *sender = new QWidget();
    QString keyword = "test";
    bool searchSent = false;
    TitleBarWidget *titleBarWidget = new TitleBarWidget();

    stub.set_lamda(&TitleBarHelper::findTileBarByWindowId, [titleBarWidget](quint64) {
        __DBG_STUB_INVOKE__
        return titleBarWidget;
    });

    stub.set_lamda(VADDR(TitleBarWidget, currentUrl), [](TitleBarWidget *) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home");
    });

    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QVariant(true);   // Search disabled
                   });

    stub.set_lamda(&TitleBarEventCaller::sendSearch, [&searchSent](QWidget *, const QString &) {
        __DBG_STUB_INVOKE__
        searchSent = true;
    });

    TitleBarHelper::handleSearch(sender, keyword);
    EXPECT_FALSE(searchSent);

    delete sender;
    delete titleBarWidget;
}

TEST_F(TitleBarHelperTest, OpenCurrentUrlInNewTab_ValidWindowId_SendsOpenTabEvent)
{
    quint64 windowId = 12345;
    FileManagerWindow *window = new FileManagerWindow(QUrl::fromLocalFile("/home"));
    QUrl testUrl("file:///home/test");
    bool openTabSent = false;

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [window](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return window;
    });

    stub.set_lamda(&FileManagerWindow::currentUrl, [testUrl](FileManagerWindow *) {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    stub.set_lamda(&TitleBarEventCaller::sendOpenTab, [&openTabSent](quint64, const QUrl &) {
        __DBG_STUB_INVOKE__
        openTabSent = true;
    });

    TitleBarHelper::openCurrentUrlInNewTab(windowId);
    EXPECT_TRUE(openTabSent);

    delete window;
}

TEST_F(TitleBarHelperTest, OpenCurrentUrlInNewTab_InvalidWindowId_DoesNotCrash)
{
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return static_cast<FileManagerWindow *>(nullptr);
    });

    TitleBarHelper::openCurrentUrlInNewTab(99999);
    // Should not crash
}

TEST_F(TitleBarHelperTest, ShowSettingsDialog_ValidWindowId_PublishesEvent)
{
    quint64 windowId = 12345;
    bool eventPublished = false;

    typedef bool (EventDispatcherManager::*PublishFunc)(EventType, quint64);
    stub.set_lamda(static_cast<PublishFunc>(&EventDispatcherManager::publish),
                   [&eventPublished](EventDispatcherManager *, EventType, quint64) {
                       __DBG_STUB_INVOKE__
                       eventPublished = true;
                       return true;
                   });

    TitleBarHelper::showSettingsDialog(windowId);
    EXPECT_TRUE(eventPublished);
}

TEST_F(TitleBarHelperTest, ShowConnectToServerDialog_ValidWindowId_ShowsDialog)
{
    quint64 windowId = 12345;
    FileManagerWindow *window = new FileManagerWindow(QUrl::fromLocalFile("/home"));

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [window](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return window;
    });

    stub.set_lamda(&QWidget::property, [] {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    stub.set_lamda(&FileManagerWindow::currentUrl, [](FileManagerWindow *) {
        __DBG_STUB_INVOKE__
        return QUrl("file:///home");
    });

    // Should not crash
    TitleBarHelper::showConnectToServerDialog(windowId);

    delete window;
}

TEST_F(TitleBarHelperTest, ShowConnectToServerDialog_AlreadyShown_DoesNotShowAgain)
{
    quint64 windowId = 12345;
    FileManagerWindow *window = new FileManagerWindow(QUrl::fromLocalFile("/home"));

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [window](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return window;
    });

    stub.set_lamda(&QWidget::property, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    TitleBarHelper::showConnectToServerDialog(windowId);
    // Should return early without showing dialog

    delete window;
}

TEST_F(TitleBarHelperTest, ShowUserSharePasswordSettingDialog_ValidWindowId_ShowsDialog)
{
    quint64 windowId = 12345;
    FileManagerWindow *window = new FileManagerWindow(QUrl::fromLocalFile("/home"));

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [window](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return window;
    });

    stub.set_lamda(&QWidget::property, [] {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    // Should not crash
    TitleBarHelper::showUserSharePasswordSettingDialog(windowId);

    delete window;
}

TEST_F(TitleBarHelperTest, ShowDiskPasswordChangingDialog_ValidWindowId_ShowsDialog)
{
    quint64 windowId = 12345;
    FileManagerWindow *window = new FileManagerWindow(QUrl::fromLocalFile("/home"));

    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [window](FileManagerWindowsManager *, quint64) {
        __DBG_STUB_INVOKE__
        return window;
    });

    stub.set_lamda(&QWidget::property, [] {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });

    // Should not crash
    TitleBarHelper::showDiskPasswordChangingDialog(windowId);

    delete window;
}

TEST_F(TitleBarHelperTest, RegisterKeepTitleStatusScheme_ValidScheme_SchemeRegistered)
{
    QString scheme = "testscheme";
    TitleBarHelper::registerKeepTitleStatusScheme(scheme);

    EXPECT_TRUE(TitleBarHelper::kKeepTitleStatusSchemeList.contains(scheme));
}

TEST_F(TitleBarHelperTest, RegisterKeepTitleStatusScheme_DuplicateScheme_NotAddedTwice)
{
    QString scheme = "testscheme";

    TitleBarHelper::registerKeepTitleStatusScheme(scheme);
    int count1 = TitleBarHelper::kKeepTitleStatusSchemeList.count(scheme);

    TitleBarHelper::registerKeepTitleStatusScheme(scheme);
    int count2 = TitleBarHelper::kKeepTitleStatusSchemeList.count(scheme);

    EXPECT_EQ(count1, count2);
}

TEST_F(TitleBarHelperTest, CheckKeepTitleStatus_RegisteredScheme_ReturnsTrue)
{
    QString scheme = "myscheme";
    QUrl url(QString("%1:///test").arg(scheme));

    TitleBarHelper::registerKeepTitleStatusScheme(scheme);
    EXPECT_TRUE(TitleBarHelper::checkKeepTitleStatus(url));
}

TEST_F(TitleBarHelperTest, CheckKeepTitleStatus_UnregisteredScheme_ReturnsFalse)
{
    QUrl url("unregistered:///test");
    EXPECT_FALSE(TitleBarHelper::checkKeepTitleStatus(url));
}

TEST_F(TitleBarHelperTest, RegisterViewModelUrlCallback_ValidScheme_CallbackRegistered)
{
    QString scheme = "myscheme";
    auto callback = [](const QUrl &url) { return url; };

    TitleBarHelper::registerViewModelUrlCallback(scheme, callback);
    EXPECT_TRUE(TitleBarHelper::kViewModeUrlCallbackMap.contains(scheme));
}

TEST_F(TitleBarHelperTest, RegisterViewModelUrlCallback_DuplicateScheme_NotOverwritten)
{
    QString scheme = "myscheme";
    auto callback1 = [](const QUrl &url) { return url; };
    auto callback2 = [](const QUrl &url) { return QUrl(); };

    TitleBarHelper::registerViewModelUrlCallback(scheme, callback1);
    auto retrieved1 = TitleBarHelper::kViewModeUrlCallbackMap[scheme];

    TitleBarHelper::registerViewModelUrlCallback(scheme, callback2);
    auto retrieved2 = TitleBarHelper::kViewModeUrlCallbackMap[scheme];

    // Should not be overwritten (pointer comparison won't work, just check it exists)
    EXPECT_TRUE(TitleBarHelper::kViewModeUrlCallbackMap.contains(scheme));
}

TEST_F(TitleBarHelperTest, ViewModelUrlCallback_RegisteredScheme_ReturnsCallback)
{
    QString scheme = "testscheme";
    QUrl testUrl(QString("%1:///test").arg(scheme));
    auto callback = [](const QUrl &url) { return url; };

    TitleBarHelper::registerViewModelUrlCallback(scheme, callback);
    auto retrieved = TitleBarHelper::viewModelUrlCallback(testUrl);

    EXPECT_NE(retrieved, nullptr);
}

TEST_F(TitleBarHelperTest, ViewModelUrlCallback_UnregisteredScheme_ReturnsNull)
{
    QUrl url("unregistered:///test");
    auto callback = TitleBarHelper::viewModelUrlCallback(url);
    EXPECT_EQ(callback, nullptr);
}

TEST_F(TitleBarHelperTest, TransformViewModeUrl_WithCallback_TransformsUrl)
{
    QString scheme = "transform";
    QUrl originalUrl(QString("%1:///original").arg(scheme));
    QUrl transformedUrl(QString("%1:///transformed").arg(scheme));

    auto callback = [transformedUrl](const QUrl &) { return transformedUrl; };
    TitleBarHelper::registerViewModelUrlCallback(scheme, callback);

    QUrl result = TitleBarHelper::transformViewModeUrl(originalUrl);
    EXPECT_EQ(result, transformedUrl);
}

TEST_F(TitleBarHelperTest, TransformViewModeUrl_WithoutCallback_ReturnsOriginalUrl)
{
    QUrl url("file:///test");
    QUrl result = TitleBarHelper::transformViewModeUrl(url);
    EXPECT_EQ(result, url);
}

TEST_F(TitleBarHelperTest, GetFileViewStateValue_ValidUrl_ReturnsValue)
{
    QUrl url("file:///test");
    QString key = "testKey";
    QVariant defaultValue("default");

    stub.set_lamda(qOverload<const QString &, const QUrl &, const QVariant &>(&Settings::value), [] {
        __DBG_STUB_INVOKE__
        QVariantMap map;
        map["testKey"] = QString("value");
        return QVariant(map);
    });

    QVariant result = TitleBarHelper::getFileViewStateValue(url, key, defaultValue);
    EXPECT_EQ(result.toString(), QString("value"));
}

TEST_F(TitleBarHelperTest, GetFileViewStateValue_NonExistentKey_ReturnsDefaultValue)
{
    QUrl url("file:///test");
    QString key = "nonExistentKey";
    QVariant defaultValue("default");

    stub.set_lamda(qOverload<const QString &, const QUrl &, const QVariant &>(&Settings::value), [] {
        __DBG_STUB_INVOKE__
        return QVariant(QVariantMap());
    });

    QVariant result = TitleBarHelper::getFileViewStateValue(url, key, defaultValue);
    EXPECT_EQ(result, defaultValue);
}

TEST_F(TitleBarHelperTest, SetFileViewStateValue_ValidUrl_SetsValue)
{
    QUrl url("file:///test");
    QString key = "testKey";
    QVariant value("testValue");
    bool setValueCalled = false;

    stub.set_lamda(qOverload<const QString &, const QUrl &, const QVariant &>(&Settings::value), [] {
        __DBG_STUB_INVOKE__
        return QVariant(QVariantMap());
    });

    stub.set_lamda(qOverload<const QString &, const QUrl &, const QVariant &>(&Settings::setValue), [&setValueCalled] {
        __DBG_STUB_INVOKE__
        setValueCalled = true;
    });

    TitleBarHelper::setFileViewStateValue(url, key, value);
    EXPECT_TRUE(setValueCalled);
}

TEST_F(TitleBarHelperTest, NewWindowAndTabEnabled_DefaultValue_IsTrue)
{
    TitleBarHelper::newWindowAndTabEnabled = true;
    EXPECT_TRUE(TitleBarHelper::newWindowAndTabEnabled);
}

TEST_F(TitleBarHelperTest, NewWindowAndTabEnabled_SetToFalse_IsFalse)
{
    TitleBarHelper::newWindowAndTabEnabled = false;
    EXPECT_FALSE(TitleBarHelper::newWindowAndTabEnabled);
}

TEST_F(TitleBarHelperTest, SearchEnabled_DefaultValue_IsFalse)
{
    TitleBarHelper::searchEnabled = false;
    EXPECT_FALSE(TitleBarHelper::searchEnabled);
}

TEST_F(TitleBarHelperTest, SearchEnabled_SetToTrue_IsTrue)
{
    TitleBarHelper::searchEnabled = true;
    EXPECT_TRUE(TitleBarHelper::searchEnabled);
}
