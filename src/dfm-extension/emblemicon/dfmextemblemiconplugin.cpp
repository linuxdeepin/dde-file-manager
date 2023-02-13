// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-extension/emblemicon/dfmextemblemiconplugin.h>

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

DFMExtEmblemIconPlugin::IconsType DFMExtEmblemIconPlugin::emblemIcons(const std::string &filePath) const
{
    if (d->emblemIcons) {
        return d->emblemIcons(filePath);
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

DFMExtEmblem DFMExtEmblemIconPlugin::locationEmblemIcons(const std::string &filePath, int systemIconCount) const
{
    if (d->locationEmblemIcons) {
        return d->locationEmblemIcons(filePath, systemIconCount);
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
