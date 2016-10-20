#ifndef USERSHAREPASSWORDSETTINGDIALOG_H
#define USERSHAREPASSWORDSETTINGDIALOG_H

#include <QObject>
#include <ddialog.h>
#include <dpasswordedit.h>

DWIDGET_USE_NAMESPACE

class UserSharePasswordSettingDialog : public DDialog
{
    Q_OBJECT
public:
    explicit UserSharePasswordSettingDialog(QWidget *parent = 0);
    void initUI();

signals:

public slots:
    void onButtonClicked(const int& index);

private:
    DPasswordEdit *m_passwordEdit;
    QWidget *m_content;
};

#endif // USERSHAREPASSWORDSETTINGDIALOG_H
