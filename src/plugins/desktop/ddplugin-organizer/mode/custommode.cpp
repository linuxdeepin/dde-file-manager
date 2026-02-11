// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "custom/custommode_p.h"
#include "models/collectionmodel.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/fileinfomodelshell.h"
#include "config/configpresenter.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"
#include "utils/fileoperator.h"
#include "view/collectionview.h"
#include "interface/canvasmanagershell.h"

#include <dfm-base/dfm_desktop_defines.h>

#include <QDebug>
#include <QUuid>
#include <QMimeData>

using namespace ddplugin_organizer;
using namespace dfmbase;

CustomModePrivate::CustomModePrivate(CustomMode *qq)
    : q(qq)
{

    selectionModel = new ItemSelectionModel(nullptr, qq);
    selectionHelper = new SelectionSyncHelper(qq);
}

CustomModePrivate::~CustomModePrivate()
{
    holders.clear();
}

CustomMode::CustomMode(QObject *parent)
    : CanvasOrganizer(parent), d(new CustomModePrivate(this))
{
    d->dataSyncTimer.setInterval(500);
    d->dataSyncTimer.setSingleShot(true);
    connect(&d->dataSyncTimer, &QTimer::timeout, this, &CustomMode::onItemsChanged);
    fmDebug() << "CustomMode created";
}

CustomMode::~CustomMode()
{
    if (model && (model->handler() == d->dataHandler))
        model->setHandler(nullptr);

    delete d->dataHandler;
    d->dataHandler = nullptr;

    delete d;
    fmDebug() << "CustomMode destroyed";
}

OrganizerMode CustomMode::mode() const
{
    return OrganizerMode::kCustom;
}

bool CustomMode::initialize(CollectionModel *m)
{
    Q_ASSERT(m);
    Q_ASSERT(!d->dataHandler);
    Q_ASSERT(!model);
    model = m;
    d->selectionModel->setModel(m);

    // sync selection
    //d->selectionHelper->setInnerModel(d->selectionModel);
    //d->selectionHelper->setExternalModel(canvasManagerShell->selectionModel());

    connect(CfgPresenter, &ConfigPresenter::newCollection, this, &CustomMode::onNewCollection, Qt::QueuedConnection);

    d->dataHandler = new CustomDataHandler();
    connect(d->dataHandler, &CustomDataHandler::itemsChanged, this, [this]() {
        d->dataSyncTimer.start();
    });   // delay to save collection datas

    // restore collection as config
    QList<CollectionBaseDataPtr> store = CfgPresenter->customProfile();
    d->dataHandler->reset(store);

    model->setHandler(d->dataHandler);
    FileOperatorIns->setDataProvider(d->dataHandler);

    // must be DirectConnection to keep sequential
    connect(model, &CollectionModel::rowsInserted, this, &CustomMode::onFileInserted, Qt::DirectConnection);
    connect(model, &CollectionModel::rowsAboutToBeRemoved, this, &CustomMode::onFileAboutToBeRemoved, Qt::DirectConnection);
    connect(model, &CollectionModel::dataReplaced, this, &CustomMode::onFileRenamed, Qt::DirectConnection);

    // connect after refreshing model to avoid that redundantly rebuilding by modelReset.
    connect(model, &CollectionModel::dataChanged, this, &CustomMode::onFileDataChanged, Qt::QueuedConnection);
    connect(model, &CollectionModel::modelReset, this, &CustomMode::rebuild, Qt::QueuedConnection);

    {
        int srcState = model->modelShell()->modelState();
        if (srcState & 0x1) {   // 0x1 is ready
            model->refresh(model->rootIndex(), false, 0);   // refresh own model.
        } else if (srcState == 0) {   // 0x0 is uninitialized
            model->refresh(model->rootIndex(), true, 0);   // refresh source model.
        } else {
            fmDebug() << "source model is refreshing" << srcState;
        }
    }

    return true;
}

void CustomMode::reset()
{
    // 获取配置
    QList<CollectionBaseDataPtr> store = CfgPresenter->customProfile();
    d->dataHandler->reset(store);

    model->refresh(model->rootIndex(), false, 0);
}

void CustomMode::layout()
{
    const int holderSize = d->holders.values().size();
    if (holderSize < 1) {
        fmDebug() << "No collection holders to layout";
        return;
    }

    QList<CollectionHolderPointer> unassigned;
    QRect *used = new QRect[holderSize];
    int usedCount = 0;

    // first retore geometry as config file.
    for (const CollectionHolderPointer &holder : d->holders.values()) {
        auto style = CfgPresenter->customStyle(holder->id());
        if (style.rect.isValid()) {
            holder->setStyle(style);
            used[usedCount++] = style.rect;
        } else {
            unassigned.append(holder);
        }
    }

    // second assign geomrey for unrecord view
    QRegion usedRegion;
    usedRegion.setRects(used, usedCount);
    delete[] used;

    for (const CollectionHolderPointer &holder : unassigned) {
        // todo 二维矩形排样算法
    }

    // third adjust overlaped or outranged view

    // save style
    QList<CollectionStyle> styles;
    for (const CollectionHolderPointer &holder : d->holders.values())
        styles << holder->style();
    CfgPresenter->writeCustomStyle(styles);
}

void CustomMode::detachLayout()
{
    for (auto holder : d->holders) {
        holder->setSurface(nullptr);
    }
}

void CustomMode::rebuild()
{
    {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        auto files = QSet<QUrl>::fromList(model->files());
#else
        auto files = QSet<QUrl>(model->files().begin(), model->files().end());
#endif
        // remove invaild url
        d->dataHandler->check(files);

        // write config. do not save if no files.
        if (!files.isEmpty()) {
            CfgPresenter->saveCustomProfile(d->dataHandler->baseDatas());
            fmDebug() << "Saved custom profile with" << files.size() << "files";
        }
    }

    // get region name and items, then create collection.
    for (const QString &key : d->dataHandler->keys()) {
        const QString &name = d->dataHandler->name(key);
        auto files = d->dataHandler->items(key);
        fmDebug() << "name" << name << "files" << files.size();

        // 复用已有分组
        CollectionHolderPointer collectionHolder = d->holders.value(key);
        // 创建没有的组

        if (collectionHolder.isNull()) {
            fmDebug() << "Creating new collection holder for" << name;
            collectionHolder.reset(new CollectionHolder(key, d->dataHandler));
            collectionHolder->createFrame(surfaces.first().data(), model);
            // set view
            {
                auto view = collectionHolder->itemView();
                Q_ASSERT(view);
                view->setCanvasModelShell(canvasModelShell);
                view->setCanvasViewShell(canvasViewShell);
                view->setCanvasGridShell(canvasGridShell);
                view->setCanvasManagerShell(canvasManagerShell);
                view->setSelectionModel(d->selectionModel);
            }

            collectionHolder->setName(name);

            connect(collectionHolder.data(), &CollectionHolder::sigRequestClose, this, &CustomMode::onDeleteCollection);
            // temp. save style.
            connect(collectionHolder.data(), &CollectionHolder::styleChanged, this, [this](const QString &id) {
                if (auto holder = d->holders.value(id)) {
                    CfgPresenter->updateCustomStyle(holder->style());
                }
            });

            // enable rename,move,adjust,close,stretch
            collectionHolder->setRenamable(true);
            collectionHolder->setMovable(true);
            collectionHolder->setAdjustable(true);
            collectionHolder->setClosable(true);
            collectionHolder->setStretchable(true);

            d->holders.insert(key, collectionHolder);
            fmInfo() << "Created and configured new collection holder:" << name;
        }

        collectionHolder->show();
    }

    layout();
    emit collectionChanged();
}

void CustomMode::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    d->dataHandler->replace(oldUrl, newUrl).isEmpty();
}

void CustomMode::onFileInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)

    if (first < 0 || last < 0) {
        fmWarning() << "Invalid file insertion range:" << first << "to" << last;
        return;
    }

    const QList<QUrl> &files = model->files();
    if (first >= files.count() || last >= files.count()) {
        fmWarning() << "insert file err:" << first << last << files.count();
        return;
    }

    QString targetKey;
    int targeIndex = 0;
    for (int i = first; i <= last; ++i) {
        auto file = files.at(i);
        if (d->dataHandler->takePreItem(file, targetKey, targeIndex)) {
            d->dataHandler->insert(file, targetKey, targeIndex);
        } else {
            fmInfo() << "it not belong collection:" << file;
            continue;
        }
    }
}

void CustomMode::onFileAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex index = model->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        QUrl url = model->fileUrl(index);
        d->dataHandler->remove(url);
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

bool CustomMode::filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData)
{
    auto urls = mimeData->urls();
    QList<QUrl> collectionItems;
    QStringList files;
    for (auto url : urls) {
        QString &&key = d->dataHandler->key(url);
        if (key.isEmpty())
            continue;
        collectionItems << url;
        files << url.toString();
    }

    if (collectionItems.isEmpty()) {
        fmDebug() << "No collection items found in drop data";
        return false;
    }

    QPoint gridPos = canvasViewShell->gridPos(viewIndex, viewPoint);
    if (!canvasGridShell->item(viewIndex, gridPos).isEmpty()) {
        fmDebug() << "Drop position is not empty, cannot drop collection items";
        return false;
    }

    model->take(collectionItems);
    canvasGridShell->tryAppendAfter(files, viewIndex, gridPos);

    for (auto url : collectionItems)
        canvasModelShell->fetch(url);

    return true;
}

void CustomMode::onNewCollection(const QList<QUrl> &list)
{
    if (list.isEmpty()) {
        fmWarning() << "Cannot create collection with empty file list";
        return;
    }

    // todo 检查数据有效性
    CollectionBaseDataPtr base(new CollectionBaseData);
    base->name = tr("New Collection");
    base->key = QUuid::createUuid().toString(QUuid::WithoutBraces);
    base->items = list;

    d->dataHandler->addBaseData(base);

    Q_ASSERT(!list.isEmpty());
    QPoint gridPos;
    int screen = canvasGridShell->point(list.first().toString(), &gridPos);

    // todo match the surface.
    // screen index
    if (screen > 0) {
        auto rect = canvasViewShell->visualRect(screen, list.first());
        CollectionStyle style;
        style.key = base->key;
        style.screenIndex = screen;
        style.rect = QRect(rect.topLeft(), QSize(rect.width() * 4, rect.height() * 2))
                             .marginsRemoved(QMargins(4, 4, 4, 4));   // defalut size

        // save the style of new collection.
        CfgPresenter->updateCustomStyle(style);
        fmDebug() << "Set initial style for new collection on screen" << screen;
    }

    model->refresh(model->rootIndex(), false, 0);
}

void CustomMode::onDeleteCollection(const QString &key)
{
    QList<QUrl> urls;
    {
        auto bd = d->dataHandler->baseDatas();
        auto it = std::find_if(bd.begin(), bd.end(), [&key](const CollectionBaseDataPtr &handler) {
            return handler->key == key;
        });
        if (it != bd.end())
            urls = (*it)->items;
    }

    d->dataHandler->removeBaseData(key);
    // write config
    CfgPresenter->saveCustomProfile(d->dataHandler->baseDatas());

    if (urls.isEmpty()) {
        d->holders.remove(key);
        fmInfo() << "Collection deleted (was empty):" << key;
        return;
    }

    int viewIndex = 1;
    auto holder = d->holders.take(key);
    auto &&viewName = holder->surface()->property(DesktopFrameProperty::kPropScreenName).toString();
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    for (auto win : root) {
        const QString &&screen = win->property(DesktopFrameProperty::kPropScreenName).toString();
        if (screen == viewName)
            break;
        viewIndex++;
    }
    if (viewIndex >= root.count()) {
        viewIndex = 1;
        fmWarning() << "Could not find matching screen, using default view index";
    }

    auto &&viewPoint = holder->frame()->geometry().topLeft();
    auto &&gridPos = canvasViewShell->gridPos(viewIndex, viewPoint);

    QStringList files;
    for (auto url : urls) {
        files << url.toString();
    }

    model->take(urls);
    canvasGridShell->tryAppendAfter(files, viewIndex, gridPos);

    for (auto url : urls)
        canvasModelShell->fetch(url);
}

void CustomMode::onItemsChanged()
{
    CfgPresenter->saveCustomProfile(d->dataHandler->baseDatas());
}
