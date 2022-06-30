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

#include "core.h"
#include "utils/windowutils.h"
#include "screen/screenproxyqt.h"
#include "frame/windowframe.h"

#include "interfaces/screen/abstractscreen.h"
#include "stubext.h"

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DDPCORE_USE_NAMESPACE

TEST(TestEventHandle, init_event)
{
    EventHandle *handle = new EventHandle;

    stub_ext::StubExt stub;
    stub.set_lamda(&WindowUtils::isWayLand, [](){
        return false;
    });

    bool reset = false;
    stub.set_lamda(VADDR(ScreenProxyQt, reset), [&reset](){
        reset = true;
    });

    bool init = false;
    stub.set_lamda(&WindowFrame::init, [&init](){
        init = true;
        return true;
    });

    delete handle;
}
