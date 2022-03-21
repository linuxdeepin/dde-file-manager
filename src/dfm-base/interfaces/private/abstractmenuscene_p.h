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

DFMBASE_BEGIN_NAMESPACE

class AbstractMenuScenePrivate : public QObject
{
public:
    friend class AbstractMenuScene;
    explicit AbstractMenuScenePrivate(AbstractMenuScene *qq);

public:
    QString currentDir;
    QStringList selectFiles;
    QString focusFile;
    bool onDesktop;
    bool isEmptyArea;

    AbstractFileInfoPointer focusFileInfo;
    QList<QAction *> providSelfActionList;
    QMap<QString, QAction *> predicateAction;
    QMap<QString, QString> predicateName;
};

DFMBASE_END_NAMESPACE
#endif   // ABSTRACTMENUSCENE_P_H
