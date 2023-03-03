// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "dfm-framework/lifecycle/private/pluginmanager_p.h"
#include "dfm-framework/lifecycle/private/pluginmetaobject_p.h"

#include <dfm-framework/lifecycle/pluginmetaobject.h>

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
