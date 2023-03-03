// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRAMELOGMANAGER_P_H
#define FRAMELOGMANAGER_P_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/log/framelogmanager.h>

DPF_BEGIN_NAMESPACE

class FrameLogManagerPrivate
{
public:
    explicit FrameLogManagerPrivate(FrameLogManager *qq);
    void initFilterAppender();
    FilterAppender *filterAppender();

public:
    FilterAppender *curFilterAppender { nullptr };

    FrameLogManager *const q;
};

DPF_END_NAMESPACE

#endif   // FRAMELOGMANAGER_P_H
