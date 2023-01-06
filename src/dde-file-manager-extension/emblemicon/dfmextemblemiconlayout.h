// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTEMBLEMICONLAYOUT_H
#define DFMEXTEMBLEMICONLAYOUT_H

#include "dfm-extension-global.h"

#include <string>
#include <memory>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblemIconLayoutPrivate;
class DFMExtEmblemIconLayout
{
    friend class DFMExtEmblemIconLayoutPrivate;

public:
    enum class LocationType : uint8_t
    {
        BottomRight = 0,
        BottomLeft,
        TopLeft,
        TopRight,
        Custom = 0xff
    };

    explicit DFMExtEmblemIconLayout(LocationType type,
                           const std::string &path,
                           int x = 0, int y = 0);
    ~DFMExtEmblemIconLayout();
    DFMExtEmblemIconLayout(const DFMExtEmblemIconLayout &emblem);
    DFMExtEmblemIconLayout &operator=(const DFMExtEmblemIconLayout &emblem);
    LocationType locationType() const;
    std::string iconPath() const;
    int x() const;
    int y() const;

private:
    DFMExtEmblemIconLayoutPrivate *d { nullptr };
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTEMBLEMICONLAYOUT_H
