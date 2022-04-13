/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "modelextendinterface.h"

DDP_CANVAS_USE_NAMESPACE

ModelExtendInterface::ModelExtendInterface()
{

}

ModelExtendInterface::~ModelExtendInterface()
{

}

bool ModelExtendInterface::modelData(const QUrl &url, int role, QVariant *out, void *extData) const
{
    return false;
}

bool ModelExtendInterface::dataInserted(const QUrl &url, void *extData) const
{
    return false;
}

bool ModelExtendInterface::dataRemoved(const QUrl &url, void *extData) const
{
    return false;
}

bool ModelExtendInterface::dataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData) const
{
    return false;
}

bool ModelExtendInterface::dataRested(QList<QUrl> *urls, void *extData) const
{
    return false;
}

bool ModelExtendInterface::dataChanged(const QUrl &url, void *extData) const
{
    return false;
}

bool ModelExtendInterface::dropMimeData(const QMimeData *data, Qt::DropAction action, void *extData) const
{
    return false;
}

bool ModelExtendInterface::mimeData(const QList<QUrl> &urls, QMimeData *out, void *extData) const
{
    return false;
}

bool ModelExtendInterface::mimeTypes(QStringList *types, void *extData) const
{
    return false;
}

bool ModelExtendInterface::sortData(int role, int order, QList<QUrl> *files, void *extData) const
{
    return false;
}
