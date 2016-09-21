#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <dlistview.h>

#include "durl.h"

#include <QDir>

class DFileSystemModel;

QT_BEGIN_NAMESPACE
class QHeaderView;
class QTimer;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DAction;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class FileController;
class FileMenuManager;
class FMEvent;
class DFileSystemModel;
class DFileItemDelegate;
class AbstractFileInfo;
class DStatusBar;
class DFileViewPrivate;
class DFileView : public DListView
{
    Q_OBJECT
public:
    enum ViewMode {
        IconMode = 0x01,
        ListMode = 0x02,
        ExtendMode = 0x04,
        AllViewMode = IconMode | ListMode | ExtendMode
    };

    Q_DECLARE_FLAGS(ViewModes, ViewMode)

    explicit DFileView(QWidget *parent = 0);
    ~DFileView();

    void initUI();
    void initDelegate();
    void initModel();
    void initConnects();
    void initActions();
    void initKeyboardSearchTimer();

    DFileSystemModel *model() const;
    DFileItemDelegate *itemDelegate() const;
    DStatusBar *statusBar() const;

    DUrl currentUrl() const;
    DUrlList selectedUrls() const;

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

    int windowId() const;

    using DListView::edit;

    void setIconSize(const QSize &size);

    ViewMode getDefaultViewMode() const;
    ViewMode viewMode() const;

    int getSortRoles() const;

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

    bool isCutIndex(const QModelIndex &index) const;
    bool isActiveIndex(const QModelIndex &index) const;

    QList<QIcon> fileAdditionalIcon(const QModelIndex &index) const;

public slots:
    void preHandleCd(const FMEvent &event);
    void cd(const FMEvent &event);
    void cd(const DUrl &url);
    void cdUp(const FMEvent &event);
    void edit(const FMEvent &event);
    bool edit(const QModelIndex & index, EditTrigger trigger, QEvent * event) Q_DECL_OVERRIDE;
    bool select(const FMEvent &event);
    void select(const DUrlList &list);
    void selectAndRename(const FMEvent &event);
    inline void setViewModeToList()
    { setViewMode(ListMode);}
    inline void setViewModeToIcon()
    { setViewMode(IconMode);}
    inline void setViewModeToExtend()
    { setViewMode(ExtendMode);}
    void setViewMode(ViewMode mode);
    void sortByRole(int role);
    void sortByColumn(int column);

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &nameFilters);
    QDir::Filters filters() const;
    void setFilters(QDir::Filters filters);

    void clearHeardView();

    void clearKeyBoardSearchKeys();
    void setFoucsOnFileView(const FMEvent& event);
    void refreshFileView(const FMEvent& event);

    void clearSelection();

    void handleDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles = QVector<int>());
    void updateStatusBar();

    void setSelectionRectVisible(bool visible);
    bool isSelectionRectVisible() const;
    bool canShowSelectionRect() const;

    void setContentLabel(const QString &text);
    void setIconSizeWithIndex(const int index);

signals:
    void currentUrlChanged(const DUrl &url);
    void viewModeChanged(ViewMode viewMode);

private slots:
    bool setCurrentUrl(DUrl fileUrl);
    void selectAll(int windowId);
    void dislpayAsActionTriggered(QAction * action);
    void sortByActionTriggered(QAction * action);
    void openWithActionTriggered(QAction * action);

    void handleCommitData(QWidget * editor);

protected:
    void wheelEvent(QWheelEvent * event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
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
    void rowsInserted(const QModelIndex & parent, int start, int end);

private:
    inline bool isEmptyArea(const QPoint &pos) const
    { return isEmptyArea(indexAt(pos), pos);}
    bool isEmptyArea(const QModelIndex &index, const QPoint &pos) const;

    QSize currentIconSize() const;

    void enlargeIcon();
    void shrinkIcon();
    void openIndex(const QModelIndex &index);
    void keyboardSearch(const QString & search) Q_DECL_OVERRIDE;
    void updateHorizontalOffset();
    void switchViewMode(ViewMode mode);
    void showEmptyAreaMenu();
    void showNormalMenu(const QModelIndex &index);
    void updateListHeaderViewProperty();
    void updateExtendHeaderViewProperty();
    void updateItemSizeHint();
    void updateColumnWidth();
    void popupHeaderViewContextMenu(const QPoint &pos);
    void onModelStateChanged(int state);
    void updateContentLabel();
    void updateSelectionRect();

    using DListView::setOrientation;

    void preproccessDropEvent(QDropEvent *event) const;

    QScopedPointer<DFileViewPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileView)
};

#endif // DFILEVIEW_H
