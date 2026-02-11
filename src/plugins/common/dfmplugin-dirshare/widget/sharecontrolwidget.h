// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECONTROLWIDGET_H
#define SHARECONTROLWIDGET_H

#include "dfmplugin_dirshare_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <DLabel>
#include <DTipLabel>
#include <DLineEdit>
#include <DCommandLinkButton>
#include <DArrowLineDrawer>
#include <QUrl>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLineEdit;
class QComboBox;
class QLabel;
class QTextBrowser;
class QPushButton;
class QFormLayout;
class QGridLayout;
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

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
    void setupShareNameEditor();
    void setupSharePermissionSelector();
    void setupShareAnonymousSelector();
    QHBoxLayout *setupNetworkPath();
    QHBoxLayout *setupUserName();
    QHBoxLayout *setupSharePassword();
    void setupShareNotes();
    void init();
    void initConnection();
    bool validateShareName();

protected Q_SLOTS:
    void updateShare();
    bool shareFolder();
    bool unshareFolder();
    void updateWidgetStatus(const QString &filePath);
    void updateFile(const QUrl &oldOne, const QUrl &newOne);
    void onSambaPasswordSet(bool result);
    void onShareNameChanged(const QString &name);

private:
    void showMoreInfo(bool showMore);
    void userShareOperation(bool checked);
    void showSharePasswordSettingsDialog();

private:
    QVBoxLayout *mainLayout { Q_NULLPTR };
    QFrame *moreInfoFrame { Q_NULLPTR };
    QCheckBox *shareSwitcher { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLineEdit *shareNameEditor { Q_NULLPTR };
    QComboBox *sharePermissionSelector { Q_NULLPTR };
    QComboBox *shareAnonymousSelector { Q_NULLPTR };

    // network area
    QLabel *netScheme { Q_NULLPTR };
    QLabel *networkAddrLabel { Q_NULLPTR };
    QLabel *userNamelineLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *sharePassword { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DTipLabel *m_shareNotes { Q_NULLPTR };
    QPushButton *splitLineGray { Q_NULLPTR };
    QPushButton *copyNetAddr { Q_NULLPTR };
    QPushButton *copyUserNameBt { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *setPasswordBt { Q_NULLPTR };
    bool isSharePasswordSet { false };
    QTimer *refreshIp { Q_NULLPTR };

    // QTimer *m_jobTimer;
    QString selfIp;

    QTimer *timer { Q_NULLPTR };

    QUrl url;
    FileInfoPointer info { Q_NULLPTR };
    AbstractFileWatcherPointer watcher { Q_NULLPTR };
};
}

#endif   // SHARECONTROLWIDGET_H
