#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <dlistview.h>

#include <QFrame>
#include <QLabel>
#include <QUrl>
#include <QActionGroup>
#include <QContextMenuEvent>

#include "durl.h"

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

    void setLocalFileSettings();
    void setNetworkFileSetting();

    DFileSystemModel *model() const;
    DFileItemDelegate *itemDelegate() const;

    DUrl currentUrl() const;
    DUrlList selectedUrls() const;

    bool isIconViewMode() const;

    int columnWidth(int column) const;
    void setColumnWidth(int column, int width);
    int columnCount() const;

    QList<int> columnRoleList() const;

    int selectedIndexCount() const
    { return selectedIndexes().count();}

    int windowId() const;

    using DListView::edit;

    void startKeyboardSearch(int windowId, const QString &key);
    void stopKeyboardSearch(int windowId);

    void setIconSize(const QSize &size);

    ViewMode getDefaultViewMode();

    int getSortRoles();

    bool testViewMode(ViewModes modes, ViewMode mode);

    int horizontalOffset() const;

    bool isSelected(const QModelIndex &index) const;
    QModelIndexList selectedIndexes() const Q_DECL_OVERRIDE;
    QModelIndex indexAt(const QPoint &point) const Q_DECL_OVERRIDE;

    inline QSize itemSizeHint() const
    { return m_itemSizeHint;}

    using DListView::viewportMargins;

public slots:
    void preHandleCd(const FMEvent &event);
    void cd(const FMEvent &event);
    void cdUp(const FMEvent &event);
    void edit(const FMEvent &event);
    bool edit(const QModelIndex & index, EditTrigger trigger, QEvent * event) Q_DECL_OVERRIDE;
    bool select(const FMEvent &event);
    void selectAndRename(const FMEvent &event);
    inline void setViewModeToList()
    { setViewMode(ListMode);}
    inline void setViewModeToIcon()
    { setViewMode(IconMode);}
    inline void setViewModeToExtend()
    { setViewMode(ExtendMode);}
    void setViewMode(ViewMode mode);
    void sort(int role);

    bool setCurrentUrl(DUrl fileUrl);
    void clearHeardView();

    void clearKeyBoardSearchKeys();
    void handleSelectionChanged();
    void setFoucsOnFileView(const FMEvent& event);

    void clearSelection();

    void handleDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles = QVector<int>());
    void updateStatusBar();

signals:
    void currentUrlChanged(const DUrl &url);
    void viewModeChanged(ViewMode viewMode);

private slots:
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
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) Q_DECL_OVERRIDE;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) Q_DECL_OVERRIDE;

private:
    FileController *m_controller;
    FileMenuManager* m_fileMenuManager;
    QHeaderView *m_headerView = Q_NULLPTR;

    QActionGroup* m_displayAsActionGroup;
    QActionGroup* m_sortByActionGroup;
    QActionGroup* m_openWithActionGroup;

    QList<int> m_columnRoles;
    QList<int> m_iconSizes;

    ViewMode m_defaultViewMode = IconMode;
    ViewMode m_currentViewMode = IconMode;

    int m_currentIconSizeIndex = 1;

    inline bool isEmptyArea(const QPoint &pos) const
    { return isEmptyArea(indexAt(pos), pos);}
    bool isEmptyArea(const QModelIndex &index, const QPoint &pos) const;

    QSize currentIconSize() const;

    void enlargeIcon();
    void shrinkIcon();
    void openIndex(const QModelIndex &index);
    void keyboardSearch(const QString & search) Q_DECL_OVERRIDE;
    void stopSearch();
    void updateViewportMargins();
    void switchViewMode(ViewMode mode);
    void showEmptyAreaMenu();
    void showNormalMenu(const QModelIndex &index);
    void updateListHeaderViewProperty();
    void updateExtendHeaderViewProperty();
    void updateItemSizeHint();

    using DListView::setOrientation;

    QPoint m_pressedPos;
    QWidget *m_selectionRectWidget;
    int m_horizontalOffset = 0;

    QTimer* m_keyboardSearchTimer;
    QString m_keyboardSearchKeys;

    QSize m_itemSizeHint;

    /// move cursor later selecte index when pressed key shift
    QModelIndex m_lastCursorIndex;
};

#endif // DFILEVIEW_H
