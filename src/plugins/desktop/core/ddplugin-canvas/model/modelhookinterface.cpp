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
#include "modelhookinterface.h"

using namespace ddplugin_canvas;

ModelHookInterface::ModelHookInterface()
{

}

ModelHookInterface::~ModelHookInterface()
{

}

bool ModelHookInterface::modelData(const QUrl &url, int role, QVariant *out, void *extData) const
{
    return false;
}

bool ModelHookInterface::dataInserted(const QUrl &url, void *extData) const
{
    return false;
}

bool ModelHookInterface::dataRemoved(const QUrl &url, void *extData) const
{
    return false;
}

bool ModelHookInterface::dataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData) const
{
    return false;
}

bool ModelHookInterface::dataRested(QList<QUrl> *urls, void *extData) const
{
    return false;
}

bool ModelHookInterface::dataChanged(const QUrl &url, void *extData) const
{
    return false;
}

bool ModelHookInterface::dropMimeData(const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData) const
{
    return false;
}

bool ModelHookInterface::mimeData(const QList<QUrl> &urls, QMimeData *out, void *extData) const
{
    return false;
}

bool ModelHookInterface::mimeTypes(QStringList *types, void *extData) const
{
    return false;
}

bool ModelHookInterface::sortData(int role, int order, QList<QUrl> *files, void *extData) const
{
    return false;
}
