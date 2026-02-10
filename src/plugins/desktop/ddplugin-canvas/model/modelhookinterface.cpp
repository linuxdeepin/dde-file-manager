// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

bool ModelHookInterface::dataChanged(const QUrl &url, const QVector<int> &roles, void *extData) const
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

void ModelHookInterface::hiddenFlagChanged(bool show) const
{

}
