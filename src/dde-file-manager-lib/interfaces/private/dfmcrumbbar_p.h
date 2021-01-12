#ifndef DFMCRUMBBAR_P_H
#define DFMCRUMBBAR_P_H

#include <dfmglobal.h>
#include <QPushButton>
#include <DListView>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
DFM_BEGIN_NAMESPACE

class DFMCrumbBar;
class DFMCrumbListviewModel;
class DFMAddressBar;
class DFMCrumbInterface;
class DFMCrumbBarPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbBar)

public:
    explicit DFMCrumbBarPrivate(DFMCrumbBar *qq);

    // UI
    QPushButton leftArrow;
    QPushButton rightArrow;
    DListView crumbListView;
    DFMCrumbListviewModel *crumbListviewModel = nullptr;
    QHBoxLayout *crumbBarLayout;
    QPoint clickedPos;
    DFMAddressBar *addressBar = nullptr;

    // Scheme support
    DFMCrumbInterface *crumbController = nullptr;

    // Misc
    bool clickableAreaEnabled = false;

    DFMCrumbBar *q_ptr = nullptr;

    void clearCrumbs();
    void checkArrowVisiable();
    void updateController(const DUrl &url);
    void setClickableAreaEnabled(bool enabled);

private:
    void initUI();
    void initData();
    void initConnections();
};

DFM_END_NAMESPACE

#endif // DFMCRUMBBAR_P_H
