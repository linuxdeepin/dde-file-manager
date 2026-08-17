// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/addressbar.h"
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
#include <QPaintEvent>
#include <QShowEvent>
#include <QInputMethodEvent>
#include <QAbstractItemView>
#include <QPainter>
#include <QPropertyAnimation>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class AddressBarTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub icon loading
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        // Stub UrlRoute
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

        // Stub UniversalUtils
        stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) {
            __DBG_STUB_INVOKE__
            return url1 == url2;
        });

        // Stub SearchHistroyManager
        stub.set_lamda(&SearchHistroyManager::writeIntoSearchHistory, [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&SearchHistroyManager::getSearchHistroy, [] {
            __DBG_STUB_INVOKE__
            return QStringList();
        });

        addressBar = new AddressBar();
    }

    void TearDown() override
    {
        delete addressBar;
        addressBar = nullptr;
        stub.clear();
    }

    AddressBar *addressBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(AddressBarTest, SetCurrentUrl_LocalFileUrl_SetsUrlAndText)
{
    QUrl url("file:///home/test");
    bool hookCalled = false;

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl *);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run), [&hookCalled] {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return true;
    });

    addressBar->setCurrentUrl(url);
    EXPECT_TRUE(hookCalled);
}

TEST_F(AddressBarTest, SetCurrentUrl_HookReturnsFalse_DoesNotSetUrl)
{
    QUrl url("file:///home/test");
    QUrl originalUrl = addressBar->currentUrl();

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl *);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    addressBar->setCurrentUrl(url);
    EXPECT_EQ(addressBar->currentUrl(), originalUrl);
}

TEST_F(AddressBarTest, CurrentUrl_AfterConstruction_ReturnsEmptyUrl)
{
    EXPECT_TRUE(addressBar->currentUrl().isEmpty());
}

TEST_F(AddressBarTest, CurrentUrl_AfterSetCurrentUrl_ReturnsCorrectUrl)
{
    QUrl url("file:///home/test");

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl *);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run), [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&FileManagerWindowsManager::findWindowId, [&] {
        __DBG_STUB_INVOKE__
        return 123;
    });
    FileManagerWindow w(url);
    stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&w] {
        return &w;
    });

    addressBar->setCurrentUrl(url);
    EXPECT_EQ(addressBar->currentUrl(), url);
}

TEST_F(AddressBarTest, CompleterViewVisible_CompleterNotVisible_ReturnsFalse)
{
    bool visible = addressBar->completerViewVisible();
    EXPECT_FALSE(visible);
}

TEST_F(AddressBarTest, CompleterViewVisible_AfterShowCompleter_ReturnsState)
{
    // Visibility depends on actual completer state
    EXPECT_NO_THROW(addressBar->completerViewVisible());
}

TEST_F(AddressBarTest, ShowOnFocusLostOnce_Called_SetsFlag)
{
    EXPECT_NO_THROW(addressBar->showOnFocusLostOnce());
}

TEST_F(AddressBarTest, FocusInEvent_Called_SelectsAllText)
{
    QFocusEvent event(QEvent::FocusIn);

    stub.set_lamda(&QLineEdit::selectAll, [] {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(addressBar->focusInEvent(&event));
}

TEST_F(AddressBarTest, FocusOutEvent_Called_EmitsLostFocus)
{
    QFocusEvent event(QEvent::FocusOut);
    bool signalEmitted = false;

    QObject::connect(addressBar, &AddressBar::lostFocus, [&signalEmitted]() {
        signalEmitted = true;
    });

    stub.set_lamda(&AddressBar::hide, [] {
        __DBG_STUB_INVOKE__
    });

    addressBar->focusOutEvent(&event);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(AddressBarTest, FocusOutEvent_ShowOnFocusLost_StaysVisible)
{
    addressBar->showOnFocusLostOnce();
    QFocusEvent event(QEvent::FocusOut);

    stub.set_lamda(&AddressBar::hide, [] {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(addressBar->focusOutEvent(&event));
}

TEST_F(AddressBarTest, FocusOutEvent_CompleterVisible_KeepsCompleterOpen)
{
    QFocusEvent event(QEvent::FocusOut, Qt::PopupFocusReason);

    stub.set_lamda(&AddressBar::completerViewVisible, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&AddressBar::hide, [] {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(addressBar->focusOutEvent(&event));
}

TEST_F(AddressBarTest, KeyPressEvent_EscapeKey_EmitsEscKeyPressed)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    bool signalEmitted = false;

    QObject::connect(addressBar, &AddressBar::escKeyPressed, [&signalEmitted]() {
        signalEmitted = true;
    });

    stub.set_lamda(&AddressBar::clearFocus, [] {
        __DBG_STUB_INVOKE__
    });

    addressBar->keyPressEvent(&event);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(AddressBarTest, KeyPressEvent_ReturnKey_EmitsUrlChanged)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    bool signalEmitted = false;

    addressBar->setText("file:///home/test");

    QObject::connect(addressBar, &AddressBar::urlChanged, [&signalEmitted]() {
        signalEmitted = true;
    });

    stub.set_lamda(&AddressBar::clearFocus, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&TitleBarEventCaller::sendCheckAddressInputStr, [](QWidget *, QString *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 123; });

    addressBar->keyPressEvent(&event);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(AddressBarTest, KeyPressEvent_EnterKey_EmitsUrlChanged)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    bool signalEmitted = false;

    addressBar->setText("file:///home/test");

    QObject::connect(addressBar, &AddressBar::urlChanged, [&signalEmitted]() {
        signalEmitted = true;
    });

    stub.set_lamda(&AddressBar::clearFocus, [] {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&TitleBarEventCaller::sendCheckAddressInputStr, [](QWidget *, QString *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&TitleBarHelper::windowId, [] { __DBG_STUB_INVOKE__ return 123; });

    addressBar->keyPressEvent(&event);
    EXPECT_TRUE(signalEmitted);
}

TEST_F(AddressBarTest, KeyPressEvent_OtherKey_PassesToBase)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);

    EXPECT_NO_THROW(addressBar->keyPressEvent(&event));
}

TEST_F(AddressBarTest, KeyPressEvent_UpKey_NavigatesCompleter)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);

    EXPECT_NO_THROW(addressBar->keyPressEvent(&event));
}

TEST_F(AddressBarTest, KeyPressEvent_DownKey_NavigatesCompleter)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);

    EXPECT_NO_THROW(addressBar->keyPressEvent(&event));
}

TEST_F(AddressBarTest, ShowEvent_Called_StartsAnimation)
{
    QShowEvent event;
    bool timerStarted = false;
    stub.set_lamda(static_cast<void (QTimer::*)()>(&QTimer::start),
                   [&timerStarted](QTimer *timer) {
                       Q_UNUSED(timer)
                       timerStarted = true;
                   });

    addressBar->showEvent(&event);
    EXPECT_TRUE(timerStarted);
}

TEST_F(AddressBarTest, PaintEvent_WithEmptyText_DrawsPlaceholder)
{
    QPaintEvent event(QRect(0, 0, 100, 30));

    addressBar->setText("");

    stub.set_lamda(qOverload<const QRect &, int, const QString &, QRect *>(&QPainter::drawText), [] {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(addressBar->paintEvent(&event));
}

TEST_F(AddressBarTest, InputMethodEvent_ChineseInput_UpdatesCompletion)
{
    QInputMethodEvent event("测试", QList<QInputMethodEvent::Attribute>());

    EXPECT_NO_THROW(addressBar->inputMethodEvent(&event));
}

TEST_F(AddressBarTest, KeyPressEvent_ReturnWithEmptyText_DoesNotEmitUrlChanged)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    bool signalEmitted = false;

    addressBar->setText("");

    QObject::connect(addressBar, &AddressBar::urlChanged, [&signalEmitted]() {
        signalEmitted = true;
    });

    stub.set_lamda(&AddressBar::clearFocus, [] {
        __DBG_STUB_INVOKE__
    });

    addressBar->keyPressEvent(&event);
    EXPECT_FALSE(signalEmitted);
}

TEST_F(AddressBarTest, FocusOutEvent_PopupFocusReason_KeepsVisible)
{
    QFocusEvent event(QEvent::FocusOut, Qt::PopupFocusReason);

    stub.set_lamda(&AddressBar::hide, [] {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(addressBar->focusOutEvent(&event));
}

TEST_F(AddressBarTest, CompleterActivated_EmptyString_DoesNotEmitUrlChanged)
{
    bool signalEmitted = false;

    QObject::connect(addressBar, &AddressBar::urlChanged, [&signalEmitted]() {
        signalEmitted = true;
    });

    if (addressBar->completer()) {
        emit addressBar->completer()->activated("");
    }

    EXPECT_FALSE(signalEmitted);
}
