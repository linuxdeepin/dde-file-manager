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

class QCheckBox;
class QLineEdit;
class QComboBox;

DPDIRSHARE_BEGIN_NAMESPACE

class ShareControlWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
public:
    explicit ShareControlWidget(const QUrl &url, QWidget *parent = nullptr);

protected:
    void setupUi();
    void init();
    void initConnection();
    bool validateShareName();

protected Q_SLOTS:
    void updateShare();
    void shareFolder();
    void unshareFolder();
    void updateWidgetStatus(const QString &filePath);
    void updateFile(const QUrl &oldOne, const QUrl &newOne);

private:
    QCheckBox *shareSwitcher { nullptr };
    QLineEdit *shareNameEditor { nullptr };
    QComboBox *sharePermissionSelector { nullptr };
    QComboBox *shareAnonymousSelector { nullptr };

    QUrl url;
    AbstractFileInfoPointer info { nullptr };
    AbstractFileWatcherPointer watcher { nullptr };
};

DPDIRSHARE_END_NAMESPACE

#endif   // SHARECONTROLWIDGET_H
