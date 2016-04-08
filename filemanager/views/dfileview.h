#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <dlistview.h>

#include <QFrame>
#include <QUrl>
#include <QContextMenuEvent>

class FileController;
class FileMenuManager;
class FMEvent;

QT_BEGIN_NAMESPACE
class QFileSystemModel;
class QHeaderView;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DAction;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFileSystemModel;
class DFileItemDelegate;
class AbstractFileInfo;

class DFileView : public DListView
{
    Q_OBJECT
public:
    explicit DFileView(QWidget *parent = 0);
    ~DFileView();

    void initUI();
    void initDelegate();
    void initModel();
    void initConnects();

    DFileSystemModel *model() const;
    DFileItemDelegate *itemDelegate() const;

    QString currentUrl() const;
    QList<QString> selectedUrls() const;

    bool isIconViewMode();

    int columnWidth(int column) const;
    void setColumnWidth(int column, int width);
    int columnCount() const;

    QList<int> columnRoleList() const;

    int selectedIndexCount() const;

    using DListView::edit;

public slots:
    void cd(const FMEvent &event);
    void edit(const FMEvent &event);
    void switchToListMode();
    void switchToIconMode();
    void sort(int windowId, int role);

signals:
    void currentUrlChanged(QString url);

private slots:
    void moveColumnRole(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void onChildrenChanged(const FMEvent &event, const QList<AbstractFileInfo*> &list);

protected:
    void contextMenuEvent(QContextMenuEvent * event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent * event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected) Q_DECL_OVERRIDE;
    void commitData(QWidget * editor) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

private:
    FileController *m_controller;
    FileMenuManager* m_fileMenuManager;
    QHeaderView *m_headerView = Q_NULLPTR;

    QList<int> m_logicalIndexs;
    QList<int> m_columnRoles;
    QList<int> m_iconSizes;

    int m_currentIconSizeIndex = 0;

    bool m_ctrlIsPressed = false;

    QModelIndexList m_selectedIndexList;

    bool isEmptyArea(const QPoint &pos) const;

    QSize currentIconSize() const;
    void enlargeIcon();
    void shrinkIcon();
    void openIndex(const QModelIndex &index);
    void onMenuActionTrigger(const DAction *action, const QModelIndex &index);
};

#endif // DFILEVIEW_H
