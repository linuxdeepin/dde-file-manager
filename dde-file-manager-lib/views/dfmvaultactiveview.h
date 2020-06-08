#ifndef DFMVAULTACTIVEVIEW_H
#define DFMVAULTACTIVEVIEW_H

#include <DDialog>

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QMouseEvent;
QT_END_NAMESPACE

class DFMVaultActiveStartView;
class DFMVaultActiveSetUnlockMethodView;
class DFMVaultActiveSaveKeyView;
class DFMVaultActiveFinishedView;

DWIDGET_USE_NAMESPACE

class DFMVaultActiveView  : public DDialog
{
    Q_OBJECT
public:
    static DFMVaultActiveView &getInstance(){
        static DFMVaultActiveView dlg;
        return dlg;
    }

    void setWndPtr(QWidget *wnd);
    QWidget *getWndPtr() const;

    // 置顶显示
    void showTop();

protected:
    void closeEvent(QCloseEvent *) override;

private slots:
    void slotNextWidget();

private:
    DFMVaultActiveView(QWidget *parent = nullptr);

    // 复原操作
    void setBeginingState();

private:
    // 窗口容器
    QStackedWidget                          *m_pStackedWidget;

    DFMVaultActiveStartView                 *m_pStartVaultWidget;
    DFMVaultActiveSetUnlockMethodView       *m_pSetUnclockMethodWidget;
    DFMVaultActiveSaveKeyView               *m_SaveKeyWidget;
    DFMVaultActiveFinishedView              *m_ActiveVaultFinishedWidget;

    QWidget *m_wndptr = nullptr;
};

#endif // DFMVAULTACTIVEVIEW_H
