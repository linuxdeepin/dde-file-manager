/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractbaseview.h"
#include "dfm-base/dfm_global_defines.h"
#include "workspace/workspace_defines.h"

#include <DListView>

DWIDGET_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

class SelectHelper;
class DragDropHelper;
class ViewDrawHelper;
class FileViewMenuHelper;
class FileViewItem;
class FileViewModel;
class FileViewPrivate;
class BaseItemDelegate;
class FileSortFilterProxyModel;
class FileView final : public DListView, public DFMBASE_NAMESPACE::AbstractBaseView
{
    Q_OBJECT
    friend class SelectHelper;
    friend class DragDropHelper;
    friend class ViewDrawHelper;
    friend class FileViewMenuHelper;
    friend class ShortcutHelper;
    friend class FileViewPrivate;

    QSharedPointer<FileViewPrivate> d;

    using RandeIndex = QPair<int, int>;
    using RandeIndexList = QList<RandeIndex>;

public:
    enum class ClickedAction : uint8_t {
        kClicked = 0,
        kDoubleClicked
    };

    explicit FileView(const QUrl &url, QWidget *parent = nullptr);

    QWidget *widget() const override;
    bool setRootUrl(const QUrl &url) override;
    QUrl rootUrl() const override;
    ViewState viewState() const override;
    QList<QAction *> toolBarActionList() const override;
    QList<QUrl> selectedUrlList() const override;
    void refresh() override;

    void setViewMode(DFMBASE_NAMESPACE::Global::ViewMode mode);
    void setDelegate(DFMBASE_NAMESPACE::Global::ViewMode mode, BaseItemDelegate *view);
    FileViewModel *model() const;
    void setModel(QAbstractItemModel *model) override;

    QModelIndex indexAt(const QPoint &pos) const override;
    virtual QRect visualRect(const QModelIndex &index) const override;

    FileSortFilterProxyModel *proxyModel() const;
    int getColumnWidth(const int &column) const;
    int getHeaderViewWidth() const;
    bool isSelected(const QModelIndex &index) const;
    int selectedIndexCount() const;

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

    using DListView::edit;
    using DListView::updateGeometries;
    using DListView::viewportMargins;

public slots:
    void onHeaderViewMouseReleased();
    void onHeaderSectionResized(int logicalIndex, int oldSize, int newSize);
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void onClicked(const QModelIndex &index);
    void onDoubleClicked(const QModelIndex &index);
    void onScalingValueChanged(const int value);
    void delayUpdateStatusBar();
    void viewModeChanged(quint64 windowId, int viewMode);
    void onRowCountChanged();
    void setFilterData(const quint64 windowID, const QUrl &url, const QVariant &data);
    void setFilterCallback(const quint64 windowID, const QUrl &url, const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback);
    void setMenuScene(const quint64 windowID, const QUrl &url, const QString &scene);

    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;

    void setDetailFileUrl(const QItemSelection &selected, const QItemSelection &deselected);

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

Q_SIGNALS:
    void reqOpenNewWindow(const QList<QUrl> &urls);
    void viewStateChanged();
    void reqOpenAction(const QList<QUrl> &urls, const DirOpenMode openMode = DirOpenMode::kOpenInCurrentWindow);

private slots:
    void loadViewState(const QUrl &url);
    void saveViewModeState();
    void delaySort();
    void onModelStateChanged();
    void setIconSizeBySizeIndex(const int sizeIndex);
    void onShowHiddenFileChanged(bool isShow);

private:
    void initializeModel();
    void initializeDelegate();
    void initializeStatusBar();
    void initializeConnect();

    void updateStatusBar();
    void updateLoadingIndicator();
    void updateContentLabel();
    void setDefaultViewMode();
    void openIndexByClicked(const ClickedAction action, const QModelIndex &index);
    void openIndex(const QModelIndex &index);
    const FileViewItem *sourceItem(const QModelIndex &index) const;

    QVariant fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue);
    void setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value);

    void updateModelActiveIndex();
    RandeIndexList visibleIndexes(QRect rect) const;

    QSize itemSizeHint() const;

    void increaseIcon();
    void decreaseIcon();

    bool isIconViewMode() const;
    bool isListViewMode() const;

    bool cdUp();
    DirOpenMode currentDirOpenMode() const;
};

DPWORKSPACE_END_NAMESPACE

#endif   // FILEVIEW_H
