#ifndef DCRUMBWIDGET_H
#define DCRUMBWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include "durl.h"

class FMEvent;
class DCrumbButton;
class DStateButton;
class DCrumbWidget;

class ListWidgetPrivate : public QListWidget
{
public:
    ListWidgetPrivate(DCrumbWidget * crumbWidget);
private:
    DCrumbWidget * m_crumbWidget;
protected:
    void mousePressEvent(QMouseEvent *event);
};

class DCrumbWidget : public QFrame
{
    Q_OBJECT
public:
    explicit DCrumbWidget(QWidget *parent = 0);
    void addCrumb(const QString &text);
    void addCrumb(const QStringList &list);
    void setCrumb(const DUrl &path);
    void clear();
    QString back();
    QString path();
private:
    void addRecentCrumb();
    void addComputerCrumb();
    void addTrashCrumb();
    void addHomeCrumb();
    void addLocalCrumbs(const DUrl & path);
    void initUI();
    void prepareCrumbs(const DUrl &path);
    bool hasPath(QString path);
    bool isInHome(QString path);
    bool isHomeFolder(QString path);
    bool isRootFolder(QString path);
    void createCrumbs();
    void createArrows();
    void checkArrows();
    QHBoxLayout * m_buttonLayout;
    QButtonGroup m_group;
    DUrl m_path;
    QString m_homePath;
    DStateButton * m_leftArrow = NULL;
    DStateButton * m_rightArrow = NULL;
    ListWidgetPrivate * m_listWidget = NULL;
    QList<QPushButton *> m_buttons;
    bool m_needArrows = false;
    int m_prevCheckedId = -1;
    QList<QListWidgetItem*> m_items;
    int m_crumbTotalLen = 0;
public slots:
    void buttonPressed();
    void crumbModified();
    void crumbMoveToLeft();
    void crumbMoveToRight();
signals:
    void crumbSelected(const FMEvent &event);
    void searchBarActivated();

protected:
    void resizeEvent(QResizeEvent *e);
};

#endif // DCRUMBWIDGET_H
