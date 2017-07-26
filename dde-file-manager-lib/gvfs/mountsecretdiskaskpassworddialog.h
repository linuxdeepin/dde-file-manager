#ifndef MOUNTSECRETDISKASKPASSWORDDIALOG_H
#define MOUNTSECRETDISKASKPASSWORDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <ddialog.h>
#include <dpasswordedit.h>
#include <dpasswordedit.h>

DWIDGET_USE_NAMESPACE



class MountSecretDiskAskPasswordDialog : public DDialog
{
    Q_OBJECT
public:
    explicit MountSecretDiskAskPasswordDialog(QWidget *parent = 0);
    ~MountSecretDiskAskPasswordDialog();

    void initUI();
    void initConnect();

    QString password() const;

    int passwordSaveMode() const;

public slots:
    void handleRadioButtonClicked(int index);
    void handleButtonClicked(int index, QString text);

protected:
    void showEvent(QShowEvent* event);

private:
    QLabel* m_titleLabel = NULL;
    QLabel* m_descriptionLabel = NULL;
    QLabel* m_passwordLabel = NULL;
    DPasswordEdit* m_passwordLineEdit = NULL;

    QRadioButton* m_neverRadioCheckBox = NULL;
    QRadioButton* m_sessionRadioCheckBox = NULL;
    QRadioButton* m_forerverRadioCheckBox = NULL;

    QButtonGroup* m_passwordButtonGroup = NULL;


    QString m_password = "";
    int m_passwordSaveMode = 0;
};

#endif // MOUNTSECRETDISKASKPASSWORDDIALOG_H
