// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "normalized/normalizedmode_p.h"
#include "models/collectionmodel.h"
#include "config/configpresenter.h"
#include "interface/canvasviewshell.h"
#include "utils/fileoperator.h"

#include <QDebug>
#include <QTime>

using namespace ddplugin_organizer;

static constexpr int kCollectionGridColumnCount = 4;
static constexpr int kSmallCollectionGridRowCount = 2;
static constexpr int kLargeCollectionGridRowCount = 4;
static constexpr int kCollectionGridMargin = 2;

NormalizedModePrivate::NormalizedModePrivate(NormalizedMode *qq)
    : QObject(qq)
    , q(qq)
{
    broker = new NormalizedModeBroker(qq);
    broker->init();
}

NormalizedModePrivate::~NormalizedModePrivate()
{

}

QPoint NormalizedModePrivate::findValidPos(QPoint &nextPos, int &currentIndex, CollectionStyle &style, const int width, const int height)
{
    auto gridSize = q->canvasViewShell->gridSize(currentIndex);
    if (!gridSize.isValid()) {
        // fix to last screen,and use overlap pos
        currentIndex = q->surfaces.count();
        gridSize = q->canvasViewShell->gridSize(currentIndex);
    }

    if (nextPos.y() + height > gridSize.height()) {
        // fix to first row
        nextPos.setY(0);
        nextPos.setX(nextPos.x() + width);
    }

    if (nextPos.x() + width > gridSize.width()) {

        if (currentIndex == q->surfaces.count()) {
            // overlap pos
            nextPos.setX(gridSize.width() - width);
            nextPos.setY(gridSize.height() - height);
            qDebug() << "stack collection:" << gridSize << width << height << nextPos;

            QPoint validPos(nextPos);
            // update next pos
            nextPos.setY(nextPos.y() + height);

            return validPos;
        }

        // fix to next screen
        currentIndex += 1;

        // restart find valid pos, in the first position of the next screen
        nextPos.setX(0);
        nextPos.setY(0);

        return findValidPos(nextPos, currentIndex, style, width, height);
    }

    QPoint validPos(nextPos);
    // update next pos
    nextPos.setY(nextPos.y() + height);

    return validPos;
}

void NormalizedModePrivate::collectionStyleChanged(const QString &id)
{
    if (auto holder = holders.value(id)) {
        CfgPresenter->updateNormalStyle(holder->style());
        q->layout();
    }
}

CollectionHolderPointer NormalizedModePrivate::createCollection(const QString &id)
{
    QString name = classifier->className(id);
    Q_ASSERT(!name.isEmpty());

    CollectionHolderPointer holder;
    holder.reset(new CollectionHolder(id, classifier));
    holder->createFrame(q->surfaces.first().data(), q->model);
    holder->setCanvasModelShell(q->canvasModelShell);
    holder->setCanvasViewShell(q->canvasViewShell);
    holder->setCanvasGridShell(q->canvasGridShell);
    holder->setCanvasManagerShell(q->canvasManagerShell);
    holder->setName(name);

    // disable rename,move,file shift,close,stretch
    holder->setRenamable(false);
    holder->setMovable(false);
    holder->setFileShiftable(false);
    holder->setClosable(false);
    holder->setStretchable(false);

    // enable adjust
    holder->setAdjustable(true);

    return holder;
}

void NormalizedModePrivate::switchCollection()
{
    bool changed = false;
    for (const CollectionBaseDataPtr &base : classifier->baseData()) {
        if (holders.contains(base->key)) {
            if (base->items.isEmpty()) {
                qDebug() << "Collection " << base->key << "is empty, remove it.";
                holders.remove(base->key);
                changed = true;
            }
        } else {
            if (!base->items.isEmpty()) {
                // create new collection.
                qDebug() << "Collection " << base->key << "isn't existed, create it.";
                CollectionHolderPointer collectionHolder(createCollection(base->key));
                connect(collectionHolder.data(), &CollectionHolder::styleChanged, this, &NormalizedModePrivate::collectionStyleChanged);
                holders.insert(base->key, collectionHolder);
                changed = true;
            }
        }
    }

    // relayout all collections.
    if (changed)
        q->layout();
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
    // delete all views first.
    d->holders.clear();
    removeClassifier();
}

OrganizerMode NormalizedMode::mode() const
{
    return OrganizerMode::kNormalized;
}

bool NormalizedMode::initialize(CollectionModel *m)
{
    Q_ASSERT(m);
    model = m;

    // 根据配置创建分类器, 默认按类型
    auto type = CfgPresenter->classification();
    qInfo() << "classification:" << type;

    setClassifier(type);
    Q_ASSERT(d->classifier);

    // must be DirectConnection to keep sequential
    connect(model, &CollectionModel::rowsInserted, this, &NormalizedMode::onFileInserted, Qt::DirectConnection);
    connect(model, &CollectionModel::rowsAboutToBeRemoved, this, &NormalizedMode::onFileAboutToBeRemoved, Qt::DirectConnection);
    connect(model, &CollectionModel::dataReplaced, this, &NormalizedMode::onFileRenamed, Qt::DirectConnection);

    connect(model, &CollectionModel::dataChanged, this, &NormalizedMode::onFileDataChanged, Qt::QueuedConnection);
    connect(model, &CollectionModel::modelReset, this, &NormalizedMode::rebuild, Qt::QueuedConnection);

    connect(model, &CollectionModel::openEditor, this, &NormalizedMode::onOpenEditor, Qt::QueuedConnection);

    // creating if there already are files.
    if (!model->files().isEmpty())
        rebuild();

    return true;
}

void NormalizedMode::reset()
{
    auto type = CfgPresenter->classification();
    qInfo() << "normalized mode reset to " << type;

    // delete the current classifier
    removeClassifier();

    setClassifier(type);
    Q_ASSERT(d->classifier);
}

void NormalizedMode::layout()
{
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

    // screen num is start with 1
    int screenIdx = 1;
    QList<CollectionStyle> toSave;
    QPoint nextPos(0, 0);

    for (const CollectionHolderPointer &holder : holders) {
        auto style = CfgPresenter->normalStyle(holder->id());
        if (Q_UNLIKELY(style.key != holder->id())) {
            qWarning() << "unknow err:style key is error:" << style.key << ",and fix to :" << holder->id();
            style.key = holder->id();
        }

        int currentHeightTime = style.sizeMode == CollectionFrameSize::kSmall ? kSmallCollectionGridRowCount : kLargeCollectionGridRowCount;
        auto pos = d->findValidPos(nextPos, screenIdx, style, kCollectionGridColumnCount, currentHeightTime);

        Q_ASSERT(screenIdx > 0);
        Q_ASSERT(screenIdx <= surfaces.count());

        style.screenIndex = screenIdx;
        holder->setSurface(surfaces.at(screenIdx - 1).data());

        auto rect = canvasViewShell->gridVisualRect(style.screenIndex, pos);

        style.rect = QRect(rect.topLeft(), QSize(rect.width() * kCollectionGridColumnCount, rect.height() * currentHeightTime))
                .marginsRemoved(QMargins(kCollectionGridMargin, kCollectionGridMargin, kCollectionGridMargin, kCollectionGridMargin));
        holder->setStyle(style);
        holder->show();

        toSave << style;
    }

    CfgPresenter->writeNormalStyle(toSave);
}

void NormalizedMode::detachLayout()
{
    for (auto holder : d->holders) {
        holder->setSurface(nullptr);
    }
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
        auto files = d->classifier->items(key);
        qDebug() << "type" << key << "files" << files.size();

        // 复用已有分组
        CollectionHolderPointer collectionHolder = d->holders.value(key);

        // remove or do not create the collection that is empty.
        bool unused = files.isEmpty();
        if (unused) {
            // remove it.
            if (!collectionHolder.isNull()) {
                d->holders.remove(key);
                collectionHolder.clear();
            }
        } else {
            // 创建没有的组
            if (collectionHolder.isNull()) {
                collectionHolder = d->createCollection(key);
                connect(collectionHolder.data(), &CollectionHolder::styleChanged, d, &NormalizedModePrivate::collectionStyleChanged);
                d->holders.insert(key, collectionHolder);
            }
        }
    }

    layout();

    emit collectionChanged();
}

void NormalizedMode::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    d->classifier->replace(oldUrl, newUrl);
    d->switchCollection();

    const auto &renameFileData = FileOperatorIns->renameFileData();
    if (renameFileData.contains(oldUrl) && renameFileData.value(oldUrl) == newUrl) {
        FileOperatorIns->removeRenameFileData(oldUrl);
        auto key = d->classifier->key(newUrl);
        if (key.isEmpty()) {
            qWarning() << "warning:can not find key for :" << newUrl;
            return;
        }
        auto holder = d->holders.value(key);
        if (Q_UNLIKELY(!holder)) {
            qWarning() << "warning:can not find holder for :" << key;
            return;
        }

        holder->selectUrl(newUrl, QItemSelectionModel::Select);
    }
}

void NormalizedMode::onFileInserted(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex index = model->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = model->fileUrl(index);
        d->classifier->prepend(url);
   }

    d->switchCollection();
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

    d->switchCollection();
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

void NormalizedMode::onOpenEditor(const QUrl &url)
{
    auto key = d->classifier->key(url);
    if (key.isEmpty())
        return;

    auto holder = d->holders.value(key);
    if (Q_UNLIKELY(!holder))
        return;

    holder->openEditor(url);
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

        // remove old classifier.
        removeClassifier();
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

void NormalizedMode::removeClassifier()
{
    if (d->classifier) {
        if (model && model->handler() == d->classifier->dataHandler())
            model->setHandler(nullptr);

       delete d->classifier;
       d->classifier = nullptr;
    }
}

