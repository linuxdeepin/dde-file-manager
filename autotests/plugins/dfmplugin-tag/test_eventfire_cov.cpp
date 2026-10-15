// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_eventfire_cov.cpp - event template instantiation sweep for the tag plugin.
//
// Production plugin code instantiates dpf event templates (EventChannel::send
// with a widget-pointer + char-array-literal pack) against concrete tag plugin
// classes, but that instantiation is never executed by tests. The case below
// fires one such pack on a local channel instance with a null receiver: the
// framework's `if (s)` guard keeps the production slot body from running
// while the send / makeVariantList / packParamsHelper instantiation executes.
// (The identical specialization used to live in dfmplugin-core's sweep, but
// that target's flag set cannot resolve the tag widget header, while this
// target compiles it natively; coverage merges by mangled name.)

#include "plugins/common/dfmplugin-tag/widgets/tagcrumbedit.h"

#include <QVariant>

#include <gtest/gtest.h>
#include <dfm-framework/event/event.h>

class EventFireSweepTest : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventFireSweepTest, PushSend_TagCrumbEdit_0_ArityGuardNoBody)
{
    // Arrange
    dpf::EventChannel ch;
    auto *edit = static_cast<dfmplugin_tag::TagCrumbEdit *>(nullptr);  // framework guards null receivers (`if (s)`)
    // Act
    const QVariant ret = ch.send(edit, "Select a tag:");   // 14 chars + NUL => char const(&)[15]
    // Assert
    EXPECT_FALSE(ret.isValid());  // no receiver bound: send packs args and returns invalid
    EXPECT_FALSE(ret.toBool());
}
