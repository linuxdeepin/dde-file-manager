// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINALLYUTIL_H
#define FINALLYUTIL_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

#include <functional>

namespace dfmbase {

class FinallyUtil
{
    Q_DISABLE_COPY(FinallyUtil)

public:
    explicit FinallyUtil(std::function<void()> onExit);
    ~FinallyUtil();
    void dismiss(bool dismissed = true);

private:
    std::function<void()> exitFunc;
    bool hasDismissed { false };
};

}   // namespace dfmbase

#endif   // FINALLYUTIL_H
