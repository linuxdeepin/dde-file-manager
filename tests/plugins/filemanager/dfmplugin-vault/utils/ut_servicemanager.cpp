// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/servicemanager.h"

#include <gtest/gtest.h>

#include <QUrl>

#include <dfm-base/base/application/settings.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_ServiceManager, basicViewFieldFunc)
{
    ServiceManager manager;
    ServiceManager::ExpandFieldMap map = ServiceManager::basicViewFieldFunc(QUrl("file:///UT_TEST"));

    EXPECT_FALSE(map.isEmpty());
}

TEST(UT_ServiceManager, detailViewFieldFunc_one)
{
    stub_ext::StubExt stub;
    typedef QVariant(Settings::*FuncType)(const QString &, const QString &, const QVariant &)const;
    stub.set_lamda(static_cast<FuncType>(&Settings::value), []{
        return QVariant::fromValue<QString>("");
    });

    ServiceManager::ExpandFieldMap map = ServiceManager::detailViewFieldFunc(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(map.isEmpty());
}

TEST(UT_ServiceManager, detailViewFieldFunc_two)
{
    stub_ext::StubExt stub;
    typedef QVariant(Settings::*FuncType)(const QString &, const QString &, const QVariant &)const;
    stub.set_lamda(static_cast<FuncType>(&Settings::value), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });

    ServiceManager::ExpandFieldMap map = ServiceManager::detailViewFieldFunc(QUrl("dfmvault:///"));

    EXPECT_FALSE(map.isEmpty());
}
