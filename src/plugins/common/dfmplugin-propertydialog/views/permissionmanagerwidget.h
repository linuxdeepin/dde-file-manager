// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERMISSIONMANAGERWIDGET_H
#define PERMISSIONMANAGERWIDGET_H

#include "dfmplugin_propertydialog_global.h"
#include <dfm-base/interfaces/fileinfo.h>

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
    ~PermissionManagerWidget() override;

public:
    void selectFileUrl(const QUrl &url);

    void updateFileUrl(const QUrl &url);

private:
    void initUI();

    void updateBackgroundColor();

    QString getPermissionString(int enumFlag);

    void setComboBoxByPermission(QComboBox *cb, int permission, int offset);

    void toggleFileExecutable(bool isChecked);

    bool canChmod(const FileInfoPointer &info);

    void setExecText();

protected:
    void paintEvent(QPaintEvent *evt) override;

private slots:
    void onComboBoxChanged();

private:
    DTK_WIDGET_NAMESPACE::DLabel *ownerLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *groupLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *otherLabel { Q_NULLPTR };
    QComboBox *ownerComboBox { Q_NULLPTR };
    QComboBox *groupComboBox { Q_NULLPTR };
    QComboBox *otherComboBox { Q_NULLPTR };
    QFrame *executableFrame { Q_NULLPTR };
    QCheckBox *executableCheckBox { Q_NULLPTR };
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
