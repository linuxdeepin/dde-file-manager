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
#include "normalized/normalizedmode_p.h"
#include "models/fileproxymodel.h"
#include "config/configpresenter.h"

#include <QDebug>
#include <QTime>

DDP_ORGANIZER_USE_NAMESPACE

NormalizedModePrivate::NormalizedModePrivate(NormalizedMode *qq) : q(qq)
{

}

NormalizedModePrivate::~NormalizedModePrivate()
{
    delete classifier;
    classifier = nullptr;

    holders.clear();
}

NormalizedMode::NormalizedMode(QObject *parent)
    : CanvasOrganizer(parent)
    , d(new NormalizedModePrivate(this))
{

}

NormalizedMode::~NormalizedMode()
{

}

int NormalizedMode::mode() const
{
    return OrganizerMode::kNormalized;
}

bool NormalizedMode::initialize(FileProxyModel *m)
{
    Q_ASSERT(m);
    model = m;

    // 根据配置创建分类器, 默认按类型
    auto type = CfgPresenter->classification();
    qInfo() << "classification:" << type;

    setClassifier(type);
    Q_ASSERT(d->classifier);

    connect(model, &FileProxyModel::rowsInserted, this, &NormalizedMode::onFileInserted, Qt::DirectConnection);
    connect(model, &FileProxyModel::rowsAboutToBeRemoved, this, &NormalizedMode::onFileAboutToBeRemoved, Qt::DirectConnection);
    connect(model, &FileProxyModel::dataReplaced, this, &NormalizedMode::onFileRenamed, Qt::DirectConnection);
    connect(model, &FileProxyModel::dataChanged, this, &NormalizedMode::onFileDataChanged, Qt::QueuedConnection);
    connect(model, &FileProxyModel::modelReset, this, &NormalizedMode::rebuild, Qt::QueuedConnection);


    // creating if there already are files.
    if (!model->files().isEmpty())
        rebuild();

    return true;
}

void NormalizedMode::reset()
{
    auto type = CfgPresenter->classification();
    qInfo() << "normalized mode reset to " << type;
    setClassifier(type);
    Q_ASSERT(d->classifier);
}

void NormalizedMode::rebuild()
{
    // 使用分类器对文件进行分类，后续性能问题需考虑异步分类
    {
        QTime time;
        time.start();
        auto files = model->files();
        d->classifier->reset(files);
        qInfo() << QString("Classifying %0 files takes %1ms").arg(files.size()).arg(time.elapsed());
    }

    // 从分类器中获取组,根据组创建分区
    for (const QString &key : d->classifier->regionKeys()) {
        const QString &name = d->classifier->name(key);
        auto files = d->classifier->items(key);
        qDebug() << "type" << name << "files" << files.size();

        // 复用已有分组
        CollectionHolderPointer collectionHolder = d->holders.value(name);
        // 创建没有的组

        if (collectionHolder.isNull()) {
            collectionHolder.reset(new CollectionHolder(name));
            collectionHolder->createFrame(surface, model);
            collectionHolder->setName(name);
            d->holders.insert(name, collectionHolder);
        }
        collectionHolder->setUrls(files);

        // disable rename,move,adjust,drag
        collectionHolder->setRenamable(false);
        collectionHolder->setMovable(false);
        collectionHolder->setAdjustable(false);
        collectionHolder->setDragEnabled(false);

        collectionHolder->show();
    }
    // 删除无需的组
}

void NormalizedMode::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    d->classifier->repalce(oldUrl, newUrl);
}

void NormalizedMode::onFileInserted(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex index = model->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = model->fileUrl(index);
        d->classifier->append(url);
   }
}

void NormalizedMode::onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex index = model->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = model->fileUrl(index);
        d->classifier->take(url);
    }
}

void NormalizedMode::onFileDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (Q_UNLIKELY(!topLeft.isValid() || !bottomRight.isValid()))
        return;
    for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = model->index(i, 0);
        d->classifier->change(model->fileUrl(index));
    }
}

bool NormalizedMode::filterDataRested(QList<QUrl> *urls)
{
    // All datas are not displayed on the desktop.
    if (urls && d->classifier) {
        urls->clear();
        return true;
    }
    return false;
}

bool NormalizedMode::filterDataInserted(const QUrl &url)
{
    return d->classifier;
}

bool NormalizedMode::filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    return d->classifier;
}

bool NormalizedMode::setClassifier(Classifier id)
{
    if (d->classifier) {
        if (d->classifier->mode() == id) {
            qDebug() << "ingore setting, current classifier was" << id;
            return true;
        }

        if (model->handler() == d->classifier->dataHandler())
            model->setHandler(nullptr);

       delete d->classifier;
    }

    // clear all collections
    d->holders.clear();

    d->classifier = ClassifierCreator::createClassifier(id);
    if (!d->classifier)
        return false;

    model->setHandler(d->classifier->dataHandler());
    model->refresh(model->rootIndex(), false, 0);
    return true;
}

