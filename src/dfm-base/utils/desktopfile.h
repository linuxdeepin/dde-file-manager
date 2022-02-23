/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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

#ifndef DESKTOPFILE_H
#define DESKTOPFILE_H

#include "dfm-base/dfm_base_global.h"

#include <QStringList>

/**
 * @class DesktopFile
 * @brief Represents a linux desktop file
 * @author Michal Rost
 * @date 13.1.2013
 */

DFMBASE_BEGIN_NAMESPACE

class DesktopFile
{
public:
    explicit DesktopFile(const QString &fileName = "");
    QString desktopFileName() const;
    QString desktopPureFileName() const;
    QString desktopName() const;
    QString desktopLocalName() const;
    QString desktopDisplayName() const;
    QString desktopExec() const;
    QString desktopIcon() const;
    QString desktopType() const;
    QString desktopDeepinId() const;
    QString desktopDeepinVendor() const;
    bool isNoShow() const;
    QStringList desktopCategories() const;
    QStringList desktopMimeType() const;

private:
    QString fileName;
    QString name;
    QString genericName;
    QString localName;
    QString exec;
    QString icon;
    QString type;
    QStringList categories;
    QStringList mimeType;
    QString deepinId;
    QString deepinVendor;
    bool noDisplay = false;
    bool hidden = false;
};

DFMBASE_END_NAMESPACE

#endif   // DESKTOPFILE_H
