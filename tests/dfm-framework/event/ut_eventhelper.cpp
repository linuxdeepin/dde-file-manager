// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#define ARCH_MIPSEL
#include <dfm-framework/event/eventhelper.h>

#include <gtest/gtest.h>

#include <QObject>

DPF_USE_NAMESPACE

class UT_EventHelper : public testing::Test
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

using FuncType = std::function<int()>;
Q_DECLARE_METATYPE(FuncType);

TEST_F(UT_EventHelper, bug_182183_FunctionFailed)
{
    FuncType func = []() {
        return 1024;
    };
    const QVariant &var { QVariant::fromValue(func) };
#ifdef ARCH_MIPSEL
    auto ret = paramGenerator<FuncType>(var);
    EXPECT_TRUE(ret);
    EXPECT_EQ(1024, ret());
#endif
}
