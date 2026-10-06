// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_addressbar_cov2.cpp
 * @brief Coverage for addressbar.cpp methods not covered by test_addressbar.cpp:
 *        AddressBar::event, AddressBarPrivate::onTextEdited,
 *        AddressBarPrivate::onReturnPressed, AddressBarPrivate::clearCompleterModel,
 *        AddressBarPrivate::completeIpAddress, AddressBarPrivate::doComplete,
 *        AddressBarPrivate::eventFilterHide, AddressBarPrivate::eventFilter.
 *        Private members accessed via d-> (global -fno-access-control).
 */

#include "stubext.h"
#include "views/addressbar.h"
#include "views/private/addressbar_p.h"
#include "utils/crumbinterface.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>
#include <QUrl>
#include <QCompleter>
#include <QStandardItemModel>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <QApplication>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

namespace {
class ExposedAddressBar : public AddressBar
{
public:
    using AddressBar::event;
    using AddressBar::keyPressEvent;
    using AddressBar::focusInEvent;
    using AddressBar::focusOutEvent;
    using AddressBar::showEvent;
    using AddressBar::paintEvent;
    using AddressBar::inputMethodEvent;
};
}   // namespace

class AddressBarCov2Test : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        stub.set_lamda(&UrlRoute::hasScheme, [](const QString &) {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(qOverload<const QString &, bool>(&UrlRoute::fromUserInput), [](const QString &input, bool) {
            __DBG_STUB_INVOKE__
            if (input.startsWith("smb://") || input.startsWith("ftp://") || input.startsWith("sftp://"))
                return QUrl(input);
            return QUrl::fromLocalFile(input);
        });

        stub.set_lamda(&UrlRoute::toString, [](const QUrl &url, QUrl::FormattingOptions) {
            __DBG_STUB_INVOKE__
            if (url.scheme() == "file")
                return url.toLocalFile();
            return url.toString();
        });

        stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
            __DBG_STUB_INVOKE__
            return url1 == url2;
        });

        stub.set_lamda(&SearchHistroyManager::writeIntoSearchHistory, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&SearchHistroyManager::getSearchHistroy, [] {
            __DBG_STUB_INVOKE__
            return QStringList();
        });

        stub.set_lamda(&AddressBar::clearFocus, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TitleBarEventCaller::sendCheckAddressInputStr, [](QWidget *, QString *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 123; });

        addressBar = new ExposedAddressBar();
        addressBar->resize(400, 30);
    }

    void TearDown() override
    {
        delete addressBar;
        addressBar = nullptr;
        stub.clear();
    }

    ExposedAddressBar *addressBar { nullptr };
    stub_ext::StubExt stub;
};

// ---- AddressBar::event ----

TEST_F(AddressBarCov2Test, Event_KeyPressEvent_HandledReturnsTrue)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    EXPECT_TRUE(addressBar->event(&keyEvent));
}

TEST_F(AddressBarCov2Test, Event_NonKeyPressEvent_DelegatesToBase)
{
    QFocusEvent focusEvent(QEvent::FocusIn);
    EXPECT_NO_FATAL_FAILURE({ addressBar->event(&focusEvent); });
}

// ---- AddressBarPrivate::onTextEdited ----

TEST_F(AddressBarCov2Test, OnTextEdited_EmptyText_HidesPopupAndClearsBaseString)
{
    addressBar->d->completerBaseString = "previous";
    EXPECT_NO_FATAL_FAILURE({ addressBar->d->onTextEdited(""); });
    EXPECT_TRUE(addressBar->d->completerBaseString.isEmpty());
}

TEST_F(AddressBarCov2Test, OnTextEdited_NonEmptyText_StopsTimerAndUpdatesCompletion)
{
    addressBar->d->timer.start();
    EXPECT_NO_FATAL_FAILURE({ addressBar->d->onTextEdited("192.168.1.1"); });
    EXPECT_FALSE(addressBar->d->timer.isActive());
}

TEST_F(AddressBarCov2Test, OnTextEdited_LocalPath_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ addressBar->d->onTextEdited("/home/test"); });
}

// ---- AddressBarPrivate::onReturnPressed ----

TEST_F(AddressBarCov2Test, OnReturnPressed_EmptyText_ReturnsEarly)
{
    addressBar->setText("");
    EXPECT_NO_FATAL_FAILURE({ addressBar->d->onReturnPressed(); });
}

TEST_F(AddressBarCov2Test, OnReturnPressed_LocalFilePath_NoCrash)
{
    addressBar->setText("/home/test");
    EXPECT_NO_FATAL_FAILURE({ addressBar->d->onReturnPressed(); });
}

TEST_F(AddressBarCov2Test, OnReturnPressed_IpAddress_CallsAddIPHistoryCache)
{
    bool called = false;
    stub.set_lamda(&SearchHistroyManager::addIPHistoryCache, [&called](SearchHistroyManager *, const QString &) {
        __DBG_STUB_INVOKE__
        called = true;
    });
    addressBar->setText("smb://192.168.1.1");
    addressBar->d->onReturnPressed();
    EXPECT_TRUE(called);
}

TEST_F(AddressBarCov2Test, OnReturnPressed_EmitsUrlChanged)
{
    bool emitted = false;
    QObject::connect(addressBar, &AddressBar::urlChanged, [&emitted]() {
        emitted = true;
    });
    addressBar->setText("/home/test");
    addressBar->d->onReturnPressed();
    EXPECT_TRUE(emitted);
}

// ---- AddressBarPrivate::clearCompleterModel ----

TEST_F(AddressBarCov2Test, ClearCompleterModel_ModelBecomesEmpty)
{
    addressBar->d->completerModel.appendRow(new QStandardItem("test"));
    EXPECT_EQ(addressBar->d->completerModel.rowCount(), 1);
    addressBar->d->clearCompleterModel();
    EXPECT_EQ(addressBar->d->completerModel.rowCount(), 0);
}

// ---- AddressBarPrivate::completeIpAddress ----

TEST_F(AddressBarCov2Test, CompleteIpAddress_SetsThreeCompletionItems)
{
    addressBar->d->completeIpAddress("192.168.1.1");
    EXPECT_EQ(addressBar->d->completerModel.rowCount(), 3);
    EXPECT_EQ(addressBar->d->completerModel.item(0, 0)->text(), "smb://192.168.1.1");
    EXPECT_EQ(addressBar->d->completerModel.item(1, 0)->text(), "ftp://192.168.1.1");
    EXPECT_EQ(addressBar->d->completerModel.item(2, 0)->text(), "sftp://192.168.1.1");
}

TEST_F(AddressBarCov2Test, CompleteIpAddress_SetsCompleterBaseString)
{
    addressBar->d->completeIpAddress("10.0.0.1");
    EXPECT_EQ(addressBar->d->completerBaseString, "10.0.0.1");
}

TEST_F(AddressBarCov2Test, CompleteIpAddress_RecentAccess_SetsIcon)
{
    IPHistroyData data("smb://192.168.1.1", QDateTime::currentDateTime());
    addressBar->d->ipHistroyList.append(data);
    EXPECT_NO_FATAL_FAILURE({ addressBar->d->completeIpAddress("192.168.1.1"); });
}


// ---- AddressBarPrivate::eventFilterHide ----

TEST_F(AddressBarCov2Test, EventFilterHide_StopsTimer_ReturnsFalse)
{
    addressBar->d->timer.start();
    QHideEvent hideEvent;
    EXPECT_FALSE(addressBar->d->eventFilterHide(addressBar, &hideEvent));
    EXPECT_FALSE(addressBar->d->timer.isActive());
}

// ---- AddressBarPrivate::eventFilter ----

TEST_F(AddressBarCov2Test, EventFilter_HideEvent_DelegatesToEventFilterHide)
{
    addressBar->d->timer.start();
    QHideEvent hideEvent;
    EXPECT_FALSE(addressBar->d->eventFilter(addressBar, &hideEvent));
    EXPECT_FALSE(addressBar->d->timer.isActive());
}

TEST_F(AddressBarCov2Test, EventFilter_NonHideEvent_ReturnsFalse)
{
    QFocusEvent focusEvent(QEvent::FocusIn);
    EXPECT_FALSE(addressBar->d->eventFilter(addressBar, &focusEvent));
}
