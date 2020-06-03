#ifndef DFMVAULTSETUNLOCKMETHODVIEW_H
#define DFMVAULTSETUNLOCKMETHODVIEW_H

#include <QWidget>
#include <dtkwidget_global.h>

#define PASSWORD_LENGHT_MAX     24

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;
class OperatorCenter;
class QSlider;
class QComboBox;
class QGridLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultActiveSetUnlockMethodView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveSetUnlockMethodView(QWidget *parent = nullptr);
    void clearText();

signals:
    void sigAccepted();
    // 抛出二维码显示的内容
    void qrcodeContant(const QString &qrcode);

public slots:

private slots:
    void slotIsShowPassword();
    void slotIshowRepeatPassword();
    void slotPasswordEditing();
    void slotPasswordEditFinished();
    void slotRepeatPasswordEditFinished();
    void slotRepeatPasswordEditing();
    void slotGenerateEditChanged(const QString &str);
    // 下一步按钮点击
    void slotNextBtnClicked();
    // 类型切换
    void slotTypeChanged(int index);
    // 随即密码长度改变
//    void slotLengthChanged(int length);
    // 限制密码的长度
    void slotLimiPasswordLength(const QString &password);
    // 随即生成密码按钮点击
//    void slotGeneratePasswordBtnClicked();

private:
    // 校验密码是否符合规则
    bool checkPassword(const QString& password);
    // 校验重复密码框是否符合规则
    bool checkRepeatPassword();
    // 校验界面输入信息是否符合规则
    bool checkInputInfo();

private:
    QComboBox           *m_pTypeCombo;

    QLabel              *m_pPasswordLabel;
    DPasswordEdit       *m_pPassword;

//    QLabel              *m_pLengthLabel;
//    QSlider             *m_pLengthSlider;
//    QLineEdit           *m_pLengthEdit;
//    QLabel              *m_pStrengthLabel;

    QLabel              *m_pRepeatPasswordLabel;
    DPasswordEdit       *m_pRepeatPassword;

//    QLabel              *m_pResultLabel;
//    QLineEdit           *m_pResultEdit;

    QLabel              *m_pPasswordHintLabel;
    QLineEdit           *m_pTips;

    QCheckBox           *m_pOtherMethod;

    QPushButton         *m_pGenerateBtn;

    QPushButton         *m_pNext;

    QGridLayout         *play;
};

#endif // DFMVAULTSETUNLOCKMETHODVIEW_H
