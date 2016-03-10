#ifndef DTOOLBAR_H
#define DTOOLBAR_H

#include <QFrame>
#include <QStackedWidget>
#include "dstatebutton.h"

class DIconTextButton;
class DStateButton;
class DSearchBar;
class DTabBar;

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

    DStateButton::ButtonState getLayoutbuttonState();

signals:
    void requestSwitchLayout();
    void switchLayoutMode();
    void backButtonClicked();
    void refreshButtonClicked();

public slots:
    void setLayoutButtonState(DStateButton::ButtonState state);
    void searchBarSwitched();
    void searchBarActivated();
    void searchBarDeactivated();
    void searchBarTextEntered();
    void tabBarClicked(int index);
    void upButtonClicked();
    void searchBarChanged(QString path);
    void tabBarChanged(QString path);
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
    DStateButton* m_layoutButton=NULL;
    DStateButton* m_sortButton=NULL;
    DStateButton* m_hideShowButton=NULL;
    DStateButton* m_viewSwitchButton=NULL;
    DSearchBar * m_searchBar = NULL;
    DTabBar * m_tabBar = NULL;
    bool m_switchState = false;
};

#endif // DTOOLBAR_H
