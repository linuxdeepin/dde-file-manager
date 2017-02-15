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
#include <dlineedit.h>
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

    bool getDomainLineVisible() const;
    void setDomainLineVisible(bool domainLineVisible);

signals:

public slots:
    void setLoginData(const QJsonObject& obj);
    void handleConnect();
    void togglePasswordFrame(int id);
    void handleButtonClicked(int index, QString text);

private:
    QLabel* m_messageLabel;
    QLabel* m_domainLabel;

    QFrame* m_passwordFrame;

    QButtonGroup* m_anonymousButtonGroup;

    DLineEdit* m_usernameLineEdit;
    DLineEdit* m_domainLineEdit;
    DPasswordEdit* m_passwordLineEdit;

    QCheckBox* m_passwordCheckBox;

    QButtonGroup* m_passwordButtonGroup;


    QJsonObject m_loginObj;

    bool m_domainLineVisible = true;
};

#endif // MOUNTASKPASSWORDDIALOG_H
