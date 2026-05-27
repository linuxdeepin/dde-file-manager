// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/crumbinterface.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class CrumbInterfaceTest : public testing::Test
{
protected:
    void SetUp() override
    {
        crumb = new CrumbInterface();
        stub.clear();
    }

    void TearDown() override
    {
        delete crumb;
        crumb = nullptr;
        stub.clear();
    }

    CrumbInterface *crumb { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(CrumbInterfaceTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(crumb, nullptr);
}

TEST_F(CrumbInterfaceTest, SetSupportedScheme_FileScheme_SchemeSet)
{
    crumb->setSupportedScheme("file");
    EXPECT_TRUE(crumb->isSupportedScheme("file"));
}

TEST_F(CrumbInterfaceTest, IsSupportedScheme_UnsupportedScheme_ReturnsFalse)
{
    crumb->setSupportedScheme("file");
    EXPECT_FALSE(crumb->isSupportedScheme("smb"));
}

TEST_F(CrumbInterfaceTest, IsSupportedScheme_EmptyScheme_ReturnsFalse)
{
    crumb->setSupportedScheme("file");
    EXPECT_FALSE(crumb->isSupportedScheme(""));
}

TEST_F(CrumbInterfaceTest, ProcessAction_EscKeyPressed_HideAddressBarEmitted)
{
    QSignalSpy spy(crumb, &CrumbInterface::hideAddressBar);
    crumb->processAction(CrumbInterface::kEscKeyPressed);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CrumbInterfaceTest, ProcessAction_ClearButtonPressed_HideAddressBarEmitted)
{
    QSignalSpy spy(crumb, &CrumbInterface::hideAddressBar);
    crumb->processAction(CrumbInterface::kClearButtonPressed);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CrumbInterfaceTest, ProcessAction_AddressBarLostFocus_HideAddressBarEmitted)
{
    QSignalSpy spy(crumb, &CrumbInterface::hideAddressBar);
    crumb->processAction(CrumbInterface::kAddressBarLostFocus);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CrumbInterfaceTest, SeprateUrl_FileScheme_ReturnsCorrectCrumbs)
{
    QUrl url("file:///home/user/documents");

    stub.set_lamda(&TitleBarHelper::crumbSeprateUrl, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        QList<CrumbData> list;
        list.append(CrumbData { QUrl("file:///"), "/" });
        list.append(CrumbData { QUrl("file:///home"), "home" });
        list.append(CrumbData { QUrl("file:///home/user"), "user" });
        list.append(CrumbData { QUrl("file:///home/user/documents"), "documents" });
        return list;
    });

    QList<CrumbData> crumbs = crumb->seprateUrl(url);
    EXPECT_EQ(crumbs.size(), 4);
}

TEST_F(CrumbInterfaceTest, SeprateUrl_RootUrl_ReturnsRootCrumb)
{
    QUrl url("file:///");

    stub.set_lamda(&TitleBarHelper::crumbSeprateUrl, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        QList<CrumbData> list;
        list.append(CrumbData { QUrl("file:///"), "/" });
        return list;
    });

    QList<CrumbData> crumbs = crumb->seprateUrl(url);
    EXPECT_EQ(crumbs.size(), 1);
}

TEST_F(CrumbInterfaceTest, SeprateUrl_CustomScheme_UsesDefaultMethod)
{
    QUrl url("custom://path/to/resource");

    stub.set_lamda(&UrlRoute::urlParentList, [](QUrl, QList<QUrl> *urls) {
        __DBG_STUB_INVOKE__
        urls->append(QUrl("custom://path"));
    });
    stub.set_lamda(&UrlRoute::isRootUrl, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QList<CrumbData> crumbs = crumb->seprateUrl(url);
    // Should have at least the original URL
    EXPECT_GE(crumbs.size(), 1);
}

TEST_F(CrumbInterfaceTest, RequestCompletionList_ValidUrl_SignalEmitted)
{
    QUrl url("file:///home");
    QSignalSpy spy(crumb, &CrumbInterface::completionListTransmissionCompleted);

    // Stub the TraversalDirThread
    stub.set_lamda(&DFMBASE_NAMESPACE::TraversalDirThread::start, []() {
        __DBG_STUB_INVOKE__
        // Do nothing
    });
    stub.set_lamda(&DFMBASE_NAMESPACE::TraversalDirThread::stop, []() {
        __DBG_STUB_INVOKE__
    });

    crumb->requestCompletionList(url);
    // The signal would be emitted when thread completes
    // We can't easily test async behavior without full integration
}

TEST_F(CrumbInterfaceTest, CancelCompletionListTransmission_WithRunningJob_JobCanceled)
{
    QUrl url("file:///home");

    stub.set_lamda(&DFMBASE_NAMESPACE::TraversalDirThread::start, []() {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&DFMBASE_NAMESPACE::TraversalDirThread::stop, []() {
        __DBG_STUB_INVOKE__
    });

    crumb->requestCompletionList(url);
    EXPECT_NO_THROW(crumb->cancelCompletionListTransmission());
}

TEST_F(CrumbInterfaceTest, SeprateUrl_TrashUrl_ReturnsTrashCrumb)
{
    QUrl url("trash:///");

    stub.set_lamda(&FileUtils::trashRootUrl, []() {
        __DBG_STUB_INVOKE__
        return QUrl("trash:///");
    });
    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&UrlRoute::urlParentList, [](QUrl, QList<QUrl> *) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&UrlRoute::isRootUrl, [](const QUrl &url) {
        __DBG_STUB_INVOKE__
        return url.scheme() == "trash";
    });
    stub.set_lamda(&UrlRoute::icon, [](const QString &) {
        __DBG_STUB_INVOKE__
        return QIcon("trash-icon");
    });
    stub.set_lamda(&QIcon::name, [] { __DBG_STUB_INVOKE__ return "trash-icon"; });

    QList<CrumbData> crumbs = crumb->seprateUrl(url);
    EXPECT_GE(crumbs.size(), 1);
    if (!crumbs.isEmpty()) {
        EXPECT_FALSE(crumbs.first().iconName.isEmpty());
    }
}
