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
#include "interface/canvasviewshell.h"

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

void NormalizedModePrivate::restore(const QList<CollectionBaseDataPtr> &cfgs)
{
    // order by config
    for (const CollectionBaseDataPtr &cfg : cfgs) {
        if (auto base = classifier->baseData(cfg->key)) {
            QList<QUrl> org = base->items;
            QList<QUrl> ordered;
            for (const QUrl &old : cfg->items) {
                if (org.contains(old)) {
                    ordered << old;
                    org.removeOne(old);
                }
            }

            ordered.append(org);
            base->items = ordered;
        }
    }
}

NormalizedMode::NormalizedMode(QObject *parent)
    : CanvasOrganizer(parent)
    , d(new NormalizedModePrivate(this))
{

}

NormalizedMode::~NormalizedMode()
{
    if (model && (model->handler() == d->classifier->dataHandler()))
        model->setHandler(nullptr);

    delete d;
}

OrganizerMode NormalizedMode::mode() const
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

    // must be DirectConnection to keep sequential
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

void NormalizedMode::layout()
{
    // todo screen index
    const int screenIdx = 1; // todo
    const QSize gridSize = canvasViewShell->gridSize(screenIdx);
    const int widthTime = 4; // todo time is follow icon level;
    const int heightTime = 2;
    auto holders = d->holders.values();
    {
        const QStringList &ordered =  d->classifier->classes();
        const int max = ordered.size();
        std::sort(holders.begin(), holders.end(), [&ordered, max](const CollectionHolderPointer &t1,
                  const CollectionHolderPointer &t2) {
            int i1 = ordered.indexOf(t1->id());
            if (i1 < 0)
                i1 = max;
            int i2 = ordered.indexOf(t2->id());
            if (i2 < 0)
                i2 = max;
            return i1 < i2;
        });
    }

    QList<CollectionStyle> toSave;
    QPoint nextPos(0, 0);
    for (const CollectionHolderPointer &holder : holders) {
        auto style = holder->style();
        // todo 网格超出的处理

        auto rect = canvasViewShell->gridVisualRect(screenIdx, nextPos);
        style.rect = QRect(rect.topLeft(), QSize(rect.width() * widthTime, rect.height() * heightTime))
                .marginsRemoved(QMargins(4, 4, 4, 4));
        holder->setStyle(style);

        toSave << style;

        // next
        nextPos.setY(nextPos.y() + heightTime);
        if (nextPos.y() + heightTime  > gridSize.height()) {
            nextPos.setY(0);
            nextPos.setX(nextPos.x() + widthTime);
        }
    }

    CfgPresenter->writeNormalStyle(toSave);
}

void NormalizedMode::rebuild()
{
    // 使用分类器对文件进行分类，后续性能问题需考虑异步分类
    {
        QTime time;
        time.start();
        auto files = model->files();
        d->classifier->reset(files);

        // order item as config
        d->restore(CfgPresenter->normalProfile());

        qInfo() << QString("Classifying %0 files takes %1ms").arg(files.size()).arg(time.elapsed());
        if (!files.isEmpty())
            CfgPresenter->saveNormalProfile(d->classifier->baseData());
    }

    // 从分类器中获取组,根据组创建分区
    for (const QString &key : d->classifier->keys()) {
        const QString &name = d->classifier->name(key);
        auto files = d->classifier->items(key);
        qDebug() << "type" << name << "files" << files.size();

        // 复用已有分组
        CollectionHolderPointer collectionHolder = d->holders.value(name);
        // 创建没有的组

        if (collectionHolder.isNull()) {
            collectionHolder.reset(new CollectionHolder(key, d->classifier));
            collectionHolder->createFrame(surfaces.first().data(), model);
            collectionHolder->setName(name);
            d->holders.insert(name, collectionHolder);

            // disable rename,move,drag,close,stretch
            collectionHolder->setRenamable(false);
            collectionHolder->setMovable(false);
            collectionHolder->setDragEnabled(false);
            collectionHolder->setClosable(false);
            collectionHolder->setStretchable(false);

            // enable adjust
            collectionHolder->setAdjustable(true);
        }

        collectionHolder->show();
    }
    // 删除无需的组


    layout();

    emit collectionChanged();
}

void NormalizedMode::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    d->classifier->replace(oldUrl, newUrl);
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
        d->classifier->remove(url);
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

