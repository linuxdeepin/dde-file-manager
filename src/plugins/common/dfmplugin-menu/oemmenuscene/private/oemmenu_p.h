// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OEMMENU_P_H
#define OEMMENU_P_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/mimetype/dmimedatabase.h>

#include <DDesktopEntry>

#include <QObject>
#include <QTimer>
#include <QAction>
#include <QSharedPointer>
#include <QSharedData>

namespace dfmplugin_menu {

class OemMenu;
class OemMenuPrivate : public QSharedData
{
public:
    enum ArgType {
        kNoneArg = -1,
        kDirPath,
        kFilePath,
        kFilePaths,
        kUrlPath,
        kUrlPaths,
    };

    explicit OemMenuPrivate(OemMenu *qq);
    ~OemMenuPrivate();

    QStringList getValues(const Dtk::Core::DDesktopEntry &entry, const QString &key, const QString &aliasKey, const QString &section = "Desktop Entry", const QStringList &whiteList = {}) const;

    bool isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes) const;
    bool isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes) const;
    bool isActionShouldShow(const QAction *action, bool onDesktop) const;
    bool isSchemeSupport(const QAction *action, const QUrl &url) const;
    bool isSuffixSupport(const QAction *action, FileInfoPointer fileInfo, const bool allEx7z = false) const;
    bool isAllEx7zFile(const QList<QUrl> &files) const;
    bool isValid(const QAction *action, FileInfoPointer fileInfo, const bool onDesktop, const bool allEx7z = false) const;

    void clearSubMenus();
    void setActionProperty(QAction *const action, const Dtk::Core::DDesktopEntry &entry, const QString &key, const QString &section = "Desktop Entry") const;
    QStringList splitCommand(const QString &cmd);
    QPair<ArgType, int> execDynamicArg(const QStringList &args, int index) const;
    QStringList replace(QStringList &args, const QString &before, const QString &after) const;
    QStringList replaceList(QStringList &args, const QString &before, const QStringList &after) const;
    QStringList urlListToLocalFile(const QList<QUrl> &files) const;
    QString urlToString(const QUrl &file) const;
    QStringList urlListToString(const QList<QUrl> &files) const;
    void appendParentMineType(const QStringList &parentmimeTypes, QStringList &mimeTypes) const;
    QStringList applyDynamicArg(const QStringList &args, ArgType type, const QUrl &dir, const QUrl &focus, const QList<QUrl> &files) const;

public:
    QSharedPointer<QTimer> delayedLoadFileTimer;
    QSharedPointer<QObject> menuActionHolder;
    QMap<QString, QList<QAction *>> actionListByType;
    QList<QMenu *> subMenus;
    dfmbase::DMimeDatabase mimeDatabase;

    QStringList oemMenuPath;
    QStringList menuTypes;
    QStringList actionProperties;

    OemMenu *q;
};

}

#endif   // OEMMENU_P_H
