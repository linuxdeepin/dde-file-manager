// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DUMPISOOPTDIALOG_H
#define DUMPISOOPTDIALOG_H

#include "durl.h"

#include <DDialog>
#include <DFileChooserEdit>
#include <QPushButton>

class DumpISOOptDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT

public:
    explicit DumpISOOptDialog(const QString &dev, const QString &discName, QWidget *parent = nullptr);
    void setJobWindowId(quint64 wid);

private:
    void initliazeUi();
    void initConnect();

private slots:
    void onButtonClicked(int index, const QString &text);
    void onFileChoosed(const QString &fileName);
    void onPathChanged(const QString &path);

private:
    quint64 m_winId = 0;
    QString m_curDevice;
    QString m_curDiscName;
    QAbstractButton *m_createImgBtn = nullptr;
    QWidget *m_contentWidget = nullptr;
    QLabel *m_saveAsImgLabel = nullptr;
    QLabel *m_commentLabel = nullptr;
    QLabel *m_savePathLabel = nullptr;
    Dtk::Widget::DFileChooserEdit *m_fileChooser = nullptr;
    QAbstractButton *m_filedialogBtn = nullptr;
};

#endif // DUMPISOOPTDIALOG_H
