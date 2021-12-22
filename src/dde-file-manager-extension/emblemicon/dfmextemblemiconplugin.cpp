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
#include "dfmextemblemiconplugin.h"

#include <assert.h>

BEGEN_DFMEXT_NAMESPACE
class DFMExtEmblemIconPluginPrivate
{
public:
    dfmext::DFMExtEmblemIconPlugin::EmblemIcons emblemIcons;
    dfmext::DFMExtEmblemIconPlugin::LocationEmblemIcons locationEmblemIcons;
};
END_DFMEXT_NAMESPACE

USING_DFMEXT_NAMESPACE

DFMExtEmblemIconPlugin::DFMExtEmblemIconPlugin()
    : d(new DFMExtEmblemIconPluginPrivate)
{
}

DFMExtEmblemIconPlugin::~DFMExtEmblemIconPlugin()
{
    delete d;
}

DFMExtEmblemIconPlugin::IconsType DFMExtEmblemIconPlugin::emblemIcons(const std::string &fileUrl) const
{
    if (d->emblemIcons) {
        return d->emblemIcons(fileUrl);
    } else {
        IconsType icon;
        return icon;
    }
}

void DFMExtEmblemIconPlugin::registerEmblemIcons(const EmblemIcons &func)
{
    if (!d->emblemIcons)
        d->emblemIcons = func;
}

DFMExtEmblem DFMExtEmblemIconPlugin::locationEmblemIcons(const std::string &fileUrl, int systemIconCount) const
{
    if (d->locationEmblemIcons) {
        return d->locationEmblemIcons(fileUrl, systemIconCount);
    } else {
        DFMExtEmblem icon;
        return icon;
    }
}

void DFMExtEmblemIconPlugin::registerLocationEmblemIcons(const DFMExtEmblemIconPlugin::LocationEmblemIcons &func)
{
    if (!d->locationEmblemIcons)
        d->locationEmblemIcons = func;
}
