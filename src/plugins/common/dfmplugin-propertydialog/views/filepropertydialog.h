// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPROPERTYVIEW_H
#define FILEPROPERTYVIEW_H

#include "dfmplugin_propertydialog_global.h"
#include "editstackedwidget.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <DDialog>
#include <DCheckBox>
#include <DPlatformWindowHandle>

#include <QScrollArea>
#include <QTextEdit>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DArrowLineDrawer;
DWIDGET_END_NAMESPACE

namespace dfmplugin_propertydialog {
class BasicWidget;
class PermissionManagerWidget;
class NameTextEdit;
class FilePropertyDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit FilePropertyDialog(QWidget *parent = nullptr);
    virtual ~FilePropertyDialog() override;

public:
    void selectFileUrl(const QUrl &url);
    void filterControlView();
    qint64 getFileSize();
    int getFileCount();
    void setBasicInfoExpand(bool expand);
    int initalHeightOfView();

public slots:
    void processHeight(int height);
    void insertExtendedControl(int index, QWidget *widget);
    void addExtendedControl(QWidget *widget);
    void closeDialog();
    void onSelectUrlRenamed(const QUrl &url);
    void onFileInfoUpdated(const QUrl &url, const QString &infoPtr, const bool isLinkOrg);

signals:
    void closed(const QUrl url);

private:
    void initInfoUI();
    void createHeadUI(const QUrl &url);
    void createBasicWidget(const QUrl &url);
    void createPermissionManagerWidget(const QUrl &url);
    int contentHeight();
    void setFileIcon(QLabel *fileIcon, FileInfoPointer fileInfo);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    bool isShown { false };
    QScrollArea *scrollArea { nullptr };
    BasicWidget *basicWidget { nullptr };
    PermissionManagerWidget *permissionManagerWidget { nullptr };
    QLabel *fileIcon { nullptr };
    EditStackedWidget *editStackWidget { nullptr };
    QFrame *textShowFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *editButton { nullptr };
    QList<QWidget *> extendedControl {};
    QUrl currentFileUrl {};
    int extendedHeight { 0 };
    DTK_WIDGET_NAMESPACE::DPlatformWindowHandle *platformWindowHandle { nullptr };
    FileInfoPointer currentInfo { nullptr };
};
}
#endif   // FILEPROPERTYVIEW_H
