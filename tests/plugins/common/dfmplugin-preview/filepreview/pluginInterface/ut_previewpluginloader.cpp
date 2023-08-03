// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "pluginInterface/previewpluginloader.h"
#include "pluginInterface/private/pluginloader_p.h"

#include <gtest/gtest.h>

#include <QDir>

DPFILEPREVIEW_USE_NAMESPACE

TEST(UT_PreviewPluginLoader, metaData)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    QPluginLoader qLoader;
    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginLoaderList.push_back(&qLoader);
    QList<QJsonObject> result = loader.metaData();

    EXPECT_FALSE(result.isEmpty());
}

TEST(UT_PreviewPluginLoader, instance_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    PreviewPluginLoader loader("UT_TEST", "UT");
    QObject *result = loader.instance(-1);

    EXPECT_FALSE(result);
}

TEST(UT_PreviewPluginLoader, instance_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    QPluginLoader qLoader;
    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginLoaderList.push_back(&qLoader);
    QObject *result = loader.instance(0);

    EXPECT_FALSE(result);
}

TEST(UT_PreviewPluginLoader, instance_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    QPluginLoader qLoader;
    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginLoaderList.clear();
    QObject *result = loader.instance(0);

    EXPECT_FALSE(result);
}

TEST(UT_PreviewPluginLoader, pluginLoader)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    PreviewPluginLoader loader("UT_TEST", "UT");
    QPluginLoader *result = loader.pluginLoader("UT_TEST");

    EXPECT_FALSE(result);
}

TEST(UT_PreviewPluginLoader, pluginLoaderList)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    PreviewPluginLoader loader("UT_TEST", "UT");
    QList<QPluginLoader *> result = loader.pluginLoaderList("UT_TEST");

    EXPECT_TRUE(result.isEmpty());
}

TEST(UT_PreviewPluginLoader, keyMap)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    QPluginLoader qLoader;
    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginLoaderList.push_back(&qLoader);
    QMultiMap<int, QString> result = loader.keyMap();

    EXPECT_TRUE(result.isEmpty());
}

TEST(UT_PreviewPluginLoader, indexOf)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    QPluginLoader qLoader;
    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginLoaderList.push_back(&qLoader);
    int result = loader.indexOf("UT_TEST");

    EXPECT_TRUE(result == -1);
}

TEST(UT_PreviewPluginLoader, getAllIndexByKey)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, []{});

    QPluginLoader qLoader;
    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginLoaderList.push_back(&qLoader);
    QList<int> result = loader.getAllIndexByKey("UT_TEST");

    EXPECT_TRUE(result.isEmpty());
}

TEST(UT_PreviewPluginLoader, update_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef bool(QDir::*FuncType)(const QString &)const;
    stub.set_lamda(static_cast<FuncType>(&QDir::exists), [ &isOk ]{
        isOk = true;
        return false;
    });

    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginPaths.push_back("UT_TEST");
    loader.dptr->loadedPaths.clear();
    loader.update();

    EXPECT_TRUE(isOk);
}

TEST(UT_PreviewPluginLoader, update_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef bool(QDir::*FuncType)(const QString &)const;
    stub.set_lamda(static_cast<FuncType>(&QDir::exists), [ &isOk ]{
        isOk = true;
        return true;
    });

    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginPaths.push_back("UT_TEST");
    loader.dptr->loadedPaths.clear();
    loader.update();

    EXPECT_TRUE(isOk);
}

TEST(UT_PreviewPluginLoader, update_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef bool(QDir::*FuncType)(const QString &)const;
    stub.set_lamda(static_cast<FuncType>(&QDir::exists), []{
        return true;
    });
    stub.set_lamda(&QPluginLoader::load, [ &isOk ]{
        isOk = true;
        return true;
    });

    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.dptr->pluginPaths.push_back("UT_TEST");
    loader.dptr->loadedPaths.clear();
    loader.update();

    EXPECT_TRUE(isOk);
}

TEST(UT_PreviewPluginLoader, refreshAll)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&PreviewPluginLoader::update, [ &isOk ]{
        isOk = true;
    });

    PreviewPluginLoader loader("UT_TEST", "UT");
    loader.refreshAll();

    EXPECT_TRUE(isOk);
}
