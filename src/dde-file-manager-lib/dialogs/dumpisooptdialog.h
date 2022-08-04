/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
