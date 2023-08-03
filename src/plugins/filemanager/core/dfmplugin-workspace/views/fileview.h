// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>

#include <DListView>
#include <QDir>

DWIDGET_USE_NAMESPACE
namespace dfmplugin_workspace {

class SelectHelper;
class DragDropHelper;
class ViewDrawHelper;
class FileViewMenuHelper;
class FileViewHelper;
class FileViewItem;
class FileViewModel;
class FileViewPrivate;
class BaseItemDelegate;
class FileView final : public DListView, public DFMBASE_NAMESPACE::AbstractBaseView
{
    Q_OBJECT
    friend class SelectHelper;
    friend class DragDropHelper;
    friend class ViewDrawHelper;
    friend class FileViewMenuHelper;
    friend class ShortcutHelper;
    friend class FileViewPrivate;
    friend class FileViewHelper;

    QSharedPointer<FileViewPrivate> d;

    using RandeIndex = QPair<int, int>;
    using RandeIndexList = QList<RandeIndex>;

public:
    enum class ClickedAction : uint8_t {
        kClicked = 0,
        kDoubleClicked
    };

    explicit FileView(const QUrl &url, QWidget *parent = nullptr);
    ~FileView() override;

    QWidget *widget() const override;
    bool setRootUrl(const QUrl &url) override;
    QUrl rootUrl() const override;
    ViewState viewState() const override;
    QList<QAction *> toolBarActionList() const override;
    QList<QUrl> selectedUrlList() const override;
    void refresh() override;
    void doItemsLayout() override;

    void setViewMode(DFMBASE_NAMESPACE::Global::ViewMode mode);
    DFMBASE_NAMESPACE::Global::ViewMode currentViewMode();
    void setDelegate(DFMBASE_NAMESPACE::Global::ViewMode mode, BaseItemDelegate *view);
    FileViewModel *model() const;
    void setModel(QAbstractItemModel *model) override;
    void stopWork();

    QModelIndex indexAt(const QPoint &pos) const override;
    virtual QRect visualRect(const QModelIndex &index) const override;
    void setIconSize(const QSize &size);
    int horizontalOffset() const override;
    int verticalOffset() const override;

    QList<DFMGLOBAL_NAMESPACE::ItemRoles> getColumnRoles() const;
    int getColumnWidth(const int &column) const;
    int getHeaderViewWidth() const;
    bool isSelected(const QModelIndex &index) const;
    int selectedIndexCount() const;
    void selectFiles(const QList<QUrl> &files) const;
    void setSelectionMode(const SelectionMode mode);
    void setEnabledSelectionModes(const QList<SelectionMode> &modes);
    void setSort(const DFMGLOBAL_NAMESPACE::ItemRoles role, const Qt::SortOrder order);
    void setViewSelectState(bool isSelect);

    void setFilterData(const QUrl &url, const QVariant &data);
    void setFilterCallback(const QUrl &url, const FileViewFilterCallback callback);

    inline void setViewModeToList()
    {
        setViewMode(DFMBASE_NAMESPACE::Global::ViewMode::kListMode);
    }
    inline void setViewModeToIcon()
    {
        setViewMode(DFMBASE_NAMESPACE::Global::ViewMode::kIconMode);
    }

    void setAlwaysOpenInCurrentWindow(bool openInCurrentWindow);

    BaseItemDelegate *itemDelegate() const;
    int itemCountForRow() const;
    int rowCount() const;

    inline int indexOfRow(const QModelIndex &index) const
    {
        return index.row() / itemCountForRow();
    }

    QModelIndex currentPressIndex() const;

    bool isDragTarget(const QModelIndex &index) const;

    QRectF itemRect(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role) const;

    bool isVerticalScrollBarSliderDragging() const;

    using DListView::edit;
    using DListView::updateGeometries;
    using DListView::viewportMargins;

public slots:
    void onHeaderViewMouseReleased();
    void onHeaderSectionResized(int logicalIndex, int oldSize, int newSize);
    void onHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void onHeaderHiddenChanged(const QString &roleName, const bool isHidden);
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void onSectionHandleDoubleClicked(int logicalIndex);
    void onClicked(const QModelIndex &index);
    void onDoubleClicked(const QModelIndex &index);
    void onScalingValueChanged(const int value);
    void viewModeChanged(quint64 windowId, int viewMode);
    void onRowCountChanged();
    void trashStateChanged();
    void onHeaderViewSectionChanged(const QUrl &url);

    void onSelectAndEdit(const QUrl &url);

    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;

    DirOpenMode currentDirOpenMode() const;

    void onWidgetUpdate();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void updateGeometries() override;
    void startDrag(Qt::DropActions supportedActions) override;
    QModelIndexList selectedIndexes() const override;
    void showEvent(QShowEvent *event) override;
    void keyboardSearch(const QString &search) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    bool event(QEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

Q_SIGNALS:
    void reqOpenNewWindow(const QList<QUrl> &urls);
    void viewStateChanged();

private slots:
    void loadViewState(const QUrl &url);
    void saveViewModeState();
    void onModelStateChanged();
    void setIconSizeBySizeIndex(const int sizeIndex);
    void onShowFileSuffixChanged(bool isShow);
    void updateHorizontalOffset();
    void updateView();
    void updateOneView(const QModelIndex &index);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void initializeModel();
    void initializeDelegate();
    void initializeStatusBar();
    void initializeConnect();
    void initializeScrollBarWatcher();

    void delayUpdateStatusBar();
    void updateStatusBar();
    void updateLoadingIndicator();
    void updateContentLabel();
    void updateSelectedUrl();
    void updateListHeaderView();
    void setDefaultViewMode();
    QUrl parseSelectedUrl(const QUrl &url);
    void openIndexByClicked(const ClickedAction action, const QModelIndex &index);
    void openIndex(const QModelIndex &index);

    void setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value);

    RandeIndexList visibleIndexes(QRect rect) const;

    QSize itemSizeHint() const;

    void increaseIcon();
    void decreaseIcon();

    bool isIconViewMode() const;
    bool isListViewMode() const;

    void resetSelectionModes();
    QList<SelectionMode> fetchSupportSelectionModes();

    bool cdUp();
};

}

#endif   // FILEVIEW_H
