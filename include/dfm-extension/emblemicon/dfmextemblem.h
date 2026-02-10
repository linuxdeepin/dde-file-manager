// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTEMBLEM_H
#define DFMEXTEMBLEM_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/emblemicon/dfmextemblemiconlayout.h>

#include <vector>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblemPrivate;
class DFMExtEmblem
{
    friend class DFMExtEmblemPrivate;

public:
    explicit DFMExtEmblem();
    ~DFMExtEmblem();
    DFMExtEmblem(const DFMExtEmblem &emblem);
    DFMExtEmblem &operator=(const DFMExtEmblem &emblem);
    void setEmblem(const std::vector<DFMExtEmblemIconLayout> &iconPaths);
    std::vector<DFMExtEmblemIconLayout> emblems() const;

private:
    DFMExtEmblemPrivate *d { nullptr };
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTEMBLEM_H
