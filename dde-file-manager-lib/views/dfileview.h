#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include "interfaces/dfmsetting.h"
#include "dfmglobal.h"
#include "durl.h"
#include "dfmbaseview.h"

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
class DStyledItemDelegate;
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

    explicit DFileView(QWidget *parent = 0);
    ~DFileView();

    DFileSystemModel *model() const;
    DStyledItemDelegate *itemDelegate() const;
    void setItemDelegate(DStyledItemDelegate *delegate);
    DStatusBar *statusBar() const;
    FileViewHelper *fileViewHelper() const;

    DUrl rootUrl() const Q_DECL_OVERRIDE;
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

    QPair<int, Qt::SortOrder> getSortRoles() const;
    void setSortRoles(const int& role, const Qt::SortOrder& order);

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
    void setViewMode(ViewMode mode);
    void sortByColumn(int column);
    void sort(int column, Qt::SortOrder order);

    void setNameFilters(const QStringList &nameFilters);
    void setFilters(QDir::Filters filters);

    void clearHeardView();
    void clearSelection();

    void setContentLabel(const QString &text);

    void setMenuActionWhitelist(const QSet<DFMGlobal::MenuAction> &actionList);
    void setMenuActionBlacklist(const QSet<DFMGlobal::MenuAction> &actionList);

    void delayUpdateStatusBar();
    void updateStatusBar();
    void openIndexByOpenAction(const int &action, const QModelIndex& index);

    void setIconSizeBySizeIndex(const int& sizeIndex);

#ifdef SW_LABEL
    bool checkRenamePrivilege_sw(DUrl fileUrl);
#endif

signals:
    void rootUrlChanged(const DUrl &url);
    void viewModeChanged(ViewMode viewMode);
    void viewStateChanged();

private slots:
    bool setRootUrl(const DUrl &url) Q_DECL_OVERRIDE;
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

protected:
    void wheelEvent(QWheelEvent * event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
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

    using DListView::setOrientation;

    void preproccessDropEvent(QDropEvent *event) const;
    void refresh() Q_DECL_OVERRIDE;

    QScopedPointer<DFileViewPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileView)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DFileView::ViewModes)

#endif // DFILEVIEW_H
