// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "normalized/normalizedmode_p.h"
#include "models/collectionmodel.h"
#include "config/configpresenter.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasmanagershell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmodelshell.h"
#include "private/surface.h"
#include "utils/fileoperator.h"
#include "utils/renamedialog.h"
#include "view/collectionview.h"
#include "view/collectionframe.h"
#include "view/collectionwidget.h"
#include "delegate/collectionitemdelegate.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-framework/dpf.h>

#include <QScrollBar>
#include <QDebug>
#include <QTime>

#include <DGuiApplicationHelper>

using namespace ddplugin_organizer;

NormalizedModePrivate::NormalizedModePrivate(NormalizedMode *qq)
    : QObject(qq), q(qq)
{
    broker = new NormalizedModeBroker(qq);
    broker->init();

    selectionModel = new ItemSelectionModel(nullptr, qq);
    selectionHelper = new SelectionSyncHelper(qq);
}

NormalizedModePrivate::~NormalizedModePrivate()
{
}

QPoint NormalizedModePrivate::findValidPos(int &currentIndex, const int width, const int height)
{
    if (currentIndex > q->surfaces.count())
        currentIndex = q->surfaces.count();

    auto sur = q->surfaces.at(currentIndex - 1);
    Q_ASSERT(sur);

    auto gridSize = sur->gridSize();

    QPoint pos(-1, -1);
    // from UP to DOWN, RIGHT to LEFT, search an area to place the collection
    for (int x = gridSize.width() - width; x >= 0; --x) {
        for (int y = 0; y < gridSize.height() - height; ++y) {
            QRect gridR { x, y, width, height };
            auto screenR = sur->mapToPixelSize(gridR);
            if (sur->isIntersected(screenR, nullptr))
                continue;
            pos = { x, y };
            x = -1;   // break outside loop.
            break;
        }
    }

    if (pos.x() >= 0 && pos.y() >= 0) {
        fmDebug() << "Found valid position:" << pos << "on surface" << currentIndex;
        return pos;
    }

    if (currentIndex == q->surfaces.count()) {
        fmDebug() << "No space found, using bottom position";
        return { 0, gridSize.height() - height };
    }

    currentIndex += 1;
    return findValidPos(currentIndex, width, height);
}

void NormalizedModePrivate::collectionStyleChanged(const QString &id)
{
    if (auto holder = holders.value(id)) {
        CfgPresenter->updateNormalStyle(generateScreenConfigId(), holder->style());
        // q->layout();
    }
}

CollectionHolderPointer NormalizedModePrivate::createCollection(const QString &id)
{
    QString name = classifier->className(id);
    Q_ASSERT(!name.isEmpty());

    fmInfo() << "Creating new collection:" << name << "with id:" << id;

    CollectionHolderPointer holder;
    holder.reset(new CollectionHolder(id, classifier));
    holder->createFrame(q->surfaces.first().data(), q->model);
    // set view
    {
        auto view = holder->itemView();
        Q_ASSERT(view);
        view->setCanvasModelShell(q->canvasModelShell);
        view->setCanvasViewShell(q->canvasViewShell);
        view->setCanvasGridShell(q->canvasGridShell);
        view->setCanvasManagerShell(q->canvasManagerShell);
        view->setSelectionModel(selectionModel);
    }

    holder->setName(name);
    // disable rename,move,file shift,close,stretch
    holder->setRenamable(false);
    holder->setMovable(true);
    holder->setFileShiftable(false);
    holder->setClosable(false);
    holder->setStretchable(true);
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
                fmDebug() << "Collection " << base->key << "is empty, remove it.";
                holders.remove(base->key);
                changed = true;
            }
        } else {
            if (!base->items.isEmpty()) {
                // create new collection.
                fmDebug() << "Collection " << base->key << "isn't existed, create it.";
                CollectionHolderPointer collectionHolder(createCollection(base->key));
                connectCollectionSignals(collectionHolder);

                holders.insert(base->key, collectionHolder);
                changed = true;
            }
        }
    }

    // relayout all collections.
    if (changed)
        q->layout();
}

void NormalizedModePrivate::openEditor(const QUrl &url)
{
    auto key = classifier->key(url);
    if (key.isEmpty()) {
        fmDebug() << "Cannot open editor: file not in any collection:" << url.toString();
        return;
    }

    auto holder = holders.value(key);
    if (Q_UNLIKELY(!holder)) {
        fmWarning() << "Cannot open editor: collection holder not found for key:" << key;
        return;
    }

    holder->openEditor(url);
}

void NormalizedModePrivate::checkTouchFile(const QUrl &url)
{
    // get the touched file from canvas
    if (url == FileOperatorIns->touchFileData()) {
        FileOperatorIns->clearTouchFileData();
        openEditor(url);
    }
}

void NormalizedModePrivate::checkPastedFiles(const QList<QUrl> &urls)
{
    auto pasteFileData = FileOperatorIns->pasteFileData();
    for (const QUrl &url : urls) {
        if (pasteFileData.contains(url)) {
            FileOperatorIns->removePasteFileData(url);
            auto idx = q->model->index(url);
            if (idx.isValid())
                selectionModel->select(idx, QItemSelectionModel::Select);
        }
    }
}

void NormalizedModePrivate::connectCollectionSignals(CollectionHolderPointer collection)
{
    connect(collection.data(), &CollectionHolder::styleChanged,
            this, &NormalizedModePrivate::collectionStyleChanged);
    connect(collection.data(), &CollectionHolder::frameSurfaceChanged,
            this, &NormalizedModePrivate::updateHolderSurfaceIndex);
    auto frame = dynamic_cast<CollectionFrame *>(collection->frame());
    connect(frame, &CollectionFrame::editingStatusChanged,
            q, &NormalizedMode::onCollectionEditStatusChanged);
    connect(frame, &CollectionFrame::requestChangeSurface,
            q, &NormalizedMode::changeCollectionSurface);
    connect(frame, &CollectionFrame::requestDeactiveAllPredictors,
            q, &NormalizedMode::deactiveAllPredictors);
    connect(frame, &CollectionFrame::moveStateChanged,
            q, &NormalizedMode::onCollectionMoving);
    // QueueConnection: the slot function will be invoked later.
    connect(classifier, &FileClassifier::itemsChanged,
            this, &NormalizedModePrivate::switchCollection, Qt::QueuedConnection);

    {   // update freeze image when signals emitted.
        auto collWidget = collection->widget();
        connect(q, &NormalizedMode::collectionChanged,
                collWidget, &CollectionWidget::cacheSnapshot);
        connect(frame, &CollectionFrame::geometryChanged,
                collWidget, &CollectionWidget::cacheSnapshot);
        connect(collection->itemView(), &CollectionView::iconSizeChanged,
                collWidget, &CollectionWidget::cacheSnapshot);
        connect(collection->itemView()->verticalScrollBar(), &QScrollBar::valueChanged,
                collWidget, &CollectionWidget::cacheSnapshot);
        connect(classifier, &FileClassifier::itemsChanged,
                collWidget, &CollectionWidget::cacheSnapshot);
        connect(CfgPresenter, &ConfigPresenter::optimizeStateChanged,
                collWidget, [collWidget](bool state) {if (state) collWidget->cacheSnapshot(); });
        connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged,
                collWidget, &CollectionWidget::cacheSnapshot);
        connect(dfmbase::ThumbnailFactory::instance(), &dfmbase::ThumbnailFactory::produceFinished,
                collWidget, [collWidget](const QUrl &url) {
                    auto path = url.path();
                    path = path.mid(0, path.lastIndexOf("/"));
                    if (path == dfmbase::StandardPaths::location(dfmbase::StandardPaths::kDesktopPath))
                        collWidget->cacheSnapshot();
                });
        dpfSignalDispatcher->subscribe("ddplugin_background", "signal_Background_BackgroundSetted",
                                       collection->widget(), &CollectionWidget::cacheSnapshot);
        connect(collection->widget(), &QWidget::destroyed, this, [](QObject *obj) {
            dpfSignalDispatcher->unsubscribe("ddplugin_background", "signal_Background_BackgroundSetted",
                                             obj, &CollectionWidget::cacheSnapshot);
        });
    }
}

bool NormalizedModePrivate::tryPlaceRect(QRect &item, const QList<QRect> &inSeats, const QSize &table)
{
    auto isIntersects = [](const QRect &item, const QList<QRect> &items) -> bool {
        for (auto rect : items) {
            if (rect.intersects(item))
                return true;
        }
        return false;
    };

    for (int x = table.width() - item.width(); x >= 0; --x) {
        for (int y = 0; y <= table.height() - item.height(); ++y) {
            item.moveTopLeft({ x, y });
            if (!isIntersects(item, inSeats))
                return true;
        }
    }
    return false;
}

void NormalizedModePrivate::onSelectFile(QList<QUrl> &urls, int flag)
{
    QItemSelection sel;
    for (auto it = urls.begin(); it != urls.end();) {
        auto idx = q->model->index(*it);
        if (idx.isValid()) {
            sel.append(QItemSelectionRange(idx));
            it = urls.erase(it);
        } else {
            ++it;
        }
    }

    if (!sel.isEmpty())
        selectionModel->select(sel, static_cast<QItemSelectionModel::SelectionFlags>(flag));
}

void NormalizedModePrivate::onClearSelection()
{
    Q_ASSERT(q->canvasSelectionShell);
    // the clear canvas seletion will also clear collection
    if (auto cs = q->canvasSelectionShell->selectionModel())
        cs->clear();
    else
        selectionModel->clear();
}

void NormalizedModePrivate::onDropFile(const QString &collection, QList<QUrl> &urls)
{
    // files will be automatically classified to collection.
    urls.clear();
}

void NormalizedModePrivate::onIconSizeChanged()
{
    bool lay = false;
    auto lv = q->canvasManagerShell->iconLevel();
    for (const CollectionHolderPointer &holder : holders.values()) {
        auto view = holder->itemView();
        auto del = view->itemDelegate();
        if (del->iconLevel() != lv) {
            int ret = del->setIconLevel(lv);
            lay |= ret > -1;
        }
        view->updateRegionView();
    }

    //    if (lay)
    //        q->layout();
}

void NormalizedModePrivate::onFontChanged()
{
    for (const CollectionHolderPointer &holder : holders.values()) {
        auto view = holder->itemView();
        view->updateRegionView();
    }

    // q->layout();
}

void NormalizedModePrivate::refreshViews(bool silence)
{
    for (const CollectionHolderPointer &holder : holders.values()) {
        auto view = holder->itemView();
        if (view) view->refresh(silence);
    }
}

void NormalizedModePrivate::updateHolderSurfaceIndex(QWidget *surface)
{
    auto holder = dynamic_cast<CollectionHolder *>(sender());
    if (!holder) return;

    for (int i = 0; i < q->surfaces.count(); ++i) {
        if (surface == q->surfaces.at(i).data()) {
            auto style = holder->style();
            style.screenIndex = i + 1;
            holder->setStyle(style);
            break;
        }
    }
}

bool NormalizedModePrivate::batchRenameFiles()
{
    if (holders.count() == 0)
        return false;

    QList<QUrl> selectedUrls;
    // 1. get files from canvas view.
    auto canvasSelects = dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasView_SelectedUrls", -1).value<QList<QUrl>>();
    selectedUrls.append(canvasSelects);

    // 2. get files from collections
    for (const QModelIndex &idx : selectionModel->selectedIndexes()) {
        auto url = q->getModel()->fileUrl(idx);
        if (url.isValid())
            selectedUrls << url;
    }
    if (selectedUrls.count() <= 1) return false;

    auto view = holders.values().at(0)->itemView();
    RenameDialog renameDlg(selectedUrls.count());
    renameDlg.moveToCenter();

    // see DDialog::exec,it will return the index of buttons
    if (1 == renameDlg.exec()) {
        RenameDialog::ModifyMode mode = renameDlg.modifyMode();
        if (RenameDialog::kReplace == mode) {
            auto content = renameDlg.getReplaceContent();
            FileOperatorIns->renameFiles(view, selectedUrls, content, true);
        } else if (RenameDialog::kAdd == mode) {
            auto content = renameDlg.getAddContent();
            FileOperatorIns->renameFiles(view, selectedUrls, content);
        } else if (RenameDialog::kCustom == mode) {
            auto content = renameDlg.getCustomContent();
            FileOperatorIns->renameFiles(view, selectedUrls, content, false);
        }
    }

    return true;
}

bool NormalizedModePrivate::moveFilesToCanvas(int viewIndex, const QList<QUrl> &urls, const QPoint &viewPoint)
{
    QList<QUrl> collectionItems;
    QStringList files;
    for (auto url : urls) {
        QString &&key = classifier->key(url);
        if (key.isEmpty())
            continue;
        collectionItems << url;
        files << url.toString();
    }

    if (collectionItems.isEmpty()) {
        fmDebug() << "No collection items found in move request";
        return false;
    }

    QPoint gridPos = q->canvasViewShell->gridPos(viewIndex, viewPoint);
    if (!q->canvasGridShell->item(viewIndex, gridPos).isEmpty()) {
        fmDebug() << "Canvas position is not empty, cannot move files";
        return false;
    }

    q->canvasGridShell->tryAppendAfter(files, viewIndex, gridPos);

    for (auto url : collectionItems) {
        classifier->remove(url);
        q->canvasModelShell->fetch(url);
    }

    dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasView_Select", collectionItems);
    return true;
}

void NormalizedModePrivate::restore(const QList<CollectionBaseDataPtr> &cfgs, bool reorganized)
{
    if (cfgs.isEmpty() && CfgPresenter->organizeOnTriggered() && !reorganized) {
        // feature: if org on trigger is enabled and no saved configs
        // files should not be organized when launch.
        classifier->reset({});
        return;
    }

    relayoutedFiles.clear();
    relayoutedCollectionIDs.clear();
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

            // those are not in config files should not be organized.
            if (reorganized || !CfgPresenter->organizeOnTriggered())
                ordered.append(org);
            if (reorganized && !org.isEmpty()) {
                relayoutedFiles.append(org);
                relayoutedCollectionIDs.insert(cfg->key);
            }

            base->items = ordered;
        }
    }
}

QString NormalizedModePrivate::generateScreenConfigId()
{
    QString id = "StyleConfig";
    for (auto surface : q->surfaces) {
        if (surface)
            id += QString("_%1x%2").arg(surface->width()).arg(surface->height());
    }
    return id;
}

NormalizedMode::NormalizedMode(QObject *parent)
    : CanvasOrganizer(parent), d(new NormalizedModePrivate(this))
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
    d->selectionModel->setModel(m);

    // sync selection
    d->selectionHelper->setInnerModel(d->selectionModel);
    d->selectionHelper->setExternalModel(canvasSelectionShell->selectionModel());
    d->selectionHelper->setShell(canvasSelectionShell);
    d->selectionHelper->setEnabled(true);

    // 根据配置创建分类器, 默认按类型
    auto type = CfgPresenter->classification();
    fmInfo() << "classification:" << type;

    setClassifier(type);
    Q_ASSERT(d->classifier);
    FileOperatorIns->setDataProvider(d->classifier);
    // select the file pasted.
    connect(FileOperatorIns, &FileOperator::requestSelectFile, d, &NormalizedModePrivate::onSelectFile, Qt::DirectConnection);
    connect(FileOperatorIns, &FileOperator::requestClearSelection, d, &NormalizedModePrivate::onClearSelection, Qt::DirectConnection);
    connect(FileOperatorIns, &FileOperator::requestDropFile, d, &NormalizedModePrivate::onDropFile, Qt::DirectConnection);

    connect(canvasManagerShell, &CanvasManagerShell::iconSizeChanged, d, &NormalizedModePrivate::onIconSizeChanged);
    connect(canvasManagerShell, &CanvasManagerShell::fontChanged, d, &NormalizedModePrivate::onFontChanged);
    connect(canvasManagerShell, &CanvasManagerShell::requestRefresh, d, &NormalizedModePrivate::refreshViews);

    // must be DirectConnection to keep sequential
    connect(model, &CollectionModel::rowsInserted, this, &NormalizedMode::onFileInserted, Qt::DirectConnection);
    connect(model, &CollectionModel::rowsAboutToBeRemoved, this, &NormalizedMode::onFileAboutToBeRemoved, Qt::DirectConnection);
    connect(model, &CollectionModel::dataReplaced, this, &NormalizedMode::onFileRenamed, Qt::DirectConnection);

    connect(model, &CollectionModel::dataChanged, this, &NormalizedMode::onFileDataChanged, Qt::QueuedConnection);
    connect(
            model, &CollectionModel::modelReset, this, [this] { rebuild(); }, Qt::QueuedConnection);

    connect(CfgPresenter, &ConfigPresenter::reorganizeDesktop, this, &NormalizedMode::onReorganizeDesktop, Qt::QueuedConnection);
    connect(CfgPresenter, &ConfigPresenter::releaseCollection, this, &NormalizedMode::releaseCollection, Qt::QueuedConnection);

    // creating if there already are files.
    if (!model->files().isEmpty()) {
        fmDebug() << "Found existing files, triggering rebuild";
        rebuild();
    }

    return true;
}

void NormalizedMode::reset()
{
    auto type = CfgPresenter->classification();
    fmInfo() << "normalized mode reset to " << type;

    // delete the current classifier
    removeClassifier();

    setClassifier(type);
    Q_ASSERT(d->classifier);
}

void NormalizedMode::layout()
{
    auto holders = d->holders.values();
    {
        const QStringList &ordered = d->classifier->classes();
        const int max = ordered.size();
        std::sort(holders.begin(), holders.end(), [&ordered, max](const CollectionHolderPointer &t1, const CollectionHolderPointer &t2) {
            int i1 = ordered.indexOf(t1->id());
            if (i1 < 0)
                i1 = max;
            int i2 = ordered.indexOf(t2->id());
            if (i2 < 0)
                i2 = max;
            return i1 < i2;
        });
    }

    auto lastConfigId = CfgPresenter->lastStyleConfigId();
    const auto configId = d->generateScreenConfigId();
    auto validConfigId = configId;
    if (!CfgPresenter->hasConfigId(configId)) {
        // 如果当前配置不存在，则使用上一次的配置进行重新排列
        validConfigId = lastConfigId;
    }

    // see if collections should be re-layout
    //      1. calc the bounding rect of all collections which in same screen.
    QList<QRect> orphanRects;   // those collections who's surface loses， should be re-layouted into surface_0.
    QList<QRect> rectsOnSurface0;
    QMap<int, QRect> boundingRects;
    for (int i = 0; i < holders.count(); ++i) {
        const CollectionHolderPointer &holder = holders.at(i);
        auto style = CfgPresenter->normalStyle(validConfigId, holder->id());
        if (style.key.isEmpty()) continue;
        int sIdx = style.screenIndex - 1;
        boundingRects[sIdx] = boundingRects.value(sIdx).united(style.rect);

        if (surfaces.count() > 0) {
            if (sIdx > surfaces.count() - 1)
                orphanRects.append(QRect(QPoint(0, 0), Surface::mapToGridSize(style.rect.size())));
            if (sIdx == 0)
                rectsOnSurface0.append(surfaces[0]->mapToGridGeo(style.rect));
        }
    }

    //      1.1 see if the bounding rect in screen is widther or higher than screen rect
    QMap<int, bool> surfaceRelayout;
    QMap<int, QPoint> offsetOnSurface;   // 屏幕上的集合需要平移的偏移量
    for (auto iter = boundingRects.cbegin(); iter != boundingRects.cend(); ++iter) {
        int idx = iter.key();
        if (idx >= surfaces.count())
            continue;
        auto surface = surfaces.at(idx);
        auto boundingRect = iter.value();
        // 可能存在 surface 能容纳但网格不能容纳的极端场景，因此转换为 gridSize 对应的实际像素大小
        auto avaiSize = QSize { Surface::cellWidth() * surface->gridSize().width(),
                                Surface::cellWidth() * surface->gridSize().height() };
        if (boundingRect.width() > avaiSize.width() || boundingRect.height() > avaiSize.height()) {
            surfaceRelayout.insert(idx, true);
            continue;
        }

        // 判断是否需要整体平移。两个 surface 相并，如果结果不等于大的，说明没有全包裹，则需要平移
        if (boundingRect.united(surface->rect()) != surface->rect()
            && !CfgPresenter->hasConfigId(configId)) {
            fmInfo() << "Collection out of screen detected, surface index:" << idx
                     << "current configId:" << configId << "not exist, try migrate from last config:" << lastConfigId;
            // 获取之前配置的分辨率。configId = StyleConfig_1920x1080_1366x768
            auto resolutions = lastConfigId.split("_").mid(1);
            if (resolutions.count() > idx) {
                auto resolution = resolutions.at(idx);
                auto cood = resolution.split('x');
                if (cood.count() != 2) {
                    fmWarning() << "Incorrect coordinate!" << resolution;
                    continue;
                }

                // 新旧分辨率宽度差值即为偏移量
                // 如果叠加偏移量之后左侧或下侧边缘超出屏幕宽度，则也需要进行调整
                // 则往左/往下放置到网格坐标中最远的位置（从右上算起）
                int oldWidth = cood.at(0).toInt();
                int oldHeight = cood.at(1).toInt();
                fmInfo() << "Resolution changed from" << oldWidth << "x" << oldHeight
                         << "to" << surface->width() << "x" << surface->height()
                         << "boundingRect:" << boundingRect;

                auto dx = surface->width() - oldWidth;
                if (boundingRect.left() + dx < surface->gridMargins().left()) {
                    fmInfo() << "Left edge overflow detected, adjust dx. original dx:" << dx;
                    /* dx = 左边新位置 - 左边旧位置
                     * 新位置：左边距 + 顶点在网格内的 5 像素偏移
                     * 旧位置：原矩形的左边位置 
                     *
                     * 因为布局是以右上角为原点开始的，并且以网格进行对齐，网格边长为 20px，
                     * 从右开始计算，则左边可能会剩余不够 20px 的距离，此为左边距。下边距同理。
                     */
                    dx = surface->gridMargins().left() + 5 - boundingRect.left();
                }

                auto dy = 0;
                if (boundingRect.bottom() > surface->height() - surface->gridMargins().bottom()) {
                    fmInfo() << "Bottom edge overflow detected, adjust dy.";
                    /* dy = 底边新位置 - 底边旧位置
                     * 新位置：surface 底边位置 - 网格底边距 - 网格内 5 像素偏移
                     * 旧位置：原矩形的底边位置 
                     */
                    dy = surface->rect().bottom() - surface->gridMargins().bottom() - 5 - boundingRect.bottom();
                }

                fmInfo() << "Calculated offset for surface" << idx << ": dx=" << dx << ", dy=" << dy;
                offsetOnSurface.insert(idx, { dx, dy });
            } else {
                fmInfo() << "Last screen config is not exist.";
            }
        }
    }

    //      1.2 make sure all orphan rects can be placed on surface 0
    if (orphanRects.count() > 0 && surfaces.count() > 0 && !surfaceRelayout.contains(0)) {
        for (auto &orphan : orphanRects) {
            if (!d->tryPlaceRect(orphan, rectsOnSurface0, surfaces[0]->gridSize())) {
                surfaceRelayout.insert(0, true);
                break;
            }
            rectsOnSurface0.append(orphan);
        }
    }

    //      3. if screen count == 1, make sure that all of the collections can be placed without overlap
    auto prefferDefaultSize = kMiddle;
    if (surfaces.count() == 1 && surfaceRelayout.contains(0)) {   // if re-layout is already decided, only need to decided the default size.
        auto surSize = surfaces.at(0)->gridSize();
        // see if current size can hold all collections with middle size
        auto rows = surSize.height() / kDefaultCollectionSize[kMiddle].height();
        auto cols = surSize.width() / kDefaultCollectionSize[kMiddle].width();
        if (rows * cols < holders.count())
            prefferDefaultSize = kSmall;
    }

    // screen num is start with 1
    static constexpr char kPropertyReLayout[] = "re-layout";
    int screenIdx = 1;
    QList<CollectionStyle> toSave;
    for (int i = 0; i < holders.count(); ++i) {
        const CollectionHolderPointer &holder = holders.at(i);
        auto style = CfgPresenter->normalStyle(validConfigId, holder->id());
        if (Q_UNLIKELY(style.key != holder->id())) {
            if (!style.key.isEmpty())
                fmWarning() << "unknow err:style key is error:" << style.key << ",and fix to :" << holder->id();
            style.key = holder->id();
        }

        if (style.screenIndex > surfaces.count()   // maybe screen count reduced, screenIndex starts at 1
            || surfaceRelayout.contains(style.screenIndex - 1)   // if current surface cannot cover the boundingrect, re-layout items
            || style.screenIndex == -1   // new coming items.
        ) {
            if (!holder->property(kPropertyReLayout).toBool()) {
                holder->setProperty(kPropertyReLayout, true);
                holder->setSurface(nullptr);   // take off it's parent surface so when do re-layout on it, the position can be correctly calculated.
                holders.append(holder);   // add to end of the list so items can be handle later.
                continue;
            }
        }

        if (holder->property(kPropertyReLayout).toBool()) {
            holder->setProperty(kPropertyReLayout, false);
            // need to find a preffered place to place this item.
            auto size = kDefaultCollectionSize.value(prefferDefaultSize);
            auto gridPos = d->findValidPos(screenIdx, size.width(), size.height());
            Q_ASSERT(screenIdx > 0);
            Q_ASSERT(screenIdx <= surfaces.count());
            style.screenIndex = screenIdx;
            style.sizeMode = prefferDefaultSize;

            QRect gridGeo = { gridPos, size };
            auto rect = surfaces.at(screenIdx - 1)->mapToPixelSize(gridGeo);
            style.rect = rect.marginsRemoved({ kCollectionGridMargin,
                                               kCollectionGridMargin,
                                               kCollectionGridMargin,
                                               kCollectionGridMargin });
        }

        // 对坐标进行平移处理
        QPoint offset = offsetOnSurface.value(style.screenIndex - 1, { 0, 0 });
        if (offset.x() != 0 || offset.y() != 0) {
            QRect oldRect = style.rect;
            style.rect.moveLeft(style.rect.left() + offset.x());
            style.rect.moveTop(style.rect.top() + offset.y());
            fmInfo() << "Apply offset to collection, screen:" << style.screenIndex - 1
                     << "offset:" << offset << "rect changed from" << oldRect << "to" << style.rect;
        }

        holder->setSurface(surfaces.at(style.screenIndex - 1).data());
        holder->setStyle(style);
        holder->show();
        toSave << style;
    }

    if (!toSave.isEmpty()) {
        CfgPresenter->writeNormalStyle(configId, toSave);
    }
    // save new screen resolutions.
    QList<QWidget *> surfaceList;
    for (auto s : surfaces)
        surfaceList.append(s.data());
    CfgPresenter->setSurfaceInfo(surfaceList);
    CfgPresenter->setLastStyleConfigId(configId);
}

void NormalizedMode::detachLayout()
{
    for (auto holder : d->holders) {
        holder->setSurface(nullptr);
    }
}

void NormalizedMode::rebuild(bool reorganize)
{
    // 使用分类器对文件进行分类，后续性能问题需考虑异步分类
    const auto profiles = CfgPresenter->normalProfile();
    QElapsedTimer time;
    time.start();
    {
        if (reorganize && d->classifier->updateClassifier())  // classifier's categories should be updated.
            model->refresh(model->rootIndex(), false, 0);
        auto files = model->files();
        d->classifier->reset(files);

        // 从配置文件中恢复集合中元素顺序
        d->restore(profiles, reorganize);

        fmInfo() << QString("Classifying %0 files takes %1 ms").arg(files.size()).arg(time.elapsed());
        time.restart();

        if (!files.isEmpty())
            CfgPresenter->saveNormalProfile(d->classifier->baseData());
    }

    // 从分类器中获取组,根据组创建分区
    for (const QString &key : d->classifier->keys()) {
        auto files = d->classifier->items(key);
        fmDebug() << "type" << key << "files" << files.size();

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
                d->connectCollectionSignals(collectionHolder);
                d->holders.insert(key, collectionHolder);

                bool inProfile = std::any_of(profiles.cbegin(),
                                             profiles.cend(),
                                             [key](const CollectionBaseDataPtr &base) {
                                                 return base->key == key;
                                             });
                if (!d->relayoutedCollectionIDs.contains(key) && !inProfile) {
                    /* 在前序 restore 中进行该变量的填充时，如果此分组不在默认的 profile 中存在
                     * 则该组中的元素不会在集合后被选择。
                     * 因此需要将该分组添加到重排列表中，以便桌面整理后能正常选中该分组内文件。
                     */
                    d->relayoutedCollectionIDs.insert(key);
                    d->relayoutedFiles.append(files);
                }
            }
        }
    }

    layout();

    QTimer::singleShot(0, this, [this] {
        if (!d->relayoutedCollectionIDs.isEmpty()) {
            // QParallelAnimationGroup *anis = new QParallelAnimationGroup();
            for (auto id : d->relayoutedCollectionIDs) {
                // auto ani = d->holders.value(id)->createAnimation();
                // ani->setParent(anis);
                // anis->addAnimation(ani);
                d->holders.value(id)->selectFiles(d->relayoutedFiles);
            }
            // anis->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });

    fmInfo() << QString("create groups %0 takes %1 ms").arg(d->holders.size()).arg(time.elapsed());

    emit collectionChanged();
}

void NormalizedMode::onFileRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (CfgPresenter->organizeOnTriggered()) {
        QString oldType = d->classifier->key(oldUrl);
        if (oldType.isEmpty()) {  // old item is not in collection
            fmDebug() << "Renamed file was not in any collection";
            return;
        }
        QString newType = d->classifier->classify(newUrl);
        if (newType == oldType) {
            auto idx = d->classifier->baseData(oldType)->items.indexOf(oldUrl);
            d->classifier->baseData(oldType)->items.replace(idx, newUrl);
        } else {
            d->classifier->baseData(oldType)->items.removeAll(oldUrl);
            dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasView_Select", QList<QUrl> { newUrl });
        }

        Q_EMIT d->classifier->itemsChanged(oldType);
    } else {
        d->classifier->replace(oldUrl, newUrl);
    }

    d->switchCollection();

    const auto &renameFileData = FileOperatorIns->renameFileData();
    if (renameFileData.contains(oldUrl) && renameFileData.value(oldUrl) == newUrl) {
        FileOperatorIns->removeRenameFileData(oldUrl);
        auto key = d->classifier->key(newUrl);
        if (key.isEmpty()) {
            fmWarning() << "warning:can not find key for :" << newUrl;
            return;
        }
        auto holder = d->holders.value(key);
        if (Q_UNLIKELY(!holder)) {
            fmWarning() << "warning:can not find holder for :" << key;
            return;
        }

        holder->selectUrl(newUrl, QItemSelectionModel::Select);
    }
}

void NormalizedMode::onFileInserted(const QModelIndex &parent, int first, int last)
{
    if (ConfigPresenter::instance()->organizeOnTriggered()) {
        fmDebug() << "File insertion skipped due to organize-on-trigger mode";
        return;
    }

    QList<QUrl> urls;
    for (int i = first; i <= last; i++) {
        QModelIndex index = model->index(i, 0, parent);
        if (Q_UNLIKELY(!index.isValid()))
            continue;
        auto url = model->fileUrl(index);
        d->classifier->prepend(url);
        urls.append(url);
    }

    d->switchCollection();

    if (urls.size() == 1)
        d->checkTouchFile(urls.first());

    d->checkPastedFiles(urls);
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

void NormalizedMode::onReorganizeDesktop()
{
    rebuild(true);
    for (auto type : d->classifier->classes())
        Q_EMIT d->classifier->itemsChanged(type);   // to update the collection view's vertical scroll range.
}

void NormalizedMode::releaseCollection(int category)
{
    if (!d->classifier)
        return;
    QString key = kCategory2Key.value(ItemCategory(category), "");
    if (key.isEmpty())
        return;

    auto files = model->files();
    QList<QUrl> releases;
    for (auto url : files) {
        if (d->classifier->classify(url) == key)
            releases << url;
    }
    if (!releases.isEmpty())
        d->moveFilesToCanvas(0, releases, { 0, 0 });
}

void NormalizedMode::onCollectionEditStatusChanged(bool editing)
{
    this->editing = editing;
}

void NormalizedMode::changeCollectionSurface(const QString &screenName)
{
    auto frame = dynamic_cast<QWidget *>(sender());
    if (!frame) return;

    for (auto surface : surfaces) {
        auto surfaceScreenName = surface->property(dfmbase::DesktopFrameProperty::kPropScreenName).toString();
        if (surfaceScreenName == screenName) {
            frame->setParent(surface.data());
            frame->show();
            break;
        }
    }
}

void NormalizedMode::deactiveAllPredictors()
{
    for (auto surface : surfaces) {
        if (surface)
            surface->deactivatePosIndicator();
    }
}

void NormalizedMode::onCollectionMoving(bool moving)
{
    for (auto holder : d->holders) {
        if (holder)
            holder->setFreeze(moving);
    }
}

bool NormalizedMode::filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData)
{
    if (!CfgPresenter->organizeOnTriggered())
        return false;

    Qt::DropAction act = Qt::DropAction::MoveAction;
    QVariantHash *ext = reinterpret_cast<QVariantHash *>(extData);
    if (ext && ext->contains("QDropEvent")) {
        QDropEvent *event = reinterpret_cast<QDropEvent *>(ext->value("QDropEvent").toLongLong());
        if (event)
            act = event->dropAction();
    }

    if (act == Qt::MoveAction)
        return d->moveFilesToCanvas(viewIndex, mimeData->urls(), viewPoint);
    return false;
}

bool NormalizedMode::filterDataRested(QList<QUrl> *urls)
{
    bool filter { false };
    // remove urls that not contained in collections
    if (urls && d->classifier) {
        for (auto iter = urls->begin(); iter != urls->end();) {
            bool contained { false };
            for (const auto &key : d->classifier->keys()) {
                if (d->classifier->contains(key, *iter)) {
                    contained = true;
                    break;
                }
            }
            if (contained) {
                iter = urls->erase(iter);
                filter = true;
            } else {
                iter++;
            }
        }
    }
    return filter;
}

bool NormalizedMode::filterDataInserted(const QUrl &url)
{
    if (ConfigPresenter::instance()->organizeOnTriggered())
        return false;

    if (d->classifier)
        return d->classifier->acceptInsert(url);

    return false;
}

bool NormalizedMode::filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (!d->classifier)
        return false;

    if (!CfgPresenter->organizeOnTriggered())
        return d->classifier->acceptRename(oldUrl, newUrl);

    QString oldType = d->classifier->key(oldUrl);
    if (oldType.isEmpty())   // old item is not in collection.
        return false;

    QString newType = d->classifier->classify(newUrl);
    if (newType != oldType)   // the renamed result should be placed on desktop not in collection
        return false;

    return true;
}

bool NormalizedMode::filterShortcutkeyPress(int viewIndex, int key, int modifiers) const
{
    // select all
    if (modifiers == Qt::ControlModifier && key == Qt::Key_A)
        d->broker->selectAllItems();

    return CanvasOrganizer::filterShortcutkeyPress(viewIndex, key, modifiers);
}

bool NormalizedMode::filterKeyPress(int viewIndex, int key, int modifiers) const
{
    if (key == Qt::Key_F2 && modifiers == Qt::NoModifier) {
        return d->batchRenameFiles();
    }
    return CanvasOrganizer::filterKeyPress(viewIndex, key, modifiers);
}

bool NormalizedMode::filterContextMenu(int, const QUrl &, const QList<QUrl> &, const QPoint &) const
{
    return isEditing();
}

bool NormalizedMode::setClassifier(Classifier id)
{
    if (d->classifier) {
        if (d->classifier->mode() == id) {
            fmDebug() << "ingore setting, current classifier was" << id;
            return true;
        }

        // remove old classifier.
        fmInfo() << "Removing old classifier and setting new one:" << id;
        removeClassifier();
    } else {
        fmInfo() << "Setting initial classifier:" << id;
    }

    // clear all collections
    d->holders.clear();

    d->classifier = ClassifierCreator::createClassifier(id);
    if (!d->classifier) {
        fmWarning() << "Failed to create classifier:" << id;
        return false;
    }

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
