/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef CRUMBMANAGER_H
#define CRUMBMANAGER_H

#include "dfmplugin_titlebar_global.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>

#include <functional>

DPTITLEBAR_BEGIN_NAMESPACE

class CrumbInterface;
class CrumbManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CrumbManager)

public:
    using KeyType = QString;
    using CrumbCreator = std::function<CrumbInterface *()>;
    using CrumbCreatorMap = QMap<KeyType, CrumbCreator>;

public:
    static CrumbManager *instance();

    void registerCrumbCreator(const KeyType &scheme, const CrumbCreator &creator);
    bool isRegisted(const KeyType &scheme) const;
    CrumbInterface *createControllerByUrl(const QUrl &url);

private:
    explicit CrumbManager(QObject *parent = nullptr);
    ~CrumbManager();

private:
    CrumbCreatorMap creators;
};

DPTITLEBAR_END_NAMESPACE

#endif   // CRUMBMANAGER_H
