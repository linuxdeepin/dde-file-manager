// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "events/recenteventreceiver.h"
#include <dfm-base/base/application/application.h>
#include <dfm-framework/event/event.h>
#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>
DPF_USE_NAMESPACE
using namespace dfmplugin_recent;
Q_DECLARE_METATYPE(QString *)

class RecentEventReceiverTest : public testing::Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(RecentEventReceiverTest, handleWindowUrlChanged)
{
    stub_ext::StubExt st;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, quint64, const QUrl &);
    auto push = static_cast<Push>(&EventChannelManager::push);
    st.set_lamda(push, [] { return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(RecentEventReceiver::instance()->handleWindowUrlChanged(123, QUrl("recent:/hello/world")););
}
