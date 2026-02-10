// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKENCRYPTENTRY_H
#define DISKENCRYPTENTRY_H

#include "dfmplugin_disk_encrypt_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_diskenc {

class DFMPLUGIN_DISK_ENCRYPT_EXPORT DiskEncryptEntry : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "diskencryptentry.json")

    // Plugin interface
public:
    virtual void initialize() override;
    virtual bool start() override;

private Q_SLOTS:
    void initEncryptEvents();

private:
    void onComputerMenuSceneAdded(const QString &scene);
    void processUnfinshedDecrypt(const QString &device);
};
}

#endif   // DISKENCRYPTENTRY_H
