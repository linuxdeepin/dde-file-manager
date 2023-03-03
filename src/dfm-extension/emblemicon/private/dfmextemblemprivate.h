// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTEMBLEMPRIVATE_H
#define DFMEXTEMBLEMPRIVATE_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/emblemicon/dfmextemblem.h>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblem;
class DFMExtEmblemPrivate
{
    friend class DFMExtEmblem;

public:
    explicit DFMExtEmblemPrivate(DFMExtEmblem *qq);
    virtual ~DFMExtEmblemPrivate();
    void setEmblem(const std::vector<DFMExtEmblemIconLayout> &iconPath);
    std::vector<DFMExtEmblemIconLayout> emblems() const;

private:
    DFMExtEmblem *const q;
    std::vector<DFMExtEmblemIconLayout> emblemContainer {};
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTEMBLEMPRIVATE_H
