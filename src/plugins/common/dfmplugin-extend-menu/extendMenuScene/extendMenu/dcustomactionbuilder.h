/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

#ifndef DCUSTOMACTIONBUILDER_H
#define DCUSTOMACTIONBUILDER_H

#include "dfmplugin_extend_menu_global.h"
#include "dcustomactiondata.h"
#include "dfm-base/mimetype/dmimedatabase.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>
#include <QIcon>
#include <QFontMetrics>
#include <QUrl>
#include <QAction>
#include <QMenu>

DPEXTENDMENU_BEGIN_NAMESPACE

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
    static bool isSuffixSupport(const DCustomActionEntry &action, const QUrl &url);
    static void appendAllMimeTypes(const AbstractFileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes);
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

DPEXTENDMENU_END_NAMESPACE

#endif   // DCUSTOMACTIONBUILDER_H
