// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dfmplugin_trashcore_global.h"
#include "plugins/common/core/dfmplugin-trashcore/trashcore.h"
#include "plugins/common/core/dfmplugin-trashcore/utils/trashcorehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-io/denumerator.h>

#include <gtest/gtest.h>

DPTRASHCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_TrashCoreHelper : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
    ~UT_TrashCoreHelper() override {}
};

TEST_F(UT_TrashCoreHelper, testTrashCoreHelper)
{
    TrashCore core;
    core.initialize();

    EXPECT_EQ(TrashCoreHelper::scheme(), "trash");
    EXPECT_EQ(TrashCoreHelper::icon().name(), "user-trash-symbolic");
    qInfo() << TrashCoreHelper::rootUrl();
    EXPECT_EQ(TrashCoreHelper::rootUrl(), QUrl("trash:/"));

    stub_ext::StubExt stub;
    stub.set_lamda(&UniversalUtils::urlEquals, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&FileUtils::isTrashDesktopFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_EQ(nullptr, TrashCoreHelper::createTrashPropertyDialog(QUrl()));

    stub.set_lamda(&FileUtils::isTrashDesktopFile, []{ __DBG_STUB_INVOKE__ return true;});
    auto widget = TrashCoreHelper::createTrashPropertyDialog(QUrl());
    EXPECT_EQ(widget, TrashCoreHelper::createTrashPropertyDialog(QUrl()));
    widget->deleteLater();

    stub.set_lamda(&dfmio::DEnumerator::hasNext, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(TrashCoreHelper::calculateTrashRoot().first);

    int index = 0;
    stub.set_lamda(&dfmio::DEnumerator::hasNext, [&index]{ __DBG_STUB_INVOKE__ index++; return index <= 1;});
    stub.set_lamda(&dfmio::DEnumerator::next, []{ __DBG_STUB_INVOKE__ return QUrl();});
    EXPECT_FALSE(TrashCoreHelper::calculateTrashRoot().first);

    index = 0;
    stub.set_lamda(&dfmio::DEnumerator::next, []{ __DBG_STUB_INVOKE__ return QUrl("trash:///");});
    EXPECT_TRUE(TrashCoreHelper::calculateTrashRoot().second == 1);
}
