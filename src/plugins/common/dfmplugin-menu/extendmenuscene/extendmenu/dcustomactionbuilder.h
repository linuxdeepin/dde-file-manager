// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCUSTOMACTIONBUILDER_H
#define DCUSTOMACTIONBUILDER_H

#include "dfmplugin_menu_global.h"
#include "dcustomactiondata.h"
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>
#include <QIcon>
#include <QFontMetrics>
#include <QUrl>
#include <QAction>
#include <QMenu>

namespace dfmplugin_menu {

class DCustomActionBuilder : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionBuilder(QObject *parent = nullptr);
    QAction *buildAciton(const DCustomActionData &actionData, QWidget *parentForSubmenu) const;
    void setActiveDir(const QUrl &dir);
    void setFocusFile(const QUrl &file);
    QString getCompleteSuffix(const QString &fileName, const QString &suf);
    static DCustomActionDefines::ComboType checkFileCombo(const QList<QUrl> &files);
    static DCustomActionDefines::ComboType checkFileComboWithFocus(const QUrl &focus, const QList<QUrl> &files);
    static QList<DCustomActionEntry> matchFileCombo(const QList<DCustomActionEntry> &rootActions,
                                                    DCustomActionDefines::ComboTypes type);
    static QList<DCustomActionEntry> matchActions(const QList<QUrl> &selects,
                                                  QList<DCustomActionEntry> oriActions);
    static QPair<QString, QStringList> makeCommand(const QString &cmd, DCustomActionDefines::ActionArg arg,
                                                   const QUrl &dir, const QUrl &foucs, const QList<QUrl> &files);
    static QStringList splitCommand(const QString &cmd);

private:
    static bool isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes);
    static bool isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes);
    static bool isSchemeSupport(const DCustomActionEntry &action, const QUrl &url);
    static bool isSuffixSupport(const DCustomActionEntry &action, FileInfoPointer fileInfo);
    static void appendAllMimeTypes(const FileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes);
    static void appendParentMimeType(const QStringList &parentmimeTypes, QStringList &mimeTypes);

protected:
    QAction *createMenu(const DCustomActionData &actionData, QWidget *parentForSubmenu) const;
    QAction *createAciton(const DCustomActionData &actionData) const;
signals:

public slots:
protected:
    QIcon getIcon(const QString &iconName) const;
    QString makeName(const QString &name, DCustomActionDefines::ActionArg arg) const;

private:
    QFontMetrics fontMetriecs;
    QString dirName;
    QUrl dirPath;
    QString fileBaseName;
    QString fileFullName;
    QUrl filePath;
    dfmbase::DMimeDatabase mimeDatabase;
};

}

#endif   // DCUSTOMACTIONBUILDER_H
