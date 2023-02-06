/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SHARECONTROLWIDGET_H
#define SHARECONTROLWIDGET_H

#include "dfmplugin_dirshare_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

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
    QLineEdit *sharePasswordlineEditor { nullptr };
    QTextBrowser *m_shareNotes { nullptr };
    QPushButton *splitLineGray { nullptr };
    QPushButton *copyNetAddr { nullptr };
    QPushButton *copyUserNameBt { nullptr };
    QPushButton *setPasswordBt { nullptr };
    bool isSharePasswordSet { false };
    QTimer *refreshIp { nullptr };

    //QTimer *m_jobTimer;
    QString selfIp;

    QTimer *timer { nullptr };

    QUrl url;
    AbstractFileInfoPointer info { nullptr };
    AbstractFileWatcherPointer watcher { nullptr };
};
}

#endif   // SHARECONTROLWIDGET_H
