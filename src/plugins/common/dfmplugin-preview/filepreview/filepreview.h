// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEW_H
#define FILEPREVIEW_H

#include "dfmplugin_filepreview_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_filepreview {
class FilePreview : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "filepreview.json")

    DPF_EVENT_NAMESPACE(DPFILEPREVIEW_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_PreviewDialog_Show)

    DPF_EVENT_REG_SIGNAL(signal_ThumbnailDisplay_Changed)

public:
    virtual void initialize() override;
    virtual bool start() override;

private Q_SLOTS:
    void onConfigChanged(const QString &cfg, const QString &key);
};
}   // namespace dfmplugin_filepreview
#endif   // FILEPREVIEW_H
