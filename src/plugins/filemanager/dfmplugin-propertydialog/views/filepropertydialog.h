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
#include "dfm-base/interfaces/extendedcontrolview.h"
#include "basicwidget.h"

#include <dplatformwindowhandle.h>
#include <DDialog>
#include <DCheckBox>

#include <QScrollArea>
#include <QTextEdit>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class BasicWidget;
class NameTextEdit;
class FilePropertyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit FilePropertyDialog(QWidget *parent = nullptr);
    virtual ~FilePropertyDialog() override;

private:
    void initHeadUI();

    void initInfoUI();

    int contentHeight();

public:
    void setSelectFileUrl(const QUrl &url);

public slots:

    void processHeight(int height);

    void insertExtendedControl(int index, DFMBASE_NAMESPACE::ExtendedControlView *widget);

    void addExtendedControl(DFMBASE_NAMESPACE::ExtendedControlView *widget);

    void insertExtendedControl(int index, DFMBASE_NAMESPACE::ExtendedControlDrawerView *widget, bool expansion = false);

    void addExtendedControl(DFMBASE_NAMESPACE::ExtendedControlDrawerView *widget, bool expansion = false);

    void closeDialog();

signals:
    void closed(const QUrl url);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private:
    QScrollArea *scrollArea { nullptr };
    BasicWidget *basicWidget { nullptr };
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
