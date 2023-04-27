// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/core/dfmplugin-computer/delegate/computeritemdelegate.h"
#include "plugins/filemanager/core/dfmplugin-computer/views/computerview.h"
#include "plugins/filemanager/core/dfmplugin-computer/models/computermodel.h"

#include <QPainter>

#include <gtest/gtest.h>

DPCOMPUTER_USE_NAMESPACE

class UT_ComputerItemDelegate : public testing::Test
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

TEST_F(UT_ComputerItemDelegate, bug_182578_SizeOverflow)
{
    ComputerItemDelegate delegate;
    ComputerView view({});
    delegate.view = &view;
    QPainter painter;
    QStyleOptionViewItem option;

    stub.set_lamda(&QModelIndex::data, [](QModelIndex *, int arole) {
        __DBG_STUB_INVOKE__
        QList<int> visibaleRoles { ComputerModel::kTotalSizeVisiableRole,
                                   ComputerModel::kUsedSizeVisiableRole };
        if (visibaleRoles.contains(arole))
            return QVariant::fromValue(true);
        if (arole == ComputerModel::kProgressVisiableRole)
            return QVariant::fromValue(false);

        // unexpted data: usage > total
        if (arole == ComputerModel::kSizeUsageRole)
            return QVariant::fromValue(2048);
        if (arole == ComputerModel::kSizeTotalRole)
            return QVariant::fromValue(1024);

        return QVariant();
    });

    auto drawText = static_cast<void (QPainter::*)(const QRect &, int, const QString &, QRect *)>(&QPainter::drawText);
    stub.set_lamda(drawText, [](QPainter *, const QRect &, int, const QString &text, QRect *) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ("0 B/1 KB", text);
    });

    delegate.drawDeviceDetail(&painter, option, {});
}
