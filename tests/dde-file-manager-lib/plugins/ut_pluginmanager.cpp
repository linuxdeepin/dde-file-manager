/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include <QDir>
#include <gtest/gtest.h>
#include <QPluginLoader>
#include "view/viewinterface.h"
#include "menu/menuinterface.h"
#include "preview/previewinterface.h"

#define private public
#define protected public

#include "plugins/pluginmanager.h"
#include "interfaces/dfmglobal.h"
#include "stub.h"

namespace  {
    class PluginManagerTest : public testing::Test
    {
    public:
        void SetUp() override {
            p_manager = new PluginManager;
        }
        void TearDown() override {
            delete p_manager;
            p_manager = nullptr;
        }

        PluginManager   *p_manager = nullptr;
    };
}

TEST_F(PluginManagerTest, get_plugin_dir)
{
    ASSERT_NE(p_manager, nullptr);

    QString result = p_manager->PluginDir();
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(PluginManagerTest, load_plugin_success)
{
    ASSERT_NE(p_manager, nullptr);

    DFMGlobal::PluginLibraryPaths.append(QStringList()<<QString("/"));
    p_manager->loadPlugin();

    QStringList (*ut_entryList)() = [](){return QStringList() << QString("test1");};
    Stub stub;
    stub.set((QStringList(QDir::*)(QDir::Filters,QDir::SortFlags)const)ADDR(QDir, entryList), ut_entryList);

    static PropertyDialogExpandInfoInterface *obj = new PropertyDialogExpandInfoInterface;
    PropertyDialogExpandInfoInterface* (*ut_instance)() = [](){return obj;};
    stub.set(ADDR(QPluginLoader, instance), ut_instance);
    p_manager->loadPlugin();
    obj->deleteLater();
}

TEST_F(PluginManagerTest, get_expandInfo_interfaces)
{
    ASSERT_NE(p_manager, nullptr);

    QList<PropertyDialogExpandInfoInterface *> result = p_manager->getExpandInfoInterfaces();
    EXPECT_EQ(result.count(), p_manager->d_func()->expandInfoInterfaces.count());
}

TEST_F(PluginManagerTest, get_view_interfaces)
{
    ASSERT_NE(p_manager, nullptr);

    QList<ViewInterface *> result = p_manager->getViewInterfaces();
    EXPECT_EQ(result.count(), p_manager->d_func()->viewInterfaces.count());
}

TEST_F(PluginManagerTest, get_view_interfacesMap)
{
    ASSERT_NE(p_manager, nullptr);

    QMap<QString, ViewInterface *> result = p_manager->getViewInterfacesMap();
    EXPECT_EQ(result.count(), p_manager->d_func()->viewInterfacesMap.count());
}

TEST_F(PluginManagerTest, get_preview_inteefaces)
{
    ASSERT_NE(p_manager, nullptr);

    QList<PreviewInterface *> result = p_manager->getPreviewInterfaces();
    EXPECT_EQ(result.count(), p_manager->d_func()->previewInterfaces.count());
}

TEST_F(PluginManagerTest, get_view_interfaceByScheme)
{
    ASSERT_NE(p_manager, nullptr);

    QString scheme = "file";

    p_manager->d_func()->viewInterfacesMap.clear();
    ViewInterface * result = p_manager->getViewInterfaceByScheme(scheme);
    EXPECT_EQ(result, nullptr);

    ViewInterface obj;
    p_manager->d_func()->viewInterfacesMap.insert(scheme, &obj);
    result = p_manager->getViewInterfaceByScheme(scheme);
    EXPECT_EQ(result, &obj);
}
