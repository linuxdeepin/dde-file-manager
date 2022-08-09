/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "views/addressbar.h"
#include "views/private/addressbar_p.h"
#include "models/completerviewmodel.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"

#include "stubext.h"

#include <QCompleter>

#include <gtest/gtest.h>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(AddressBarPrivateTest, ut_completeSearchHistory)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });
    st.set_lamda(&QCompleter::setCompletionPrefix, [] {});
    st.set_lamda(&CompleterViewModel::setStringList, [] {});

    AddressBar bar;
    bar.d->completeSearchHistory("test");

    EXPECT_TRUE(bar.d->isHistoryInCompleterModel);
}

TEST(AddressBarPrivateTest, ut_completeIpAddress)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });
    st.set_lamda(&CompleterViewModel::setRowCount, [] {});

    typedef void (CompleterViewModel::*Func)(int, int, QStandardItem *);
    auto func = static_cast<Func>(&CompleterViewModel::setItem);
    st.set_lamda(func, [](CompleterViewModel *, int, int, QStandardItem *item) { delete item; });

    QStandardItem item;
    st.set_lamda(&CompleterViewModel::item, [&item] { return &item; });
    st.set_lamda(&QStandardItem::setIcon, [] {});
    st.set_lamda(&IPHistroyData::isRecentlyAccessed, [] { return true; });

    IPHistroyData data1("smb://1.0.0.0", QDateTime::currentDateTime());
    IPHistroyData data2("ftp://1.0.0.0", QDateTime::currentDateTime());
    IPHistroyData data3("sftp://1.0.0.0", QDateTime::currentDateTime());
    AddressBar bar;
    bar.d->ipHistroyList << data1 << data2 << data3;
    EXPECT_NO_FATAL_FAILURE(bar.d->completeIpAddress("1.0.0.0"));
}

TEST(AddressBarPrivateTest, ut_completeLocalPath_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });
    st.set_lamda(&LocalFileInfo::init, [] {});
    st.set_lamda(&InfoFactory::create<AbstractFileInfo>, [] {
        return QSharedPointer<LocalFileInfo>(new LocalFileInfo(QUrl::fromLocalFile("/home/test")));
    });
    st.set_lamda(VADDR(LocalFileInfo, exists), [] { return false; });

    AddressBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.d->completeLocalPath("/home/test", QUrl::fromLocalFile("/home"), 5));
}

TEST(AddressBarPrivateTest, ut_completeLocalPath_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });
    st.set_lamda(&LocalFileInfo::init, [] {});
    st.set_lamda(&InfoFactory::create<AbstractFileInfo>, [] {
        return QSharedPointer<LocalFileInfo>(new LocalFileInfo(QUrl::fromLocalFile("/home")));
    });
    st.set_lamda(&QCompleter::setCompletionPrefix, [] {});
    st.set_lamda(&AddressBarPrivate::onCompletionModelCountChanged, [] {});

    AddressBar bar;
    bar.d->isHistoryInCompleterModel = false;
    bar.d->completerBaseString = "/home/";
    EXPECT_NO_FATAL_FAILURE(bar.d->completeLocalPath("/home/test", QUrl::fromLocalFile("/home"), 5));
}

TEST(AddressBarPrivateTest, ut_completeLocalPath_3)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });
    st.set_lamda(&LocalFileInfo::init, [] {});
    st.set_lamda(&InfoFactory::create<AbstractFileInfo>, [] {
        return QSharedPointer<LocalFileInfo>(new LocalFileInfo(QUrl::fromLocalFile("/home")));
    });
    st.set_lamda(&QCompleter::setCompletionPrefix, [] {});
    st.set_lamda(&AddressBarPrivate::clearCompleterModel, [] {});
    st.set_lamda(&AddressBarPrivate::requestCompleteByUrl, [] {});

    AddressBar bar;
    bar.d->isHistoryInCompleterModel = true;
    EXPECT_NO_FATAL_FAILURE(bar.d->completeLocalPath("/home/test", QUrl::fromLocalFile("/home"), 5));
}

TEST(AddressBarPrivateTest, ut_onReturnPressed)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });
    st.set_lamda(&AddressBar::text, [] { return ""; });

    AddressBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.d->onReturnPressed());

    st.reset(&AddressBar::text);
    st.set_lamda(&AddressBar::text, [] { return "smb://1.0.0.1"; });
    st.set_lamda(&SearchHistroyManager::writeIntoSearchHistory, [] {});
    st.set_lamda(&SearchHistroyManager::writeIntoIPHistory, [] {});
    st.set_lamda(TitleBarHelper::handlePressed, [] {});

    EXPECT_NO_FATAL_FAILURE(bar.d->onReturnPressed());
}

TEST(AddressBarPrivateTest, ut_insertCompletion)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });

    AddressBar bar;
    EXPECT_NO_FATAL_FAILURE(bar.d->insertCompletion("test"));

    bar.d->urlCompleter->setWidget(&bar);
    bar.d->completerBaseString = "123";
    bar.d->insertCompletion("test");
    EXPECT_EQ("123test", bar.text());

    bar.d->inputIsIpAddress = true;
    bar.d->insertCompletion("test");
    EXPECT_EQ("test", bar.text());
}

TEST(AddressBarPrivateTest, ut_onCompletionHighlighted)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHistroyManager::getSearchHistroy, [] { return QStringList(); });
    st.set_lamda(&SearchHistroyManager::getIPHistory, [] { return QList<IPHistroyData>(); });

    AddressBar bar;
    bar.d->inputIsIpAddress = true;
    bar.d->completerBaseString = "123";
    bar.d->onCompletionHighlighted("");
    EXPECT_EQ("123", bar.text());

    bar.d->onCompletionHighlighted("smb://123");
    EXPECT_EQ("smb://123", bar.text());

    bar.d->inputIsIpAddress = false;
    bar.d->onCompletionHighlighted("test");
    EXPECT_EQ("123test", bar.text());
}
