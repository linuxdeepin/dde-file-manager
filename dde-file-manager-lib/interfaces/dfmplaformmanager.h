/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DFMPLAFORMMANAGER_H
#define DFMPLAFORMMANAGER_H

#include <QObject>
#include <QSettings>

class DFMPlaformManager : public QObject
{
    Q_OBJECT
public:
    explicit DFMPlaformManager(QObject *parent = nullptr);
    QString platformConfigPath();

    bool isDisableUnMount();
    bool isRoot_hidden();

signals:

public slots:

private:
    QSettings* m_platformSettings = nullptr;
};

#endif // DFMPLAFORMMANAGER_H
