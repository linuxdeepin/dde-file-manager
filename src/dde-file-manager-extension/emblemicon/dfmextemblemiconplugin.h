// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    using LocationEmblemIcons = std::function<DFMExtEmblem(const std::string &, int)>;

public:
    DFMExtEmblemIconPlugin();
    ~DFMExtEmblemIconPlugin();

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
