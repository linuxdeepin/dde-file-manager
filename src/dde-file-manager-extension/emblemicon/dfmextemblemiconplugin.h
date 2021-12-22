/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: yanghaolvwujun@uniontech.com>
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
#ifndef DFMEMBLEMICON_H
#define DFMEMBLEMICON_H

#include "dfm-extension-global.h"
#include "emblemicon/dfmextemblem.h"

#include <functional>
#include <vector>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblemIconPluginPrivate;
class DFMExtEmblemIconPlugin
{
    DFM_DISABLE_COPY(DFMExtEmblemIconPlugin)
public:
    using IconsType = std::vector<std::string>;
    using EmblemIcons = std::function<IconsType(const std::string &)>;
    using LocationEmblemIcons = std::function<DFMExtEmblem (const std::string &, int)>;

public:
    DFMExtEmblemIconPlugin();
    virtual ~DFMExtEmblemIconPlugin();

    // Note: If the corner mark set by emblemIcons conflicts with the corner mark position set by locationEmblemIcons,
    // the conflict position will only display the corner mark set by locationEmblemIcons
    DFM_FAKE_VIRTUAL IconsType emblemIcons(const std::string &fileUrl) const;
    void registerEmblemIcons(const EmblemIcons &func);

    DFM_FAKE_VIRTUAL DFMExtEmblem locationEmblemIcons(const std::string &fileUrl, int systemIconCount) const;
    void registerLocationEmblemIcons(const LocationEmblemIcons &func);

private:
    DFMExtEmblemIconPluginPrivate *d { nullptr };
};
END_DFMEXT_NAMESPACE

#endif   // DFMEMBLEMICON_H
