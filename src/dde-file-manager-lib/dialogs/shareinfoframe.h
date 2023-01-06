// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREINFOFRAME_H
#define SHAREINFOFRAME_H

#include <QFrame>
#include <QTimer>
#include <QPointer>

#include "dabstractfileinfo.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
class QLineEdit;
class QTextBrowser;
class QPushButton;
QT_END_NAMESPACE

class ShareInfoFrame : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool UserSharePwdSettingDialogShown READ getUserSharePwdSetDlgShow WRITE setUserSharePwdSetDlgShow)
public:
    explicit ShareInfoFrame(const DAbstractFileInfoPointer &info, QWidget *parent = 0);
    ~ShareInfoFrame();
    bool getUserSharePwdSetDlgShow() const;
    void setUserSharePwdSetDlgShow(bool value);
    void initUI();
    void initConnect();
    void setFileinfo(const DAbstractFileInfoPointer &fileinfo);
    bool checkShareName();

signals:
    void folderShared(const QString &filePath);
    void unfolderShared();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
public slots:
    void handleCheckBoxChanged(const bool &checked);
    void handleShareNameFinished();
    // 屏蔽NameChanged信号
    void handleShareNameChanged(const QString &str);
    void handlePermissionComboxChanged(const int &index);
    void handleAnonymityComboxChanged(const int &index);
    void handShareInfoChanged();
    bool doShareInfoSetting();
    void updateShareInfo(const QString &filePath);
    void activateWidgets();
    void disactivateWidgets();
private slots:
    void setOrModifySharePassword();
    void showShareInfo(bool value);
    void updatePasswordState();
private:
    QString refreshIp() const;
private:
    DAbstractFileInfoPointer m_fileinfo;
    QCheckBox *m_shareCheckBox = nullptr;
    QLineEdit *m_shareNamelineEdit = nullptr;
    QComboBox *m_permissoComBox = nullptr;
    QComboBox *m_anonymityCombox = nullptr;
    QLabel *m_netScheme = nullptr;
    QLineEdit *m_networkAddrLabel = nullptr;
    QLineEdit *m_userNamelineEdit = nullptr;
    QLineEdit *m_sharePasswordlineEdit = nullptr;
    QPointer<QWidget> m_sharePropertyBkgWidget = nullptr;
    QTextBrowser *m_shareNotes = nullptr;
    QPushButton *splitLineGray = nullptr;
    QPushButton *passwordOperation = nullptr;
    bool m_isSharePasswordSet = false;
    QTimer *m_refreshIp = nullptr;
    //QTimer *m_jobTimer;
    QString m_selfIp;
    bool m_userSharePwdSettingDialogShown = false;
};

#endif // SHAREINFOFRAME_H
