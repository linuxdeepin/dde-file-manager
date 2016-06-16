#ifndef MOUNTASKPASSWORDDIALOG_H
#define MOUNTASKPASSWORDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>

class MountAskPasswordDialog : public QDialog
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

private:
    QLabel* m_messageLabel;

    QFrame* m_passwordFrame;

    QButtonGroup* m_anonymousButtonGroup;

    QLineEdit* m_usernameLineEdit;
    QLineEdit* m_domainLineEdit;
    QLineEdit* m_passwordLineEdit;

    QButtonGroup* m_passwordButtonGroup;

    QPushButton* m_cancelButton;
    QPushButton* m_connectButton;

    QJsonObject m_loginObj;
};

#endif // MOUNTASKPASSWORDDIALOG_H
