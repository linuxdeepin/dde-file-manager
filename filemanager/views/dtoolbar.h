#ifndef DTOOLBAR_H
#define DTOOLBAR_H

#include <QFrame>
#include "dstatebutton.h"

class DIconTextButton;
class DStateButton;

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

public slots:
    void setLayoutButtonState(DStateButton::ButtonState state);

private:
    QFrame* m_addressToolBar;
    DStateButton* m_backButton=NULL;
    DStateButton* m_forwardButton=NULL;
    DStateButton* m_searchButton = NULL;
    DStateButton* m_refreshButton = NULL;
    QFrame* m_contollerToolBar;
    DIconTextButton* m_newFolderButton=NULL;
    DIconTextButton* m_deleteFileButton=NULL;
    DStateButton* m_layoutButton=NULL;
    DStateButton* m_sortButton=NULL;
    DStateButton* m_hideShowButton=NULL;
    DStateButton* m_viewSwitchButton=NULL;
};

#endif // DTOOLBAR_H
