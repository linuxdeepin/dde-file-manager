// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abrecoveryhelper.h"
#include "inhibithelper.h"

#include <QFile>
#include <QElapsedTimer>

FILE_ENCRYPT_USE_NS

void abrecovery_helper::disableRecovery()
{
    QFile cfg("/etc/default/grub.d/13_deepin_ab_recovery.cfg");
    QByteArray cfgContents;
    if (cfg.exists()) {
        if (!cfg.open(QIODevice::ReadOnly)) {
            qWarning() << "cannot open recovery config!";
            return;
        }
        cfgContents = cfg.readAll();
        cfg.close();
    }

    QByteArrayList lines = cfgContents.split('\n');
    for (int i = 0; i < lines.count(); ++i) {
        QString line = lines.at(i);
        if (line.startsWith("#"))
            continue;

        if (line.contains("DISABLE_AB_ROLLBACK")) {
            if (line.contains("export DISABLE_AB_ROLLBACK=true")) {
                qInfo() << "rollback already disabled.";
                return;
            }

            lines.removeAt(i);
            break;
        }
    }
    lines.append("export DISABLE_AB_ROLLBACK=true\n");
    cfgContents = lines.join('\n');

    qDebug() << "the ab recovery contents:\n"
             << cfgContents;

    if (!cfg.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qWarning() << "cannot open recovery config to write!";
        return;
    }
    cfg.write(cfgContents);
    cfg.close();

    // update grub
    QElapsedTimer t;
    t.start();
    auto fd = inhibit_helper::inhibit(QObject::tr("Updating grub..."));
    qInfo() << "blocking reboot:" << fd.value().fileDescriptor();
    int ret = system("update-grub");
    qInfo() << "update grub costs" << t.elapsed() << "ms and result is" << ret;
}
