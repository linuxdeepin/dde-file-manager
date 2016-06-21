#ifndef MOUNTASKPASSWORDDIALOG_H
#define MOUNTASKPASSWORDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QJsonObject>
#include <ddialog.h>
#include <dpasswordedit.h>
#include <dcheckbox.h>

DWIDGET_USE_NAMESPACE

class MountAskPasswordDialog : public DDialog
{
    Q_OBJECT
public:
    explicit MountAskPasswordDialog(QWidget *parent = 0);
    ~MountAskPasswordDialog();

    void initUI();
    void initConnect();
    QJsonObject getLoginData();

signals:

public slots:
    void setLoginData(const QJsonObject& obj);
    void handleConnect();
    void togglePasswordFrame(int id);
    void handleButtonClicked(int index, QString text);

private:
    QLabel* m_messageLabel;

    QFrame* m_passwordFrame;

    QButtonGroup* m_anonymousButtonGroup;

    QLineEdit* m_usernameLineEdit;
    QLineEdit* m_domainLineEdit;
    DPasswordEdit* m_passwordLineEdit;

    DCheckBox* m_passwordCheckBox;

    QButtonGroup* m_passwordButtonGroup;


    QJsonObject m_loginObj;
};

#endif // MOUNTASKPASSWORDDIALOG_H
