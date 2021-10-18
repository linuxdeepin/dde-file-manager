/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

// 修复wayland TASK-37638 添加子线程，完成“选择当前拷贝或剪贴过来的文件”
class SelectWork : public QThread
{
    Q_OBJECT
public:
    explicit SelectWork(QObject *parent = nullptr);
    // 设置初始数据，（未处理的文件 文件模型）
    void setInitData(QList<DUrl> lst, DFileSystemModel *model);
    // 开始线程
    void startWork();
    // 结束线程
    void stopWork();

signals:
    // 发送该信号，选择文件模型
    void sigSetSelect(DUrl url);

protected:
    void run() override;

private:
    QList<DUrl> m_lstNoValid;
    DFileSystemModel *m_pModel;
    bool m_bStop;
};

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

    DUrl rootUrl() const override;
    ViewState viewState() const override;
    QList<DUrl> selectedUrls() const;

    bool isIconViewMode() const;

    int columnWidth(int column) const;
    int headerViewHeight() const;
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

    int horizontalOffset() const override;

    bool isSelected(const QModelIndex &index) const;
    int selectedIndexCount() const;
    QModelIndexList selectedIndexes() const override;

    QModelIndex indexAt(const QPoint &point) const override;
    QRect visualRect(const QModelIndex &index) const override;

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

    QWidget *widget() const override;
    QList<QAction *> toolBarActionList() const override;

    // 设置已被销毁标志
    void setDestroyFlag(bool flag);

    // 设置总是在当前窗口打开子目录
    void setAlwaysOpenInCurrentWindow(bool openInCurrentWindow);

public slots:
    bool cd(const DUrl &url);
    bool cdUp();
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
    void select(const QList<DUrl> &list);
    // 修复wayland BUG-38453 重新添加一个函数，当拷贝和剪贴过后，调用该函数选择全部文件对象
    void selectAllAfterCutOrCopy(const QList<DUrl> &list);
    // 修复wayland TASK-37638 添加槽函数，选中文件
    void slotSetSelect(DUrl url);
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
    void openIndexByOpenAction(const int &action, const QModelIndex &index);

    void setIconSizeBySizeIndex(const int &sizeIndex);

    bool setRootUrl(const DUrl &url) override;
    //获取当前是否可以析构，判断当前是否是在draging中和mousemove中
    bool getCanDestruct() const;

#ifdef SW_LABEL
    bool checkRenamePrivilege_sw(DUrl fileUrl);
#endif

signals:
    void rootUrlChanged(const DUrl &url);
    void viewModeChanged(ViewMode viewMode);
    void viewStateChanged();
    //drop或者mousemove结束请求关闭当前窗口
    void requestWindowDestruct();

    /**
     * @brief fileDialogRename
     * 在弹出对话框中修改重名时发送次信号 bug 63430
     */
    void fileDialogRename();

private slots:
    void dislpayAsActionTriggered(QAction *action);
    void sortByActionTriggered(QAction *action);
    void openWithActionTriggered(QAction *action);
    void onRowCountChanged();
    void updateModelActiveIndex();
    void handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void freshView();
    void loadViewState(const DUrl &url);
    void saveViewState();
    void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void onDriveOpticalChanged(const QString &path);
    void reset() override;
    void setRootIndex(const QModelIndex &index) override;
    void onHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);

protected slots:
    virtual void onRootUrlDeleted(const DUrl &rootUrl);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    bool event(QEvent *e) override;
    void updateGeometries() override;
    bool eventFilter(QObject *obj, QEvent *event) override;

#if QT_CONFIG(draganddrop)
    void startDrag(Qt::DropActions supportedActions) override;
#endif

    void onShowHiddenFileChanged();

    void initDelegate();
    void initUI();
    void initModel();
    void initConnects();

    /**
     * @brief contextMenuEventShowCheck contextMenuEvent之前执行的检查函数
     * 显示右键菜单之前执行的事件之前所执行的检查函数。
     * 该函数可以做一些特殊处理，其返回结果将用作是否执行contextMenuEvent后续执行的判断条件。
     * 可继承dfileview重载实现该函数。
     * ctrl and show url context menu process method.
     * @param event 上下文菜单事件
     * @return bool
     * 如果返回true，则执行contextMenuEvent事件，
     * 返回false，将不会执行contextMenuEvent事件。
     * if return true, can show menu and execute contextMenuEvent method,
     * return false ,can't execute contextMenuEvent method.
     */
    virtual bool canShowContextMenu(QContextMenuEvent *event);

private:
    void increaseIcon();
    void decreaseIcon();
    void openIndex(const QModelIndex &index);
    void keyboardSearch(const QString &search) override;
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
    void updateToolBarActions(QWidget *widget = nullptr, QString theme = "");
    /**
     * @brief normalKeyPressEvent 一般键盘事件
     * 只允许被keyPressEvent调用，用于消除noMotify与keypadMotify之间无break时会产生警告的问题
     * @param event 键盘时间
     * @return 在keyPressEvent是否需要return
     */
    bool normalKeyPressEvent(const QKeyEvent *event);

    using DListView::setOrientation;

    void refresh() override;
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

    // 处理选择文件的子线程对象
    SelectWork  *m_pSelectWork{nullptr};
    bool m_destroyFlag = false;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileView)
    Q_PRIVATE_SLOT(d_ptr, void _q_onSectionHandleDoubleClicked(int))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DFileView::ViewModes)

#endif // DFILEVIEW_H
