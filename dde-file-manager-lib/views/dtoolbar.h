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

QT_BEGIN_NAMESPACE
class QHBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DGraphicsClipEffect;
DWIDGET_END_NAMESPACE

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

    void setCustomActionList(const QList<QAction*> &list);

signals:
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

    void back();
    void forward();

    void handleHotkeyCtrlF(quint64 winId);
    void handleHotkeyCtrlL(quint64 winId);

    void moveNavStacks(int from, int to);
    void removeNavStackAt(int index);
    void switchHistoryStack(const int index );

private:
    void checkNavHistory(DUrl url);
    void onBackButtonClicked();
    void onForwardButtonClicked();

    bool m_searchState = false;
    QFrame* m_addressToolBar;
    QPushButton* m_backButton=NULL;
    QPushButton* m_forwardButton=NULL;
    DStateButton* m_upButton=NULL;
    QPushButton* m_searchButton = NULL;
    DStateButton* m_refreshButton = NULL;
    DSearchBar * m_searchBar = NULL;
    QFrame* m_contollerToolBar;
    DGraphicsClipEffect *m_contollerToolBarClipMask;
    QHBoxLayout *m_contollerToolBarContentLayout;
    DIconTextButton* m_newFolderButton=NULL;
    DIconTextButton* m_deleteFileButton=NULL;

    QPushButton* m_iconViewButton=NULL;
    QPushButton* m_listViewButton=NULL;
    QPushButton* m_extendButton = NULL;
    QPushButton* m_settingsButton = NULL;
    DHoverButton* m_sortingButton=NULL;

    bool m_switchState = false;
    DCrumbWidget * m_crumbWidget = NULL;
    HistoryStack * m_navStack = NULL;
    QList<HistoryStack*> m_navStacks;

};

#endif // DTOOLBAR_H
