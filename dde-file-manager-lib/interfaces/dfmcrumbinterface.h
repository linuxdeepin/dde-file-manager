/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#ifndef DFMCRUMBINTERFACE_H
#define DFMCRUMBINTERFACE_H

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class CrumbData
{
public:
    DUrl url = DUrl();
    QString iconName = QString(); // icon group name
    QString displayText = "Default";
    QString iconKey = "icon"; // icon key
};

class DFMCrumbItem;
class DFMCrumbInterface : public QObject
{
    Q_OBJECT
public:
    explicit DFMCrumbInterface(QObject *parent = 0);
    ~DFMCrumbInterface();

    virtual QList<CrumbData> seprateUrl(const DUrl &url) = 0;
    virtual DFMCrumbItem* createCrumbItem(const CrumbData &data) = 0;
    virtual DFMCrumbItem* createCrumbItem(const DUrl &url) = 0;
};

DFM_END_NAMESPACE

#endif // DFMCRUMBINTERFACE_H
