/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DEFAULTCANVASMODEL_H
#define DEFAULTCANVASMODEL_H

#include "dfm_desktop_service_global.h"
#include "dfm-base/widgets/abstractcanvasmodel.h"
#include "canvas/defaultdesktopfileinfo.h"

DSB_D_BEGIN_NAMESPACE
class DefaultCanvasModel : public dfmbase::AbstractCanvasModel
{
    Q_OBJECT
public:
    explicit DefaultCanvasModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(const QString &fileUrl, int column = 0);
    QModelIndex index(const DFMDesktopFileInfoPointer &fileInfo, int column = 0) const;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QVariant dataByRole(const DefaultDesktopFileInfo *fileInfo, int role) const;
    QModelIndex createIndexByFileInfo(const DFMDesktopFileInfoPointer &fileInfo, int column) const;
};
DSB_D_END_NAMESPACE
#endif   // DEFAULTCANVASMODEL_H
