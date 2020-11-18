/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zhangyu <zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu <zhangyub@uniontech.com>
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

#include <QObject>
#include <QIcon>

class QMenu;
class QAction;
class DCustomActionBuilder : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionBuilder(QObject *parent = nullptr);
    QAction *buildAciton(const DCustomActionData &actionData, QWidget *parentForSubmenu) const;
    static DCustomActionDefines::FileComboTypes checkFileCombo(const DUrlList &files);
    static QList<DCustomActionEntry> matchFileCombo(const QList<DCustomActionEntry> &rootActions,
                                                 DCustomActionDefines::FileComboTypes type);
protected:
    QAction *createMenu(const DCustomActionData &actionData, QWidget *parentForSubmenu) const;
    QAction *createAciton(const DCustomActionData &actionData) const;
signals:

public slots:
protected:
    QIcon getIcon(const QString &iconName) const;
};

#endif // DCUSTOMACTIONBUILDER_H
