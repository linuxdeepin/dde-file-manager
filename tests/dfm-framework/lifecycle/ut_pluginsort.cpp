/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include <gtest/gtest.h>

#define private public
#include "lifecycle/private/pluginmanager_p.h"
#include "lifecycle/private/pluginmetaobject_p.h"
#include "lifecycle/pluginmetaobject.h"

DPF_USE_NAMESPACE
class UT_PluginSort : public testing::Test
{
public:
    virtual void SetUp() override
    {
        A->d->name = "A";
        B->d->name = "B";
        C->d->name = "C";
        D->d->name = "D";
        E->d->name = "E";
        F->d->name = "F";
        G->d->name = "G";
    }

    virtual void TearDown() override
    {
    }

    PluginMetaObjectPointer A { new PluginMetaObject };
    PluginMetaObjectPointer B { new PluginMetaObject };
    PluginMetaObjectPointer C { new PluginMetaObject };
    PluginMetaObjectPointer D { new PluginMetaObject };
    PluginMetaObjectPointer E { new PluginMetaObject };
    PluginMetaObjectPointer F { new PluginMetaObject };
    PluginMetaObjectPointer G { new PluginMetaObject };
};

TEST_F(UT_PluginSort, test_rhombus_depends)
{

    PluginDependGroup group { { A, B }, { A, C }, { B, D }, { C, D } };
    QMap<QString, PluginMetaObjectPointer> src { { A->name(), A }, { B->name(), B }, { C->name(), C }, { D->name(), D } };
    QQueue<PluginMetaObjectPointer> dest;
    EXPECT_TRUE(PluginManagerPrivate::doPluginSort(group, src, &dest));
    EXPECT_EQ(src.size(), dest.size());
    EXPECT_EQ(dest.at(0)->name(), "A");
    EXPECT_EQ(dest.at(3)->name(), "D");
    QStringList mid { "B", "C" };
    EXPECT_TRUE(mid.contains(dest.at(1)->name()));
    EXPECT_TRUE(mid.contains(dest.at(2)->name()));
}

TEST_F(UT_PluginSort, test_rhombus_depends_circle)
{
    PluginDependGroup group { { A, B }, { A, C }, { B, D }, { D, B } };
    QMap<QString, PluginMetaObjectPointer> src { { A->name(), A }, { B->name(), B }, { C->name(), C }, { D->name(), D } };
    QQueue<PluginMetaObjectPointer> dest;
    EXPECT_FALSE(PluginManagerPrivate::doPluginSort(group, src, &dest));
    EXPECT_EQ(dest.at(0)->name(), "A");
    EXPECT_EQ(dest.at(1)->name(), "C");

    group = { { A, B }, { A, C }, { B, D }, { A, A } };
    EXPECT_FALSE(PluginManagerPrivate::doPluginSort(group, src, &dest));
}

TEST_F(UT_PluginSort, test_rhombus_complex)
{
    PluginDependGroup group { { A, C }, { A, D }, { A, G }, { B, C }, { D, B }, { D, E }, { F, E }, { G, F }, { B, G } };
    QMap<QString, PluginMetaObjectPointer> src { { A->name(), A }, { B->name(), B }, { C->name(), C }, { D->name(), D }, { E->name(), E }, { F->name(), F }, { G->name(), G } };
    QQueue<PluginMetaObjectPointer> dest;
    EXPECT_TRUE(PluginManagerPrivate::doPluginSort(group, src, &dest));
    QStringList trueRet { "ADBCGFE", "ADBGCFE" };
    QString ret;
    for (auto v : dest) {
        ret += v->name();
    }
    EXPECT_TRUE(trueRet.contains(ret));
}
