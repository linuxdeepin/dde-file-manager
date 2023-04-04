// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/utils/finallyutil.h>

namespace dfmbase {

FinallyUtil::FinallyUtil(std::function<void()> onExit)
    : exitFunc(onExit)
{
}

FinallyUtil::~FinallyUtil()
{
    if (!hasDismissed)
        exitFunc();
}

void FinallyUtil::dismiss(bool dismissed)
{
    hasDismissed = dismissed;
}

}   // namespace dfmbase
