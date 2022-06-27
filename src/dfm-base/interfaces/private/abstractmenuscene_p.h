/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef ABSTRACTMENUSCENE_P_H
#define ABSTRACTMENUSCENE_P_H

#include "interfaces/abstractmenuscene.h"
#include "interfaces/abstractfileinfo.h"

namespace dfmbase {

class AbstractMenuScenePrivate : public QObject
{
    Q_OBJECT
public:
    friend class AbstractMenuScene;
    explicit AbstractMenuScenePrivate(AbstractMenuScene *qq);

    virtual bool initializeParamsIsValid();

public:
    QUrl currentDir;
    QList<QUrl> selectFiles;
    QUrl focusFile;
    bool onDesktop { false };
    bool isEmptyArea { false };
    bool isFocusOnDDEDesktopFile { false };
    bool isDDEDesktopFileIncluded { false };
    bool isSystemPathIncluded { false };
    quint64 windowId { 0 };
    Qt::ItemFlags indexFlags;

    AbstractFileInfoPointer focusFileInfo;
    QMap<QString, QAction *> predicateAction;   // id -- instance
    QMap<QString, QString> predicateName;   // id -- text
};

}
#endif   // ABSTRACTMENUSCENE_P_H
