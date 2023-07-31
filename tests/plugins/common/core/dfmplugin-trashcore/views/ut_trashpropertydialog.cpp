// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dfmplugin_trashcore_global.h"
#include "plugins/common/core/dfmplugin-trashcore/trashcore.h"
#include "plugins/common/core/dfmplugin-trashcore/views/trashpropertydialog.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-io/denumerator.h>

#include <gtest/gtest.h>

DPTRASHCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_TrashPropertyDialog : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
    ~UT_TrashPropertyDialog() override {}
};

TEST_F(UT_TrashPropertyDialog, testTrashPropertyDialog)
{
    TrashCore core;
    core.initialize();

    TrashPropertyDialog widget;

    widget.updateLeftInfo(0);
    widget.updateLeftInfo(1);
    widget.updateLeftInfo(2);
    widget.calculateSize();

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(DDialog,showEvent), []{ __DBG_STUB_INVOKE__ });
    widget.showEvent(nullptr);
}
