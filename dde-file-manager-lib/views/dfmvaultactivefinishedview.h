#ifndef DFMVAULTACTIVEFINISHEDVIEW_H
#define DFMVAULTACTIVEFINISHEDVIEW_H

#include <QWidget>
#include <dtkwidget_global.h>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QGridLayout;
class QVBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultActiveFinishedView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveFinishedView(QWidget *parent = nullptr);

    void setFinishedBtnEnabled(bool b);

signals:
    void sigAccepted();

public slots:
    // 连接创建保险箱返回信号
    void slotEncryptComplete(int nState);

private slots:
    void slotEncryptVault();
    void slotTimeout();

private: 
    QWidget             *m_pWidget1;
    DLabel              *m_pTips;
    DLabel              *m_pEncryVaultImage;

    QWidget             *m_pWidget2;
    DWaterProgress      *m_pWaterProgress;
    DLabel              *m_pTips3;

    QWidget             *m_pWidget3;
    DLabel              *m_pEncryptFinishedImage;
    DLabel              *m_pTips4;

    QPushButton         *m_pFinishedBtn;

    QVBoxLayout         *m_pLay;

    // 辅助进度条，实现加密过程效果
    QTimer              *m_pTimer;
};

#endif // DFMVAULTACTIVEFINISHEDVIEW_H
