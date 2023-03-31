// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/fileoperator_p.h"
#include "view/collectionview.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <stubext.h>

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#if 0
TEST(FileOperator, undoFiles)
{
    FileOperator fo;
    CollectionView view("ss", nullptr);

    stub_ext::StubExt stub;
    EventType iType;
    WId iWid;

    typedef bool (EventDispatcherManager::*PublishRevert)(int, WId, nullptr_t);
    auto func = static_cast<PublishRevert>(&EventDispatcherManager::publish);
    stub.set_lamda(func,
                   [&iType, &iWid](EventDispatcherManager *, EventType type, WId id, nullptr_t ){
        iType = type;
        iWid = id;
        return true;
    });

    fo.undoFiles(&view);
    EXPECT_EQ(iType, GlobalEventType::kRevocation);
    EXPECT_EQ(iWid, view.winId());
}
#endif
