#ifndef DTOOLBAR_H
#define DTOOLBAR_H

#include <QFrame>
#include <QStackedWidget>
#include "dfileview.h"
#include "dstatebutton.h"

class DIconTextButton;
class DCheckableButton;
class DStateButton;
class DSearchBar;
class DTabBar;
class DCrumbWidget;
class DFMEvent;
class HistoryStack;
class DHoverButton;


class DToolBar : public QFrame
{
    Q_OBJECT
public:
    explicit DToolBar(QWidget *parent = 0);
    ~DToolBar();
    static const int ButtonWidth;
    static const int ButtonHeight;
    void initData();
    void initUI();
    void initAddressToolBar();
    void initContollerToolBar();
    void initConnect();
    DSearchBar * getSearchBar();
    DCrumbWidget* getCrumWidget();
    QPushButton* getSettingsButton();
    void addHistoryStack();

    int navStackCount() const;
    void setCrumb(const DUrl& url);
    void updateBackForwardButtonsState();
    void setListModeButtonEnabled(const bool& flag);
    void setIconModeButtonEnabled(const bool& flag);

signals:
    void requestIconView();
    void requestListView();
    void requestExtendView();

    void requestSwitchLayout();
    void refreshButtonClicked();

public slots:
    void searchBarClicked();
    void searchBarActivated();
    void searchBarDeactivated();
    void searchBarTextEntered();
    void crumbSelected(const DFMEvent &e);
    void crumbChanged(const DFMEvent &event);
    void searchBarChanged(QString path);
    void backButtonClicked();
    void forwardButtonClicked();
    void checkViewModeButton(DFileView::ViewMode mode);

    void handleHotkeyBack(quint64 winId);
    void handleHotkeyForward(quint64 winId);
    void handleHotkeyCtrlF(quint64 winId);
    void handleHotkeyCtrlL(quint64 winId);
    void handleChangeIconMode(quint64 winId);
    void handleChangeListMode(quint64 winId);
    void handleChangeExtendMode(quint64 winId);

    void setViewModeButtonVisible(bool isVisible);
    void moveNavStacks(int from, int to);
    void removeNavStackAt(int index);
    void switchHistoryStack(const int index , const DUrl &url);

private:
    void checkNavHistory(DUrl url);
    bool m_searchState = false;
    QFrame* m_addressToolBar;
    QPushButton* m_backButton=NULL;
    QPushButton* m_forwardButton=NULL;
    DStateButton* m_upButton=NULL;
    QPushButton* m_searchButton = NULL;
    DStateButton* m_refreshButton = NULL;
    DSearchBar * m_searchBar = NULL;
    QFrame* m_contollerToolBar;
    DIconTextButton* m_newFolderButton=NULL;
    DIconTextButton* m_deleteFileButton=NULL;

    QPushButton* m_iconViewButton=NULL;
    QPushButton* m_listViewButton=NULL;
    QPushButton* m_extendButton = NULL;
    QPushButton* m_settingsButton = NULL;
    DHoverButton* m_sortingButton=NULL;

    bool m_switchState = false;
    DCrumbWidget * m_crumbWidget = NULL;
    QButtonGroup * m_viewButtonGroup = NULL;
    HistoryStack * m_navStack = NULL;
    QList<HistoryStack*> m_navStacks;

};

#endif // DTOOLBAR_H
