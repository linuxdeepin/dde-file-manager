#ifndef DFILEVIEW_H
#define DFILEVIEW_H

#include <dlistview.h>

#include <QFrame>
#include <QUrl>
#include <QContextMenuEvent>

class QFileSystemModel;
class QHeaderView;
class FileController;
class FileMenuManager;

DWIDGET_USE_NAMESPACE

class DFileSystemModel;
class DFileItemDelegate;

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

    bool isIconViewMode();

    int columnWidth(int column) const;
    int columnCount() const;

    QList<int> columnRoleList() const;

public slots:
    void cd(const QString &url);
    void switchListMode();

signals:
    void currentUrlChanged(QString url);

private slots:
    void moveColumnRole(int logicalIndex, int oldVisualIndex, int newVisualIndex);

protected:
    void contextMenuEvent(QContextMenuEvent * event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent * event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    FileController *m_controller;
    FileMenuManager* m_fileMenuManager;
    QHeaderView *m_headerView = Q_NULLPTR;

    QList<int> logicalIndexs;
    QList<int> columnRoles;

    bool ctrlIsPressed = false;
};

#endif // DFILEVIEW_H
