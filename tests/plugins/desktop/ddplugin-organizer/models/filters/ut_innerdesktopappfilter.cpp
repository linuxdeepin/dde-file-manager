// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/filters/innerdesktopappfilter.h"
#include <dfm-framework/dpf.h>
#include <dfm-base/utils/fileutils.h>

#include "stubext.h"

#include <gtest/gtest.h>
#include <QGSettings>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;
TEST(UT_InnerDesktopAppFilter, acceptInsert)
{
    QUrl url("temp");
    InnerDesktopAppFilter filter ;
    filter.hidden.insert("temp",true);
    filter.keys.insert("temp",QUrl("temp"));
    bool res = filter.acceptInsert(url);
    EXPECT_FALSE(res);
}

TEST(UT_InnerDesktopAppFilter, acceptReset)
{
    QList<QUrl> list{QUrl("temp")};
    InnerDesktopAppFilter filter ;
    filter.hidden.insert("temp",true);
    filter.keys.insert("temp",QUrl("temp"));
    QList<QUrl> res = filter.acceptReset(list);
    EXPECT_TRUE(res.isEmpty());
}

TEST(UT_InnerDesktopAppFilter, acceptRename)
{
    stub_ext::StubExt stub;
    typedef bool(*fun_type)(const QUrl&);
    stub.set_lamda((fun_type)(&InnerDesktopAppFilter::acceptInsert),[](const QUrl &){return false;});
    QUrl url1("url1");
    QUrl url2("url2");
    InnerDesktopAppFilter filter;
    bool res = filter.acceptRename(url1,url2);
    EXPECT_FALSE(res);
}

TEST(UT_InnerDesktopAppFilter, changed)
{
    stub_ext::StubExt stub;
    bool res = false;
    stub.set_lamda(&QGSettings::get,[&res](){return QVariant::fromValue(res);});
    bool call = false;
    stub.set_lamda(&InnerDesktopAppFilter::refreshModel,[&call](){call = true;});
    QString key("temp_str");
    InnerDesktopAppFilter filter;
    filter.hidden.insert("temp_str",false);
    filter.changed(key);
    EXPECT_TRUE(filter.hidden["temp_str"]);
    EXPECT_TRUE(call);

}
