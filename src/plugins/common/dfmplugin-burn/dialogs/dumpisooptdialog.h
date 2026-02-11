// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DUMPISOOPTDIALOG_H
#define DUMPISOOPTDIALOG_H

#include "dfmplugin_burn_global.h"

#include <DDialog>
#include <DFileChooserEdit>
#include <QPushButton>

namespace dfmplugin_burn {

class DumpISOOptDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit DumpISOOptDialog(const QString &devId, QWidget *parent = nullptr);
    ~DumpISOOptDialog();

private:
    void initliazeUi();
    void initConnect();
    void initData();

private Q_SLOTS:
    void onButtonClicked(int index, const QString &text);
    void onFileChoosed(const QString &fileName);
    void onPathChanged(const QString &path);

private:
    QString curDevId;
    QString curDev;
    QString curDiscName;
    QAbstractButton *createImgBtn { nullptr };
    QWidget *contentWidget { nullptr };
    QLabel *saveAsImgLabel { nullptr };
    QLabel *commentLabel { nullptr };
    QLabel *savePathLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DFileChooserEdit *fileChooser { nullptr };
    QAbstractButton *filedialogBtn { nullptr };
};

}   // namespace dfmplugin_burn

#endif   // DUMPISOOPTDIALOG_H
