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
    DFMVaultActiveView(QWidget *parent = nullptr);

private slots:
    void slotNextWidget();

private:
    // 窗口容器
    QStackedWidget                          *m_pStackedWidget;

    DFMVaultActiveStartView                 *m_pStartVaultWidget;
    DFMVaultActiveSetUnlockMethodView       *m_pSetUnclockMethodWidget;
    DFMVaultActiveSaveKeyView               *m_SaveKeyWidget;
    DFMVaultActiveFinishedView              *m_ActiveVaultFinishedWidget;
};

#endif // DFMVAULTACTIVEVIEW_H
