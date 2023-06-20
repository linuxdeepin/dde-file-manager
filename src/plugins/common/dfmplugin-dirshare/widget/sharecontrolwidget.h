// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECONTROLWIDGET_H
#define SHARECONTROLWIDGET_H

#include "dfmplugin_dirshare_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <DLabel>
#include <DCommandLinkButton>
#include <DArrowLineDrawer>
#include <QUrl>
#include <QPointer>

class QCheckBox;
class QLineEdit;
class QComboBox;
class QLabel;
class QTextBrowser;
class QPushButton;
class QFormLayout;
class QGridLayout;

namespace dfmplugin_dirshare {

class ShareControlWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit ShareControlWidget(const QUrl &url, bool disableState = false, QWidget *parent = nullptr);
    virtual ~ShareControlWidget() override;
    static void setOption(QWidget *w, const QVariantHash &option);

protected:
    void setupUi(bool disableState = false);
    void setupShareSwitcher();
    void setupNetworkPath();
    void setupUserName();
    void setupSharePassword();
    void setupShareNotes(QGridLayout *gridLayout);
    void init();
    void initConnection();
    bool validateShareName();

protected Q_SLOTS:
    void updateShare();
    void shareFolder();
    void unshareFolder();
    void updateWidgetStatus(const QString &filePath);
    void updateFile(const QUrl &oldOne, const QUrl &newOne);
    void onSambaPasswordSet(bool result);

private:
    void showMoreInfo(bool showMore);
    void userShareOperation(bool checked);
    void showSharePasswordSettingsDialog();

private:
    QFormLayout *mainLay { nullptr };
    QCheckBox *shareSwitcher { nullptr };
    QLineEdit *shareNameEditor { nullptr };
    QComboBox *sharePermissionSelector { nullptr };
    QComboBox *shareAnonymousSelector { nullptr };

    // network area
    QLabel *netScheme { nullptr };
    QLabel *networkAddrLabel { nullptr };
    QLabel *userNamelineLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *sharePassword { nullptr };
    QTextBrowser *m_shareNotes { nullptr };
    QPushButton *splitLineGray { nullptr };
    QPushButton *copyNetAddr { nullptr };
    QPushButton *copyUserNameBt { nullptr };
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *setPasswordBt { nullptr };
    bool isSharePasswordSet { false };
    QTimer *refreshIp { nullptr };

    //QTimer *m_jobTimer;
    QString selfIp;

    QTimer *timer { nullptr };

    QUrl url;
    FileInfoPointer info { nullptr };
    AbstractFileWatcherPointer watcher { nullptr };
};
}

#endif   // SHARECONTROLWIDGET_H
