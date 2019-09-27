/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#pragma once

#include <QObject>
#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMAdditionalMenu : public QObject
{
    Q_OBJECT
public:
    const QStringList AllMenuTypes {
        "SingleFile",
        "SingleDir",
        "MultiFileDirs",
        "EmptyArea"
    };

    const QLatin1String MENU_TYPE_KEY {"X-DFM-MenuTypes"};
    const QLatin1String MIME_TYPE_KEY {"X-DFM-MimeTypes"};

    explicit DFMAdditionalMenu(QObject *parent = nullptr);

    // files.size() == 0 is emptyArea
    QList<QAction*> actions(const QStringList &files, const QString& currentDir = QString());
protected:
    void appendParentMineType(const QStringList &parentmimeTypes,  QStringList& mimeTypes);
private:
    QList<QAction *> actionList;
    QMap<QString, QList<QAction *>> actionListByType;

signals:

public slots:
};

DFM_END_NAMESPACE
