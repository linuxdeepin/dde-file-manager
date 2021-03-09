/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "dcustomactiondata.h"
#include "durl.h"
#include "interfaces/dfileservices.h"
#include "dmimedatabase.h"

#include <QObject>
#include <QIcon>
#include <QFontMetrics>

class QMenu;
class QAction;
class DCustomActionBuilder : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionBuilder(QObject *parent = nullptr);
    QAction *buildAciton(const DCustomActionData &actionData, QWidget *parentForSubmenu) const;
    void setActiveDir(const DUrl &dir);
    void setFocusFile(const DUrl &file);
    QString getCompleteSuffix(const QString & fileName, const QString &suf);
    static DCustomActionDefines::ComboType checkFileCombo(const DUrlList &files);
    static QList<DCustomActionEntry> matchFileCombo(const QList<DCustomActionEntry> &rootActions,
                                                    DCustomActionDefines::ComboTypes type);
    static QList<DCustomActionEntry> matchActions(const DUrlList &selects,
                             QList<DCustomActionEntry> oriActions);
    static QPair<QString, QStringList> makeCommand(const QString &cmd, DCustomActionDefines::ActionArg arg,
                               const DUrl &dir, const DUrl& foucs, const DUrlList &files);
    static QStringList splitCommand(const QString &cmd);
private:
    static bool isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes);
    static bool isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes);
    static bool isSchemeSupport(const DCustomActionEntry &action, const DUrl &url);
    static bool isSuffixSupport(const DCustomActionEntry &action, const DUrl &url);
    static void appendAllMimeTypes(const DAbstractFileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes);
    static void appendParentMimeType(const QStringList &parentmimeTypes,  QStringList& mimeTypes);
protected:
    QAction *createMenu(const DCustomActionData &actionData, QWidget *parentForSubmenu) const;
    QAction *createAciton(const DCustomActionData &actionData) const;
signals:

public slots:
protected:
    QIcon getIcon(const QString &iconName) const;
    QString makeName(const QString &name, DCustomActionDefines::ActionArg arg) const;

private:
    QFontMetrics m_fm;
    QString m_dirName;
    DUrl m_dirPath;
    QString m_fileBaseName;
    QString m_fileFullName;
    DUrl m_filePath;
    DMimeDatabase mimeDatabase;
};

#endif // DCUSTOMACTIONBUILDER_H
