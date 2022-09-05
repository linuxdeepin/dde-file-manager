// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
