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

#ifndef FILEPROPERTYVIEW_H
#define FILEPROPERTYVIEW_H

#include "dfmplugin_propertydialog_global.h"
#include "dfm_common_service_global.h"
#include "editstackedwidget.h"

#include <DDialog>
#include <DCheckBox>
#include <DPlatformWindowHandle>

#include <QScrollArea>
#include <QTextEdit>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class BasicWidget;
class PermissionManagerWidget;
class NameTextEdit;
class FilePropertyDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit FilePropertyDialog(QWidget *parent = nullptr);
    virtual ~FilePropertyDialog() override;

private:
    void initInfoUI();

    void createHeadUI(const QUrl &url, int widgetFilter);

    void createBasicWidget(const QUrl &url, int widgetFilter);

    void createPermissionManagerWidget(const QUrl &url, int widgetFilter);

    int contentHeight();

public:
    void selectFileUrl(const QUrl &url, int widgetFilter);

    qint64 getFileSize();

    int getFileCount();

public slots:

    void processHeight(int height);

    void insertExtendedControl(int index, QWidget *widget);

    void addExtendedControl(QWidget *widget);

    void closeDialog();

signals:
    void closed(const QUrl url);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    QScrollArea *scrollArea { nullptr };
    BasicWidget *basicWidget { nullptr };
    PermissionManagerWidget *permissionManagerWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *fileIcon { nullptr };
    EditStackedWidget *editStackWidget { nullptr };
    QFrame *textShowFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *editButton { nullptr };
    QList<QWidget *> extendedControl {};
    QUrl currentFileUrl {};
    int extendedHeight { 0 };
    DTK_WIDGET_NAMESPACE::DPlatformWindowHandle *platformWindowHandle { nullptr };
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // FILEPROPERTYVIEW_H
