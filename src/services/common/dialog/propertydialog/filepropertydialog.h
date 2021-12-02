/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#ifndef FILEPROPERTYVIEW_H
#define FILEPROPERTYVIEW_H

#include "dfm_common_service_global.h"

#include <DAbstractDialog>
DWIDGET_USE_NAMESPACE
DSC_BEGIN_NAMESPACE
class FilePropertyDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit FilePropertyDialog(QObject *parent = nullptr);

signals:

public slots:
};
DSC_END_NAMESPACE
#endif   // FILEPROPERTYVIEW_H
