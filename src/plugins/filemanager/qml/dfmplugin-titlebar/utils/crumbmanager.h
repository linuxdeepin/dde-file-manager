// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CRUMBMANAGER_H
#define CRUMBMANAGER_H

#include "dfmplugin_titlebar_global.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>

#include <functional>

namespace dfmplugin_titlebar {

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

private:
    CrumbCreatorMap creators;
};

}

#endif   // CRUMBMANAGER_H
