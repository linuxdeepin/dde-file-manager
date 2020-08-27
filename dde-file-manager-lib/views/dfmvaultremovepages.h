#ifndef DFMVAULTREMOVEPAGES_H
#define DFMVAULTREMOVEPAGES_H

#include "dfmvaultpagebase.h"

class QStackedWidget;

class DFMVaultRemoveProgressView;
class DFMVaultRemoveByRecoverykeyView;
class DFMVaultRemoveByPasswordView;
DWIDGET_BEGIN_NAMESPACE

class DLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

class DFMVaultRemovePages : public DFMVaultPageBase
{
    Q_OBJECT
public:   
    static DFMVaultRemovePages* instance();

    void showTop() override;

public slots:
    void onButtonClicked(int index);

    void onLockVault(int state);

    void onVualtRemoveFinish(bool result);

private:    
    explicit DFMVaultRemovePages(QWidget *parent = nullptr);
    ~DFMVaultRemovePages() override {}

    void initConnect();

    void showVerifyWidget();

    void showRemoveWidget();
    void setInfo(const QString &info);

    void closeEvent(QCloseEvent *event) override;

private:
    DFMVaultRemoveByPasswordView *m_passwordView {nullptr};
    DFMVaultRemoveByRecoverykeyView *m_recoverykeyView {nullptr};
    DFMVaultRemoveProgressView *m_progressView {nullptr};

    QStackedWidget * m_stackedWidget {nullptr};   //用于页面切换
    bool m_bRemoveVault = false;

    QLabel              *m_pInfo;
};

#endif // DFMVAULTREMOVEPAGES_H
