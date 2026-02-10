// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modeldatahandler.h"

using namespace ddplugin_organizer;

ModelDataHandler::ModelDataHandler()
{

}

ModelDataHandler::~ModelDataHandler()
{

}

bool ModelDataHandler::acceptInsert(const QUrl &url)
{
    Q_UNUSED(url)
    return true;
}

bool ModelDataHandler::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(oldUrl)
    Q_UNUSED(newUrl)
    return true;
}

bool ModelDataHandler::acceptUpdate(const QUrl &url, const QVector<int> &roles)
{
    Q_UNUSED(url)
    return true;
}

QList<QUrl> ModelDataHandler::acceptReset(const QList<QUrl> &urls)
{
    return urls;
}
