// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugins/common/dfmplugin-utils/global/globaleventreceiver.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-framework/event/event.h>

#include <QProcess>

#include <stubext.h>
#include <gtest/gtest.h>

DPUTILS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_GlobalEventReceiverTest : public testing::Test
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

TEST_F(UT_GlobalEventReceiverTest, bug_129327_OpenAsAdmin)
{
    GlobalEventReceiver receiver;
    receiver.initEventConnect();
    using startDetachedType = bool (*)(const QString &, const QStringList &);
    stub.set_lamda(static_cast<startDetachedType>(QProcess::startDetached),
                   [](const QString &program, const QStringList &args) {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(program, "dde-file-manager-pkexec");
                       EXPECT_EQ(args.first(), "/tmp");
                       return true;
                   });
    dpfSignalDispatcher->publish(GlobalEventType::kOpenAsAdmin, QUrl::fromLocalFile("/tmp"));
}
