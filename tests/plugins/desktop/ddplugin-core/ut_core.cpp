// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
