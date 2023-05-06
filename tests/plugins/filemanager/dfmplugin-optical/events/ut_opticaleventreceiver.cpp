// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "stubext.h"

#include "plugins/filemanager/dfmplugin-optical/events/opticaleventreceiver.h"

#include <QUrl>

#include <gtest/gtest.h>

DPOPTICAL_USE_NAMESPACE

class UT_OpticalEventReceiver : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_OpticalEventReceiver, bug_198941_DragToDisc)
{
    QList<QUrl> from;
    from << QUrl::fromLocalFile("/tmp/test");
    QUrl to { "burn:///" };
    Qt::DropAction act { Qt::IgnoreAction };
    OpticalEventReceiver::instance().handleCheckDragDropAction(from, to, &act);
    EXPECT_EQ(act, Qt::CopyAction);
}
