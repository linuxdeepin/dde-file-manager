#ifndef DFMVAULTACTIVESAVEKEYVIEW_H
#define DFMVAULTACTIVESAVEKEYVIEW_H

#include <QWidget>
#include "dtkwidget_global.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QGridLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DButtonBoxButton;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

class DFMVaultActiveSaveKeyView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveSaveKeyView(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

public slots:

private slots:
    void slotNextBtnClicked();
    void slotKeyBtnClicked();
    void slotQRCodeBtnClicked();

signals:
    void sigAccepted();

private:

private:
    QLabel              *m_pTipsLabel;
    DButtonBoxButton    *m_pKeyBtn;
    QTextEdit           *m_pKeyText;
    DButtonBoxButton    *m_pQRCodeBtn;
    QLabel              *m_pQRCodeImage;
    QLabel              *m_pScanTipsLabel;
    QPushButton         *m_pNext;
    QGridLayout         *play1;
};

#endif // DFMVAULTACTIVESAVEKEYVIEW_H
