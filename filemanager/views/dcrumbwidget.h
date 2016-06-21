#ifndef DCRUMBWIDGET_H
#define DCRUMBWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include "deviceinfo/udiskdeviceinfo.h"
#include "durl.h"
#include "dbusinterface/dbustype.h"

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
    void addCrumb(const QStringList &list);
    void setCrumb(const DUrl &path);
    void clear();
    QString back();
    QString path();
    DUrl getUrl();
private:
    void addRecentCrumb();
    void addComputerCrumb();
    void addTrashCrumb();
    void addHomeCrumb();
    void addNetworkCrumb();

    DCrumbButton* createDeviceCrumbButtonByType(UDiskDeviceInfo::MediaType type, const QString& mountPoint);

    void addLocalCrumbs(const DUrl & url);
    void initUI();
    void prepareCrumbs(const DUrl &path);
    bool hasPath(const QString& path);
    bool isInHome(const QString& path);
    bool isHomeFolder(const QString& path);
    bool isInDevice(const QString& path);
    bool isDeviceFolder(const QString& path);
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
