/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "dfmviewmanager.h"
#include "dfmbaseview.h"

#include "views/dfileview.h"
#include "views/computerview.h"

#include <QHash>

DFM_BEGIN_NAMESPACE

class DFMViewManagerPrivate
{
public:
    DFMViewManagerPrivate(DFMViewManager *qq);

    DFMViewManager *q_ptr;

    QMultiHash<const DFMViewManager::KeyType, DFMViewManager::ViewCreatorType> controllerCreatorHash;
};

DFMViewManagerPrivate::DFMViewManagerPrivate(DFMViewManager *qq)
    : q_ptr(qq)
{

}

DFMViewManager *DFMViewManager::instance()
{
    static DFMViewManager manager;

    return &manager;
}

bool DFMViewManager::isRegisted(const QString &scheme, const QString &host, const std::type_info &info) const
{
    Q_D(const DFMViewManager);

    const KeyType &type = KeyType(scheme, host);

    foreach (const ViewCreatorType &value, d->controllerCreatorHash.values(type)) {
        if (value.first == info.name())
            return true;
    }

    return false;
}

void DFMViewManager::clearUrlView(const QString &scheme, const QString &host)
{
    Q_D(DFMViewManager);

    const KeyType handler(scheme, host);

    d->controllerCreatorHash.remove(handler);
}

DFMBaseView *DFMViewManager::createViewByUrl(const DUrl &fileUrl) const
{
    Q_D(const DFMViewManager);

    QList<KeyType> handlerTypeList;

    handlerTypeList << KeyType(fileUrl.scheme(), fileUrl.path());
    handlerTypeList << KeyType(QString(), fileUrl.path());
    handlerTypeList << KeyType(fileUrl.scheme(), QString());

    for (const KeyType &handlerType : handlerTypeList) {
        const QList<ViewCreatorType> creatorList = d->controllerCreatorHash.values(handlerType);

        if (creatorList.isEmpty())
            continue;

        return (creatorList.first().second)();
    }

    return 0;
}

QString DFMViewManager::suitedViewTypeNameByUrl(const DUrl &fileUrl) const
{
    Q_D(const DFMViewManager);

    QList<KeyType> handlerTypeList;

    handlerTypeList << KeyType(fileUrl.scheme(), fileUrl.path());
    handlerTypeList << KeyType(QString(), fileUrl.path());
    handlerTypeList << KeyType(fileUrl.scheme(), QString());

    for (const KeyType &handlerType : handlerTypeList) {
        const QList<ViewCreatorType> creatorList = d->controllerCreatorHash.values(handlerType);

        if (creatorList.isEmpty())
            continue;

        return creatorList.first().first;
    }

    return QString();
}

bool DFMViewManager::isSuited(const DUrl &fileUrl, const DFMBaseView *view) const
{
    return suitedViewTypeNameByUrl(fileUrl) == typeid(*view).name();
}

DFMViewManager::DFMViewManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMViewManagerPrivate(this))
{
    dRegisterUrlView<DFileView>("file", QString());
    dRegisterUrlView<DFileView>("trash", QString());
    dRegisterUrlView<DFileView>("search", QString());
    dRegisterUrlView<DFileView>("usershare", QString());
    dRegisterUrlView<DFileView>("network", QString());
    dRegisterUrlView<DFileView>("smb", QString());
    dRegisterUrlView<DFileView>("stp", QString());
    dRegisterUrlView<DFileView>("fstp", QString());
    dRegisterUrlView<ComputerView>("computer", "/");
}

DFMViewManager::~DFMViewManager()
{

}

void DFMViewManager::insertToCreatorHash(const DFMViewManager::KeyType &type, const DFMViewManager::ViewCreatorType &creator)
{
    Q_D(DFMViewManager);

    d->controllerCreatorHash.insertMulti(type, creator);
}

DFM_END_NAMESPACE
