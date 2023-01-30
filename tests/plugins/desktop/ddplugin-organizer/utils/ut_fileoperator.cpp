/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils/fileoperator_p.h"
#include "view/collectionview.h"

#include "dfm-base/dfm_event_defines.h"

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
