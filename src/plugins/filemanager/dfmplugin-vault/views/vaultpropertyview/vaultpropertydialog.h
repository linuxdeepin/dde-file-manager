// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTPROPERTYDIALOG_H
#define VAULTPROPERTYDIALOG_H

#include "dfmplugin_vault_global.h"
#include "basicwidget.h"

#include <DDialog>
#include <DPlatformWindowHandle>

#include <QScrollArea>

namespace dfmplugin_vault {
class VaultPropertyDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit VaultPropertyDialog(QWidget *parent = nullptr);
    virtual ~VaultPropertyDialog() override;

private:
    void initInfoUI();

    void createHeadUI(const QUrl &url);

    void createBasicWidget(const QUrl &url);

    int contentHeight();

    void insertExtendedControl(int index, QWidget *widget);

    void addExtendedControl(QWidget *widget);

private slots:
    void processHeight(int height);

public:
    void selectFileUrl(const QUrl &url);

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    BasicWidget *basicWidget { nullptr };
    QLabel *fileIconLabel { nullptr };
    QLabel *fileNameLabel { nullptr };
    QScrollArea *scrollArea { nullptr };
    QList<QWidget *> extendedControl {};
    int extendedHeight { 0 };
    DTK_WIDGET_NAMESPACE::DPlatformWindowHandle *platformWindowHandle { nullptr };
};
}
#endif   // VAULTPROPERTYDIALOG_H
