// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abrecoveryhelper.h"
#include "inhibithelper.h"

#include <QFile>
#include <QElapsedTimer>

FILE_ENCRYPT_USE_NS

void abrecovery_helper::disableRecovery()
{
    qInfo() << "[abrecovery_helper::disableRecovery] Starting AB recovery disable process";
    
    QFile cfg("/etc/default/grub.d/13_deepin_ab_recovery.cfg");
    QByteArray cfgContents;
    if (cfg.exists()) {
        qInfo() << "[abrecovery_helper::disableRecovery] Found existing recovery config file";
        if (!cfg.open(QIODevice::ReadOnly)) {
            qCritical() << "[abrecovery_helper::disableRecovery] Failed to open recovery config file for reading";
            return;
        }
        cfgContents = cfg.readAll();
        cfg.close();
    } else {
        qInfo() << "[abrecovery_helper::disableRecovery] No existing recovery config file found, creating new one";
    }

    QByteArrayList lines = cfgContents.split('\n');
    for (int i = 0; i < lines.count(); ++i) {
        QString line = lines.at(i);
        if (line.startsWith("#"))
            continue;

        if (line.contains("DISABLE_AB_ROLLBACK")) {
            if (line.contains("export DISABLE_AB_ROLLBACK=true")) {
                qInfo() << "[abrecovery_helper::disableRecovery] AB rollback already disabled, no changes needed";
                return;
            }

            lines.removeAt(i);
            qInfo() << "[abrecovery_helper::disableRecovery] Removed existing DISABLE_AB_ROLLBACK line:" << line;
            break;
        }
    }
    lines.append("export DISABLE_AB_ROLLBACK=true\n");
    cfgContents = lines.join('\n');

    qDebug() << "[abrecovery_helper::disableRecovery] Updated AB recovery configuration contents:\n" << cfgContents;

    if (!cfg.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qCritical() << "[abrecovery_helper::disableRecovery] Failed to open recovery config file for writing";
        return;
    }
    cfg.write(cfgContents);
    cfg.close();

    qInfo() << "[abrecovery_helper::disableRecovery] Recovery config file updated successfully";

    // update grub
    QElapsedTimer t;
    t.start();
    auto fd = inhibit_helper::inhibit(QObject::tr("Updating grub..."));
    qInfo() << "[abrecovery_helper::disableRecovery] System shutdown/sleep inhibited for GRUB update, fd:" << fd.value().fileDescriptor();
    
    int ret = system("update-grub");
    auto elapsed = t.elapsed();
    
    if (ret == 0) {
        qInfo() << "[abrecovery_helper::disableRecovery] GRUB update completed successfully in" << elapsed << "ms";
    } else {
        qCritical() << "[abrecovery_helper::disableRecovery] GRUB update failed with error code:" << ret << "elapsed time:" << elapsed << "ms";
    }
}
