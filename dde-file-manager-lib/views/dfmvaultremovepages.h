#ifndef DFMVAULTREMOVEPAGES_H
#define DFMVAULTREMOVEPAGES_H

#include <dtkwidget_global.h>
#include <DDialog>

class QStackedWidget;

class DFMVaultRemoveProgressView;
class DFMVaultRemoveByRecoverykeyView;
class DFMVaultRemoveByPasswordView;
DWIDGET_USE_NAMESPACE

// 页面类型，Password：密码输入页面， Key：密钥输入界面，Progress删除进度页面
enum class PageType { Password, Key, Progress};
class DFMVaultRemovePages : public DDialog
{
    Q_OBJECT
public:   
    static DFMVaultRemovePages* instance();

public slots:
    void onButtonClicked(int index);

    void onLockVault(int state);

    void onVualtRemoveFinish(bool result);

private:    
    explicit DFMVaultRemovePages(QWidget *parent = nullptr);
    ~DFMVaultRemovePages() override {}

    void initConnect();

    void showEvent(QShowEvent *event) override;
private:
    DFMVaultRemoveByPasswordView *m_passwordView {nullptr};
    DFMVaultRemoveByRecoverykeyView *m_recoverykeyView {nullptr};
    DFMVaultRemoveProgressView *m_progressView {nullptr};

    QStackedWidget * m_stackedWidget {nullptr};   //用于页面切换
    bool m_bRemoveVault = false;
};

#endif // DFMVAULTREMOVEPAGES_H
