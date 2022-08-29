/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef PERMISSIONMANAGERWIDGET_H
#define PERMISSIONMANAGERWIDGET_H

#include "dfmplugin_propertydialog_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <DArrowLineDrawer>

#include <QUrl>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

class QComboBox;
class QCheckBox;

namespace dfmplugin_propertydialog {

class PermissionManagerWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit PermissionManagerWidget(QWidget *parent = nullptr);
    ~PermissionManagerWidget();

public:
    void selectFileUrl(const QUrl &url);

    void updateFileUrl(const QUrl &url);

private:
    void initUI();

    QString getPermissionString(int enumFlag);

    void setComboBoxByPermission(QComboBox *cb, int permission, int offset);

    void toggleFileExecutable(bool isChecked);

    bool canChmod(const AbstractFileInfoPointer &info);

    void setExecText();

protected:
    void paintEvent(QPaintEvent *evt) override;

private slots:
    void onComboBoxChanged();

private:
    DTK_WIDGET_NAMESPACE::DLabel *ownerLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *groupLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *otherLabel { nullptr };
    QComboBox *ownerComboBox { nullptr };
    QComboBox *groupComboBox { nullptr };
    QComboBox *otherComboBox { nullptr };
    QCheckBox *executableCheckBox { nullptr };
    QStringList authorityList {};
    QStringList cannotChmodFsType {};
    QUrl selectUrl;

    // these are for file or folder, folder will with executable index.
    int readWriteIndex = 0;
    int readOnlyIndex = 0;
    int readOnlyFlag = 4;
    int readOnlyWithXFlag = 5;
    int readWriteFlag = 6;
    int readWriteWithXFlag = 7;
};
}
#endif   // PERMISSIONMANAGERWIDGET_H
