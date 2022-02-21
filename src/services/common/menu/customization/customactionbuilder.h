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
#ifndef CUSTOMACTIONBUILDER_H
#define CUSTOMACTIONBUILDER_H

#include "dfm_common_service_global.h"

#include "customactiondata.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/mimetype/mimedatabase.h"

#include <QObject>
#include <QIcon>
#include <QFontMetrics>

class QMenu;
class QAction;

DSC_BEGIN_NAMESPACE

class CustomActionBuilder : public QObject
{
    Q_OBJECT
public:
    explicit CustomActionBuilder(QObject *parent = nullptr);
    ~CustomActionBuilder() {}
    QAction *buildAciton(const CustomActionData &actionData, QWidget *parentForSubmenu) const;
    void setActiveDir(const QUrl &dir);
    void setFocusFile(const QUrl &file);
    QString getCompleteSuffix(const QString &fileName, const QString &suf);
    static CustomActionDefines::ComboType checkFileCombo(const QList<QUrl> &files);
    static QList<CustomActionEntry> matchFileCombo(const QList<CustomActionEntry> &rootActions,
                                                   CustomActionDefines::ComboTypes type);
    static QList<CustomActionEntry> matchActions(const QList<QUrl> &selects,
                                                 QList<CustomActionEntry> oriActions);
    static QPair<QString, QStringList> makeCommand(const QString &cmd, CustomActionDefines::ActionArg arg,
                                                   const QUrl &dir, const QUrl &foucs, const QList<QUrl> &files);
    static QStringList splitCommand(const QString &cmd);

private:
    static bool isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes);
    static bool isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes);
    static bool isSchemeSupport(const CustomActionEntry &action, const QUrl &url);
    static bool isSuffixSupport(const CustomActionEntry &action, const QUrl &url);
    static void appendAllMimeTypes(const AbstractFileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes);
    static void appendParentMimeType(const QStringList &parentmimeTypes, QStringList &mimeTypes);

protected:
    QAction *createMenu(const CustomActionData &actionData, QWidget *parentForSubmenu) const;
    QAction *createAciton(const CustomActionData &actionData) const;

protected:
    QIcon getIcon(const QString &iconName) const;
    QString makeName(const QString &name, CustomActionDefines::ActionArg arg) const;

private:
    QFontMetrics fm;
    QString dirName;
    QUrl dirPath;
    QString fileBaseName;
    QString fileFullName;
    QUrl filePath;
};

DSC_END_NAMESPACE
#endif   // CUSTOMACTIONBUILDER_H
