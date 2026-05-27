// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>
#include <QIcon>

#define private public
#define protected public
#include "events/emblemeventsequence.h"
#undef private
#undef protected

#include <dfm-framework/event/event.h>
#include <QApplication>

using namespace dfmplugin_emblem;
DPF_USE_NAMESPACE;

class UT_EmblemEventSequence : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        if (!qApp) {
            int argc = 1;
            char *argv[] = { const_cast<char *>("test") };
            new QApplication(argc, argv);
        }

        sequence = EmblemEventSequence::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    EmblemEventSequence *sequence { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_EmblemEventSequence, Instance_ReturnsSingleton)
{
    auto instance1 = EmblemEventSequence::instance();
    auto instance2 = EmblemEventSequence::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_EmblemEventSequence, DoFetchExtendEmblems_CallsHookSequence)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QList<QIcon> emblems;

    bool hookCalled = false;
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl, QList<QIcon> *&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [&] {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return false;
    });

    bool result = sequence->doFetchExtendEmblems(url, &emblems);

    EXPECT_TRUE(hookCalled);
    EXPECT_FALSE(result);
}

TEST_F(UT_EmblemEventSequence, DoFetchCustomEmblems_CallsHookSequence)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QList<QIcon> emblems;

    bool hookCalled = false;
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl, QList<QIcon> *&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    stub.set_lamda(runFunc, [&] {
        __DBG_STUB_INVOKE__
        hookCalled = true;
        return false;
    });

    bool result = sequence->doFetchCustomEmblems(url, &emblems);

    EXPECT_TRUE(hookCalled);
    EXPECT_FALSE(result);
}
