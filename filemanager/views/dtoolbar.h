#ifndef DTOOLBAR_H
#define DTOOLBAR_H

#include <QFrame>
#include <QStackedWidget>
#include "dstatebutton.h"

class DIconTextButton;
class DCheckableButton;
class DStateButton;
class DSearchBar;
class DTabBar;
class DCrumbWidget;

class DToolBar : public QFrame
{
    Q_OBJECT
public:
    explicit DToolBar(QWidget *parent = 0);
    ~DToolBar();
    static const int ButtonHeight;
    void initData();
    void initUI();
    void initAddressToolBar();
    void initContollerToolBar();
    void initConnect();

signals:
    void requestIconView();
    void requestListView();

    void requestSwitchLayout();
    void switchLayoutMode();
    void backButtonClicked();
    void refreshButtonClicked();

public slots:
    void searchBarActivated();
    void searchBarDeactivated();
    void searchBarTextEntered();
    void crumbSelected(QString path);
    void crumbChanged(const QString &url);
    void upButtonClicked();
    void searchBarChanged(QString path);
private:
    QFrame* m_addressToolBar;
    DStateButton* m_backButton=NULL;
    DStateButton* m_forwardButton=NULL;
    DStateButton* m_upButton=NULL;
    DStateButton* m_searchButton = NULL;
    DStateButton* m_refreshButton = NULL;
    QFrame* m_contollerToolBar;
    DIconTextButton* m_newFolderButton=NULL;
    DIconTextButton* m_deleteFileButton=NULL;
    QPushButton* m_iconViewButton=NULL;
    QPushButton* m_listViewButton=NULL;
    DStateButton* m_sortingButton=NULL;
    DSearchBar * m_searchBar = NULL;
    bool m_switchState = false;
    DCrumbWidget * m_crumbWidget = NULL;
    QButtonGroup * m_viewButtonGroup = NULL;
};

#endif // DTOOLBAR_H
