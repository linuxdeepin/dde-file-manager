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
#include "custom/custommode_p.h"
#include "models/fileproxymodel.h"

#include "config/configpresenter.h"

#include <QDebug>
#include <QUuid>

DDP_ORGANIZER_USE_NAMESPACE

CustomModePrivate::CustomModePrivate(CustomMode *qq) : q(qq)
{

}

CustomMode::CustomMode(QObject *parent)
    : CanvasOrganizer(parent)
    , d(new CustomModePrivate(this))
{

}

CustomMode::~CustomMode()
{
    if (model && (model->handler() == d->dataHandler))
        model->setHandler(nullptr);

    delete d->dataHandler;
    d->dataHandler = nullptr;

    delete d;
}

OrganizerMode CustomMode::mode() const
{
    return OrganizerMode::kCustom;
}

bool CustomMode::initialize(FileProxyModel *m)
{
    Q_ASSERT(m);
    Q_ASSERT(!d->dataHandler);
    Q_ASSERT(!model);
    model = m;

    connect(CfgPresenter, &ConfigPresenter::newCollection, this, &CustomMode::onNewCollection);

    // must be DirectConnection to keep sequential
    connect(model, &FileProxyModel::rowsInserted, this, &CustomMode::onFileInserted, Qt::DirectConnection);
    connect(model, &FileProxyModel::rowsAboutToBeRemoved, this, &CustomMode::onFileAboutToBeRemoved, Qt::DirectConnection);
    connect(model, &FileProxyModel::dataReplaced, this, &CustomMode::onFileRenamed, Qt::DirectConnection);

    connect(model, &FileProxyModel::dataChanged, this, &CustomMode::onFileDataChanged, Qt::QueuedConnection);
    connect(model, &FileProxyModel::modelReset, this, &CustomMode::rebuild, Qt::QueuedConnection);

    d->dataHandler = new CustomDataHandler();

    // restore collection as config
    QList<CollectionBaseDataPtr> store = CfgPresenter->customProfile();
    d->dataHandler->reset(store);

    model->setHandler(d->dataHandler);
    model->refresh(model->rootIndex(), false, 0);

    // creating if there already are files.
    if (!model->files().isEmpty())
        rebuild();

    return true;
}

void CustomMode::reset()
{
    // 获取配置
    QList<CollectionBaseDataPtr> store = CfgPresenter->customProfile();
    d->dataHandler->reset(store);

    model->refresh(model->rootIndex(), false, 0);
}

void CustomMode::rebuild()
{
    // remove invaild url
    d->dataHandler->check(QSet<QUrl>::fromList(model->files()));

    // write config
    CfgPresenter->saveCustomProfile(d->dataHandler->baseDatas());
    //

    // get region name and items, then create collection.
    for (const QString &key : d->dataHandler->keys()) {
        const QString &name = d->dataHandler->name(key);
        auto files = d->dataHandler->items(key);
        qDebug() << "name" << name << "files" << files.size();

        // 复用已有分组
        CollectionHolderPointer collectionHolder = d->holders.value(key);
        // 创建没有的组

        if (collectionHolder.isNull()) {
            collectionHolder.reset(new CollectionHolder(key));
            collectionHolder->createFrame(surface, model);
            collectionHolder->setName(name);
            d->holders.insert(key, collectionHolder);
        }
        collectionHolder->setUrls(files);

        // disable rename,move and adjust
        collectionHolder->setRenamable(true);
        collectionHolder->setMovable(true);
        collectionHolder->setAdjustable(true);

        collectionHolder->show();
    }

    emit collectionChanged();
}

void CustomMode::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (!d->dataHandler->replace(oldUrl, newUrl).isEmpty()) {
        //todo(zy)
        // 写入配置文件
    }
}

void CustomMode::onFileInserted(const QModelIndex &parent, int first, int last)
{
    //todo(wcl)
    // 特殊处理在集合中的新建
}

void CustomMode::onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    bool ok = false;
    for (int i = first; i <= last; i++) {
        QModelIndex index = model->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = model->fileUrl(index);
        if (!d->dataHandler->remove(url).isEmpty() && !ok)
            ok = true;
    }

    if (ok) {
        //todo(zy)
        // 写入配置文件
    }
}

void CustomMode::onFileDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    //todo(wcl)
    // 仅更新
}

bool CustomMode::filterDataRested(QList<QUrl> *urls)
{
    bool filter = false;
    if (urls && d->dataHandler) {
        for (auto iter = urls->begin(); iter != urls->end();) {
            if (d->dataHandler->acceptInsert(*iter)) {
                iter = urls->erase(iter);
                filter = true;
            } else {
                iter++;
            }
        }
    }
    return filter;
}

bool CustomMode::filterDataInserted(const QUrl &url)
{
    if (d->dataHandler)
        return d->dataHandler->acceptInsert(url);
    return false;
}

bool CustomMode::filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (d->dataHandler)
        return d->dataHandler->acceptRename(oldUrl, newUrl);

    return false;
}

void CustomMode::onNewCollection(const QList<QUrl> &list)
{
    // todo 检查数据有效性
    CollectionBaseDataPtr base(new CollectionBaseData);
    base->name = tr("New Collection");
    base->key = QUuid::createUuid().toString(QUuid::WithoutBraces);
    base->items = list;

    d->dataHandler->addBaseData(base);

    model->refresh(model->rootIndex(), false, 0);;
}


