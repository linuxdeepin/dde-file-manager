// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileview_p.h"
#include "views/headerview.h"
#include "views/fileviewstatusbar.h"
#include "views/baseitemdelegate.h"
#include "views/iconitemdelegate.h"
#include "models/fileviewmodel.h"
#include "utils/workspacehelper.h"
#include "utils/dragdrophelper.h"
#include "utils/viewdrawhelper.h"
#include "utils/selecthelper.h"
#include "utils/shortcuthelper.h"
#include "utils/fileoperatorhelper.h"
#include "utils/fileviewmenuhelper.h"
#include "utils/viewanimationhelper.h"
#include "utils/fileviewhelper.h"
#include "events/workspaceeventcaller.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/viewdefines.h>

#include <DGuiApplicationHelper>

#include <QScrollBar>
#include <QVBoxLayout>

inline constexpr int kContentLabelMinWidth { 145 };
inline constexpr int kContentLabelMinHeight { 60 };

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;
DGUI_USE_NAMESPACE

FileViewPrivate::FileViewPrivate(FileView *qq)
    : q(qq)
{
    fmDebug() << "FileViewPrivate initializing for FileView";

    dragDropHelper = new DragDropHelper(qq);
    viewDrawHelper = new ViewDrawHelper(qq);
    selectHelper = new SelectHelper(qq);
    shortcutHelper = new ShortcutHelper(qq);
    viewMenuHelper = new FileViewMenuHelper(qq);
    animationHelper = new ViewAnimationHelper(qq);

    enabledSelectionModes << FileView::NoSelection << FileView::SingleSelection
                          << FileView::MultiSelection << FileView::ExtendedSelection
                          << FileView::ContiguousSelection;

    allowedAdjustColumnSize = Application::instance()->appAttribute(Application::kViewSizeAdjustable).toBool();

    fmDebug() << "FileViewPrivate initialization completed - column size adjustable:" << allowedAdjustColumnSize;
}

int FileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    return columnCountByCalc;
}

int FileViewPrivate::calcColumnCount(int widgetWidth, int itemWidth) const
{
    if (itemWidth <= 0)
        itemWidth = q->itemSizeHint().width();

    int availableWidth = widgetWidth - 2 * kIconHorizontalMargin;

    // 计算列数
    int columnCount = (availableWidth + 2 * q->spacing()) / (itemWidth + 2 * q->spacing());

    return columnCount;
}

QUrl FileViewPrivate::modelIndexUrl(const QModelIndex &index) const
{
    return index.data().toUrl();
}

void FileViewPrivate::initIconModeView()
{
    fmDebug() << "Initializing icon mode view";

    if (headerWidget) {
        headerWidget->setVisible(false);

        if (headerView) {
            headerView->disconnect();
            auto headerLayout = qobject_cast<QVBoxLayout *>(headerWidget->layout());
            headerLayout->takeAt(0);
            delete headerView;
            headerView = nullptr;
            fmDebug() << "Header view removed for icon mode";
        }
    }

    if (statusBar) {
        statusBar->setScalingVisible(true);
        DFMBASE_NAMESPACE::ViewDefines viewDefines;
        q->setIconSize(QSize(viewDefines.iconSize(currentIconSizeLevel),
                             viewDefines.iconSize(currentIconSizeLevel)));
        QSignalBlocker blocker(statusBar->scalingSlider());
        statusBar->scalingSlider()->setValue(currentIconSizeLevel);
        fmDebug() << "Icon size set to level:" << currentIconSizeLevel;
    }

    if (q->itemDelegate()) {
        q->itemDelegate()->setIconSizeByIconSizeLevel(currentIconSizeLevel);
        
        // Set width level only for icon delegate
        auto iconDelegate = dynamic_cast<IconItemDelegate*>(q->itemDelegate());
        if (iconDelegate) {
            iconDelegate->setItemMinimumWidthByWidthLevel(currentGridDensityLevel);
            fmDebug() << "Item delegate configured for icon mode - density level:" << currentGridDensityLevel;
        }
    }
}

void FileViewPrivate::initListModeView()
{
    fmDebug() << "Initializing list mode view";

    if (q->itemDelegate()) {
        q->itemDelegate()->setIconSizeByIconSizeLevel(currentListHeightLevel);
        fmDebug() << "Item delegate height level set to:" << currentListHeightLevel;
    }

    if (!headerWidget) {
        headerWidget = new QWidget(q);
        QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
        headerLayout->setContentsMargins(0, 0, 0, 10);
        headerLayout->setAlignment(Qt::AlignTop);
        headerWidget->installEventFilter(q);
        q->addHeaderWidget(headerWidget);
        fmDebug() << "Header widget created for list mode";
    }

    if (!headerView) {
        q->initDefaultHeaderView();
        auto headerLayout = qobject_cast<QVBoxLayout *>(headerWidget->layout());

        headerView = new HeaderView(Qt::Orientation::Horizontal, q);

        headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        headerView->setFixedHeight(kListViewHeaderHeight);
        headerView->setMinimumSectionSize(GlobalPrivate::kListViewMinimumWidth);

        headerView->setModel(q->model());
        if (q->selectionModel()) {
            headerView->setSelectionModel(q->selectionModel());
        }

        headerLayout->addWidget(headerView);

        QObject::connect(headerView, &HeaderView::mousePressed, q, &FileView::onHeaderViewMousePressed);
        QObject::connect(headerView, &HeaderView::mouseReleased, q, &FileView::onHeaderViewMouseReleased);
        QObject::connect(headerView, &HeaderView::sectionResized, q, &FileView::onHeaderSectionResized);
        QObject::connect(headerView, &HeaderView::sortIndicatorChanged, q, &FileView::onSortIndicatorChanged);
        QObject::connect(headerView, &HeaderView::sectionMoved, q, &FileView::onHeaderSectionMoved);
        QObject::connect(headerView, &HeaderView::sectionHandleDoubleClicked, q, &FileView::onSectionHandleDoubleClicked);
        QObject::connect(headerView, &HeaderView::hiddenSectionChanged, q, &FileView::onHeaderHiddenChanged);
        QObject::connect(q->horizontalScrollBar(), &QScrollBar::valueChanged, headerView, [=](int value) {
            headerView->move(-value, headerView->y());
        });

        fmDebug() << "Header view created and configured for list mode";
    }

    headerWidget->setVisible(true);

    if (statusBar) {
        statusBar->setScalingVisible(false);
        fmDebug() << "Status bar scaling disabled for list mode";
    }
}

QModelIndexList FileViewPrivate::selectedDraggableIndexes()
{
    QModelIndexList indexes = q->selectedIndexes();

    auto isNotDragEnabled = [=](const QModelIndex &index) {
        return !(q->model()->flags(index) & Qt::ItemIsDragEnabled);
    };

    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), isNotDragEnabled),
                  indexes.end());

    return indexes;
}

void FileViewPrivate::initContentLabel()
{
    if (!contentLabel) {
        fmDebug() << "Creating content label for empty view";

        contentLabel = new QLabel(q);
        contentLabel->setMinimumSize(kContentLabelMinWidth, kContentLabelMinHeight);
        contentLabel->setAlignment(Qt::AlignCenter);

        QColor color = (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::LightType)
                ? QColor(0, 0, 0, 102)
                : QColor(255, 255, 255, 102);
        QPalette palette = contentLabel->palette();
        palette.setColor(QPalette::Text, color);
        contentLabel->setPalette(palette);
        QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                         contentLabel, [this](DGuiApplicationHelper::ColorType themeType) {
                             QColor textColor = (themeType == DGuiApplicationHelper::ColorType::LightType)
                                     ? QColor(0, 0, 0, 102)
                                     : QColor(255, 255, 255, 102);
                             QPalette labelPalette = contentLabel->palette();
                             labelPalette.setColor(QPalette::Text, textColor);
                             contentLabel->setPalette(labelPalette);
                         });

        DFontSizeManager::instance()->bind(contentLabel, DFontSizeManager::SizeType::T6);

        contentLabel.setCenterIn(q);
        contentLabel->setStyleSheet(q->styleSheet());
        contentLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        contentLabel->show();

        fmDebug() << "Content label created and configured";
    }
}

void FileViewPrivate::updateHorizontalScrollBarPosition()
{
    QWidget *widget = qobject_cast<QWidget *>(q->horizontalScrollBar()->parentWidget());

    widget->move(widget->x(), q->height() - statusBar->height() - widget->height());
}

void FileViewPrivate::pureResizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    if (currentViewMode == Global::ViewMode::kListMode || currentViewMode == Global::ViewMode::kTreeMode) {
        if (adjustFileNameColumn && headerView)
            headerView->doFileNameColumnResize(q->width());
    } else {
        if (animationHelper)
            animationHelper->aboutToPlay();
    }
}

void FileViewPrivate::loadViewMode(const QUrl &url)
{
    fmDebug() << "Loading view mode for URL:" << url.toString();

    int defaultViewMode = static_cast<int>(WorkspaceHelper::instance()->findViewMode(url.scheme()));
    int savedViewMode = fileViewStateValue(url, "viewMode", -1).toInt();
    int parentViewMode = -1;

    if (savedViewMode == -1
        && Application::appObtuselySetting()->value("ApplicationAttribute", "UseParentViewMode", false).toBool()) {
        auto info = InfoFactory::create<FileInfo>(url);
        QList<QUrl> parentUrlList {};
        UrlRoute::urlParentList(url, &parentUrlList);

        for (const QUrl &parentUrl : parentUrlList) {
            parentViewMode = fileViewStateValue(parentUrl, "viewMode", -1).toInt();
            break;
        }
    }

    if (parentViewMode != -1) {
        currentViewMode = static_cast<Global::ViewMode>(parentViewMode);
    } else if (savedViewMode != -1 && WorkspaceHelper::instance()->isViewModeSupported(url.scheme(), static_cast<Global::ViewMode>(savedViewMode))) {   // saved view mode in old version may not be supported
        currentViewMode = static_cast<Global::ViewMode>(savedViewMode);
        fmDebug() << "Using saved view mode:" << savedViewMode;
    } else {
        currentViewMode = static_cast<Global::ViewMode>(defaultViewMode);
        fmDebug() << "Using default view mode:" << defaultViewMode;
    }

    if (currentViewMode == Global::ViewMode::kTreeMode && !DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()) {
        currentViewMode = Global::ViewMode::kListMode;
        fmDebug() << "Tree mode disabled, switching to list mode";
    }

    auto winId = WorkspaceHelper::instance()->windowId(q);
    WorkspaceEventCaller::sendViewModeChanged(winId, currentViewMode);

    fmInfo() << "View mode loaded:" << static_cast<int>(currentViewMode) << "for URL:" << url.toString();
}

QVariant FileViewPrivate::fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue)
{
    return WorkspaceHelper::instance()->getFileViewStateValue(url, key, defalutValue);
}

void FileViewPrivate::updateHorizontalOffset()
{
    horizontalOffset = 0;
    if (q->isIconViewMode()) {


        int contentWidth = q->maximumViewportSize().width();
        int itemWidth = q->itemSizeHint().width() + q->spacing() * 2;
        int itemColumn = 0;
        if (itemWidth <= 0) {
            fmDebug() << "Invalid item width, skipping offset calculation";
            return;
        }
        // 根据qt虚函数去计算当前的itemColumn（每行绘制的个数）
        int startLeftPx = q->visualRect(q->model()->index(0, 0, q->rootIndex())).left();
        int rowCount = q->model()->rowCount(q->rootIndex());
        int maxColumnCount = qCeil(contentWidth / (60 + q->spacing() * 2)) + 2;   // 60是item最小宽度
        // group绘制时，设置分组为正常显示，才能正常计算出每一行绘制多少个item
        if (!q->model()->index(0,0,q->rootIndex()).data(Global::ItemRoles::kItemGroupHeaderKey).toString().isEmpty()) {
            q->model()->updateHorizontalOffset(true);
        }
        for (int i = 1; i < qMax(maxColumnCount, rowCount); i++) {
            int itemLeft = q->visualRect(q->model()->index(i, 0, q->rootIndex())).left();
            // NOTE：如果实际item数量不足以绘制到第二行，qt将不会在位置计算中加上边距，
            // 会导致新计算出的第二行itemleft比第一行少一个边距的值，所以这里需要用大于等于
            if (startLeftPx >= itemLeft) {
                itemColumn = i;
                break;
            }
        }

        // 关闭更新状态
        q->model()->updateHorizontalOffset(false);

        columnCountByCalc = itemColumn;
        // 如果itemColumn为0或itemColumn大于等于实际item数量，则说明当前只有一行，则水平偏移量为默认偏移
        if (itemColumn <= 0 || itemColumn >= rowCount) {
            return;
        }

        // itemColumn每行绘制的个数，contentWidth绘制区域宽度，itemWidth每一个item + 2倍间距的绘制宽度
        if (contentWidth - itemWidth * itemColumn <= 0
            || (contentWidth - itemWidth * itemColumn) / 2 >= itemWidth) {
            columnCountByCalc = 1;
            initHorizontalOffset = false;
            fmDebug() << "Resetting to single column layout";
            return;
        }
        horizontalOffset = -(contentWidth - itemWidth * itemColumn) / 2;
    }
}

