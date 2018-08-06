/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "popupcontrolwidget.h"

#include <QVBoxLayout>
#include <QProcess>
#include <QDebug>
#include <QDir>

#include <ddialog.h>
#include <DTrashManager>
#include <DDesktopServices>

#include <QCoreApplication>

DWIDGET_USE_NAMESPACE

const QString TrashDir = QDir::homePath() + "/.local/share/Trash";


PopupControlWidget::PopupControlWidget(QWidget *parent)
    : QWidget(parent),

      m_empty(false),

      m_fsWatcher(new QFileSystemWatcher(this))
{
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &PopupControlWidget::trashStatusChanged, Qt::QueuedConnection);

    setObjectName("trash");
    setFixedWidth(80);

    trashStatusChanged();
}

bool PopupControlWidget::empty() const
{
    return m_empty;
}

int PopupControlWidget::trashItems() const
{
    return m_trashItemsCount;
}

QSize PopupControlWidget::sizeHint() const
{
    return QSize(width(), m_empty ? 30 : 60);
}

const QString PopupControlWidget::trashDir()
{
    return TrashDir;
}

void PopupControlWidget::openTrashFloder()
{
    QProcess *proc = new QProcess;

    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), proc, &QProcess::deleteLater);

    proc->startDetached("gvfs-open trash:///");
}

void PopupControlWidget::clearTrashFloder()
{
    QString ClearTrashMutliple = qApp->translate("DialogManager", "Are you sure to empty %1 items?");

    // show confrim dialog
    DDialog d;
    QStringList buttonTexts;
    buttonTexts << qApp->translate("DialogManager", "Cancel") << qApp->translate("DialogManager", "Delete");

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    QDir dir(QDir::homePath() + "/.local/share/Trash/files");
    uint count = dir.count();
    int execCode = -1;

    if (count > 0) {
        d.setTitle(ClearTrashMutliple.arg(count));
        d.setMessage(qApp->translate("DialogManager", "This action cannot be restored"));
        d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
        d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
        d.setDefaultButton(1);
        d.getButton(1)->setFocus();
        d.moveToCenter();
        execCode = d.exec();
    }

    if (execCode != QDialog::Accepted) {
        return;
    }

    if (DTrashManager::instance()->cleanTrash()) {
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_EmptyTrash);
    } else {
        qWarning() << "Clear trash failed";
    }
//    DFMGlobal::instance()->clearTrash();
}

int PopupControlWidget::trashItemCount() const
{
    return QDir(TrashDir + "/info").entryInfoList().count() - 2;
}

void PopupControlWidget::trashStatusChanged()
{
    const bool files = QDir(TrashDir + "/files").exists();
//    const bool info = QDir(TrashDir + "/info").exists();

    // add monitor paths
    m_fsWatcher->addPath(TrashDir);
    if (files)
        m_fsWatcher->addPath(TrashDir + "/files");
//    if (info)
//        m_fsWatcher->addPath(TrashDir + "/info");

    // check empty
    if (!files)
        m_trashItemsCount = 0;
    else
        m_trashItemsCount = QDir(TrashDir + "/files").entryList().count() - 2;

    const bool empty = m_trashItemsCount == 0;
    if (m_empty == empty)
        return;

//    m_clearBtn->setVisible(!empty);
    m_empty = empty;

    setFixedHeight(sizeHint().height());

    emit emptyChanged(m_empty);
}
