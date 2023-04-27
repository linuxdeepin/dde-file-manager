// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "tag.h"
#include "utils/tagmanager.h"
#include "widgets/tagwidget.h"
#include "widgets/private/tagwidget_p.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;
using namespace dpf;

Q_DECLARE_METATYPE(CustomViewExtensionView)

class TagTest : public testing::Test
{

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    Tag ins;
};

TEST_F(TagTest, Start)
{
    typedef QVariant (EventChannelManager::*Push1)(const QString &, const QString &, CustomViewExtensionView, int &);
    typedef QVariant (EventChannelManager::*Push2)(const QString &, const QString &, CustomViewExtensionView, QString &, int &);
    typedef QVariant (EventChannelManager::*Push3)(const QString &, const QString &, QString, QStringList &);
    auto push1 = static_cast<Push1>(&EventChannelManager::push);
    auto push2 = static_cast<Push2>(&EventChannelManager::push);
    auto push3 = static_cast<Push3>(&EventChannelManager::push);
    stub.set_lamda(push1, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push2, [] { __DBG_STUB_INVOKE__ return QVariant(); });
    stub.set_lamda(push3, [] { __DBG_STUB_INVOKE__ return QVariant(); });

    EXPECT_TRUE(ins.start());
}

TEST_F(TagTest, createTagWidget)
{
    auto func = static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() -> bool { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&TagManager::getTagsByUrls, []() {
        __DBG_STUB_INVOKE__
        return QVariant();
    });
    EXPECT_TRUE(ins.createTagWidget(QUrl("file://hello/world")));
}
