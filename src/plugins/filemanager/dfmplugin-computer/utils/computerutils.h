/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef COMPUTERUTILS_H
#define COMPUTERUTILS_H

#include "dfmplugin_computer_global.h"

#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/dfm_global_defines.h"

#include <dfm-framework/dfm_framework_global.h>

#include <QString>
#include <QIcon>
#include <QUrl>
#include <QWaitCondition>

namespace dfmplugin_computer {

namespace EventNameSpace {
inline constexpr char kComputerEventSpace[] { DPF_MACRO_TO_STR(DPCOMPUTER_NAMESPACE) };
}

class ComputerUtils
{
public:
    inline static QString scheme()
    {
        return DFMBASE_NAMESPACE::Global::Scheme::kComputer;
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("computer-symbolic");
    }

    inline static QUrl rootUrl()
    {
        QUrl u;
        u.setScheme(scheme());
        u.setPath("/");
        return u;
    }

    inline static QString menuSceneName()
    {
        return "ComputerMenu";
    }

    static quint64 getWinId(QWidget *widget);

    static QUrl makeBlockDevUrl(const QString &id);
    static QString getBlockDevIdByUrl(const QUrl &url);
    static QUrl makeProtocolDevUrl(const QString &id);
    static QString getProtocolDevIdByUrl(const QUrl &url);
    static QUrl makeAppEntryUrl(const QString &filePath);
    static QUrl getAppEntryFileUrl(const QUrl &entryUrl);
    static QUrl makeStashedProtocolDevUrl(const QString &id);
    static QString getProtocolDevIdByStashedUrl(const QUrl &url);
    static QUrl convertToProtocolDevUrlFrom(const QUrl &stashedUrl);
    static QUrl convertToStashedUrlFrom(const QUrl &protocolDevUrl);
    static QUrl makeLocalUrl(const QString &path);
    static QUrl makeBurnUrl(const QString &id);   // device id

    static bool isPresetSuffix(const QString &suffix);
    static bool shouldSystemPartitionHide();
    static bool shouldLoopPartitionsHide();

    static bool sortItem(const QUrl &a, const QUrl &b);
    static bool sortItem(DFMEntryFileInfoPointer a, DFMEntryFileInfoPointer b);

    static QString deviceTypeInfo(DFMEntryFileInfoPointer info);
    static QWidget *devicePropertyDialog(const QUrl &url);
    static QUrl convertToDevUrl(const QUrl &url);

    static int getUniqueInteger();
    static bool checkGvfsMountExist(const QUrl &url, int timeout = 2000);
    static void setCursorState(bool busy = false);

    static QStringList allSystemUUIDs();
    static QList<QUrl> systemBlkDevUrlByUUIDs(const QStringList &uuids);
    static void diskHideToDConfig(int attr, const QVariant &var);
    static void diskHideToDSetting(const QString &cfgPath, const QString &cfgKey, const QVariant &var);
    static bool isEqualDiskHideConfig(const QVariant &varDConf, const QVariant &varDSet);

public:
    static bool contextMenuEnabled;   // TODO(xust) tmp solution, using GroupPolicy instead.

private:
    static QMutex mtxForCheckGvfs;
    static QWaitCondition condForCheckGvfs;
};

}

#endif   // COMPUTERUTILS_H
