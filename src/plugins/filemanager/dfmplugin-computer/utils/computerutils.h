// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERUTILS_H
#define COMPUTERUTILS_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

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
        u.setHost("");
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

    static QStringList allValidBlockUUIDs();
    static QList<QUrl> blkDevUrlByUUIDs(const QStringList &uuids);
    static QList<QVariantMap> allPreDefineItemCustomDatas();
    static bool isNativeDevice(const QString &suffix);

public:
    static bool contextMenuEnabled;   // TODO(xust) tmp solution, using GroupPolicy instead.

private:
    static QMutex mtxForCheckGvfs;
    static QWaitCondition condForCheckGvfs;
};

}

#endif   // COMPUTERUTILS_H
