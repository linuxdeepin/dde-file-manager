/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include "dfmglobal.h"
#include "durl.h"
#include "dfmbaseview.h"
#include "dfmstyleditemdelegate.h"

#include <dlistview.h>

#include <QDir>

class DFileSystemModel;

QT_BEGIN_NAMESPACE
class QHeaderView;
class QTimer;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE
DFM_USE_NAMESPACE

class FileController;
class DFileMenuManager;
class DFileSystemModel;
class DAbstractFileInfo;
class DStatusBar;
class FileViewHelper;
class DFileViewPrivate;
class DFileView : public DListView, public DFMBaseView
{
    Q_OBJECT
public:
    enum ViewMode {
        IconMode = 0x01,
        ListMode = 0x02,
        ExtendMode = 0x04,
        AllViewMode = IconMode | ListMode | ExtendMode
    };
    Q_ENUM(ViewMode)

    Q_DECLARE_FLAGS(ViewModes, ViewMode)

    explicit DFileView(QWidget *parent = nullptr);
    ~DFileView() override;

    DFileSystemModel *model() const;
    DFMStyledItemDelegate *itemDelegate() const;
    void setItemDelegate(DFMStyledItemDelegate *delegate);
    DStatusBar *statusBar() const;
    FileViewHelper *fileViewHelper() const;

    DUrl rootUrl() const Q_DECL_OVERRIDE;
    ViewState viewState() const override;
    QList<DUrl> selectedUrls() const;

    bool isIconViewMode() const;

    int columnWidth(int column) const;
    void setColumnWidth(int column, int width);
    int columnCount() const;

    int rowCount() const;

    /// icon view row index count
    int itemCountForRow() const;
    inline int indexOfRow(const QModelIndex &index) const
    { return index.row() / itemCountForRow();}

    QList<int> columnRoleList() const;

    quint64 windowId() const;

    using DListView::edit;

    void setIconSize(const QSize &size);

    ViewMode getDefaultViewMode() const;
    ViewMode viewMode() const;

    bool testViewMode(ViewModes modes, ViewMode mode) const;

    int horizontalOffset() const Q_DECL_OVERRIDE;

    bool isSelected(const QModelIndex &index) const;
    int selectedIndexCount() const;
    QModelIndexList selectedIndexes() const Q_DECL_OVERRIDE;

    QModelIndex indexAt(const QPoint &point) const Q_DECL_OVERRIDE;
    QRect visualRect(const QModelIndex &index) const Q_DECL_OVERRIDE;

    typedef QPair<int, int> RandeIndex;
    typedef QList<RandeIndex> RandeIndexList;
    RandeIndexList visibleIndexes(QRect rect) const;

    QSize itemSizeHint() const;

    using DListView::viewportMargins;
    using DListView::updateGeometries;

    bool isDropTarget(const QModelIndex &index) const;

    QStringList nameFilters() const;
    QDir::Filters filters() const;

    void setEnabledSelectionModes(const QSet<SelectionMode> &list);
    QSet<SelectionMode> enabledSelectionModes() const;

    QWidget *widget() const Q_DECL_OVERRIDE;
    QList<QAction*> toolBarActionList() const Q_DECL_OVERRIDE;

public slots:
    bool cd(const DUrl &url);
    bool cdUp();
    bool edit(const QModelIndex & index, EditTrigger trigger, QEvent * event) Q_DECL_OVERRIDE;
    void select(const QList<DUrl> &list);
    inline void setViewModeToList()
    { setViewMode(ListMode);}
    inline void setViewModeToIcon()
    { setViewMode(IconMode);}
    inline void setViewModeToExtend()
    { setViewMode(ExtendMode);}
    void setDefaultViewMode(ViewMode mode);
    void setViewMode(ViewMode mode);
    void sortByRole(int role, Qt::SortOrder order);

    void setNameFilters(const QStringList &nameFilters);
    void setFilters(QDir::Filters filters);
    void setAdvanceSearchFilter(const QMap<int, QVariant> &formData, bool turnOn = true, bool avoidUpdateView = false);

    void clearHeardView();
    void clearSelection();

    void setContentLabel(const QString &text);

    void setMenuActionWhitelist(const QSet<DFMGlobal::MenuAction> &actionList);
    void setMenuActionBlacklist(const QSet<DFMGlobal::MenuAction> &actionList);

    void delayUpdateStatusBar();
    void updateStatusBar();
    void openIndexByOpenAction(const int &action, const QModelIndex& index);

    void setIconSizeBySizeIndex(const int& sizeIndex);

    bool setRootUrl(const DUrl &url) override;

#ifdef SW_LABEL
    bool checkRenamePrivilege_sw(DUrl fileUrl);
#endif

signals:
    void rootUrlChanged(const DUrl &url);
    void viewModeChanged(ViewMode viewMode);
    void viewStateChanged();

private slots:
    void dislpayAsActionTriggered(QAction * action);
    void sortByActionTriggered(QAction * action);
    void openWithActionTriggered(QAction * action);
    void onRowCountChanged();
    void updateModelActiveIndex();
    void handleDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles = QVector<int>());
    void onRootUrlDeleted(const DUrl &rootUrl);
    void freshView();
    void loadViewState(const DUrl &url);
    void saveViewState();
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void onDriveOpticalChanged(const QString& path);
    void reset() override;
    void setRootIndex(const QModelIndex &index) override;

protected:
    void wheelEvent(QWheelEvent * event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) Q_DECL_OVERRIDE;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) Q_DECL_OVERRIDE;
    void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end) Q_DECL_OVERRIDE;
    void rowsInserted(const QModelIndex & parent, int start, int end) Q_DECL_OVERRIDE;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) Q_DECL_OVERRIDE;
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    void updateGeometries() override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void onShowHiddenFileChanged();

    void initDelegate();
    void initUI();
    void initModel();
    void initConnects();

private:
    void increaseIcon();
    void decreaseIcon();
    void openIndex(const QModelIndex &index);
    void keyboardSearch(const QString & search) Q_DECL_OVERRIDE;
    void updateHorizontalOffset();
    void switchViewMode(ViewMode mode);
    void showEmptyAreaMenu(const Qt::ItemFlags &indexFlags);
    void showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags);
    void updateListHeaderViewProperty();
    void updateExtendHeaderViewProperty();
    void updateColumnWidth();
    void popupHeaderViewContextMenu(const QPoint &pos);
    void onModelStateChanged(int state);
    void updateContentLabel();
    void updateToolBarActions(QWidget* widget = nullptr, QString theme = "");

    using DListView::setOrientation;

    void refresh() Q_DECL_OVERRIDE;
    bool fetchDragEventUrlsFromSharedMemory();

    /**
     * @brief setTargetUrlToApp 为其他应用设置目标Url
     * 仅当被拖拽行为是DFileDrag时会被调用
     * @param data mimeData
     * @param url 目标Url
     */
    void setTargetUrlToApp(const QMimeData *data, const DUrl &url);


    bool m_isRemovingCase = false;
    QScopedPointer<DFileViewPrivate> d_ptr;
    QList<QUrl> m_urlsForDragEvent;
    DUrl m_currentTargetUrl; //缓存当前目标Url

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileView)
    Q_PRIVATE_SLOT(d_ptr, void _q_onSectionHandleDoubleClicked(int))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DFileView::ViewModes)

#endif // DFILEVIEW_H
