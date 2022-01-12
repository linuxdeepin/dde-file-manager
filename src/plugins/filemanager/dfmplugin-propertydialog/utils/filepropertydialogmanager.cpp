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
#include "filepropertydialogmanager.h"

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE
FilePropertyDialogManager::FilePropertyDialogManager(QObject *parent)
    : QObject(parent)
{
    closeAllDialog = new CloseAllDialog;
    connect(closeAllDialog, &CloseAllDialog::allClosed, this, &FilePropertyDialogManager::closeAllFilePropertyDialog);
}

FilePropertyDialogManager::~FilePropertyDialogManager()
{
    filePropertyDialogs.clear();

    if(closeAllDialog){
        closeAllDialog->deleteLater();
    }

    if(trashPropertyDialog){
        trashPropertyDialog->deleteLater();
    }

    if(computerPropertyDialog){
        computerPropertyDialog->deleteLater();
    }

    devicePropertyDialogs.clear();
}

void FilePropertyDialogManager::showFilePropertyDialog(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        if (!filePropertyDialogs.contains(url)) {
            FilePropertyDialog *dialog = new FilePropertyDialog();
            dialog->setSelectFileUrl(url);
            filePropertyDialogs.insert(url, dialog);
            createControlView(url);
            dialog->show();
            connect(dialog, &FilePropertyDialog::closed, this, &FilePropertyDialogManager::closeFilePropertyDialog);
        }
        filePropertyDialogs.value(url)->show();
    }

    if (urls.count() >= 2) {
        closeAllDialog->show();
    }
}

/*!
 * \brief           Normal view control extension
 * \param index     Subscript to be inserted
 * \param widget    The view to be inserted
 */
void FilePropertyDialogManager::insertExtendedControlFileProperty(const QUrl &url, int index, ExtendedControlView *widget)
{
    if (widget) {
        FilePropertyDialog *dialog = new FilePropertyDialog();
        dialog->insertExtendedControl(index, widget);
        widget->setSelectFileUrl(url);
    }
}

/*!
 * \brief           Normal view control extension
 * \param widget    The view to be inserted
 */
void FilePropertyDialogManager::addExtendedControlFileProperty(const QUrl &url, ExtendedControlView *widget)
{
    if (widget) {
        FilePropertyDialog *dialog = new FilePropertyDialog();
        dialog->addExtendedControl(widget);
        widget->setSelectFileUrl(url);
    }
}

/*!
 * \brief               Drawer control view extension
 * \param index         Subscript to be inserted
 * \param widget        The view to be inserted
 * \param expansion     Whether to expand
 */
void FilePropertyDialogManager::insertExtendedControlFileProperty(const QUrl &url, int index, ExtendedControlDrawerView *widget, bool expansion)
{
    if (widget) {
        FilePropertyDialog *dialog = new FilePropertyDialog();
        dialog->insertExtendedControl(index, widget, expansion);
        widget->setSelectFileUrl(url);
    }
}

/*!
 * \brief               Drawer control view extension
 * \param widget        The view to be inserted
 * \param expansion     Whether to expand
 */
void FilePropertyDialogManager::addExtendedControlFileProperty(const QUrl &url, ExtendedControlDrawerView *widget, bool expansion)
{
    if (widget) {
        FilePropertyDialog *dialog = new FilePropertyDialog();
        dialog->addExtendedControl(widget, expansion);
        widget->setSelectFileUrl(url);
    }
}

void FilePropertyDialogManager::closeFilePropertyDialog(const QUrl url)
{
    if (filePropertyDialogs.contains(url)) {
        filePropertyDialogs.remove(url);
    }
}

void FilePropertyDialogManager::closeAllFilePropertyDialog()
{
    QList<FilePropertyDialog *> dialogs = filePropertyDialogs.values();
    for (FilePropertyDialog *dialog : dialogs) {
        dialog->close();
    }
}

void FilePropertyDialogManager::createControlView(const QUrl &url)
{
    QMap<int, ExtendedControlView *> controlView = createView<ExtendedControlView>(url);
    int count = controlView.keys().count();
    for (int i = 0; i < count; ++i) {
        ExtendedControlView *view = controlView.value(controlView.keys()[i]);
        if (controlView.keys()[i] == -1) {
            addExtendedControlFileProperty(url, view);
        } else {
            insertExtendedControlFileProperty(url, controlView.keys()[i], view);
        }
    }

    QMap<int, ExtendedControlDrawerView *> controlDrawerView = createView<ExtendedControlDrawerView>(url);
    count = controlView.keys().count();
    for (int i = 0; i < count; ++i) {
        ExtendedControlDrawerView *view = controlDrawerView.value(controlView.keys()[i]);
        if (controlView.keys()[i] == -1) {
            addExtendedControlFileProperty(url, view);
        } else {
            insertExtendedControlFileProperty(url, controlView.keys()[i], view);
        }
    }
}

void FilePropertyDialogManager::showTrashPropertyDialog()
{
    if (trashPropertyDialog == nullptr) {
        trashPropertyDialog = new TrashPropertyDialog;
        trashPropertyDialog->show();
    }
    trashPropertyDialog->show();
    trashPropertyDialog->activateWindow();
}

void FilePropertyDialogManager::showComputerPropertyDialog()
{
    if (computerPropertyDialog == nullptr) {
        computerPropertyDialog = new ComputerPropertyDialog;
        computerPropertyDialog->show();
    }
    computerPropertyDialog->show();
    computerPropertyDialog->activateWindow();
}

void FilePropertyDialogManager::showDevicePropertyDialog(const DeviceInfo &info)
{
    if (devicePropertyDialogs.contains(info.deviceUrl)) {
        devicePropertyDialogs.value(info.deviceUrl)->show();
        devicePropertyDialogs.value(info.deviceUrl)->activateWindow();
    } else {
        DevicePropertyDialog *devicePropertyDialog = new DevicePropertyDialog;
        devicePropertyDialog->show();
        devicePropertyDialog->setSelectDeviceInfo(info);
        devicePropertyDialogs.insert(info.deviceUrl, devicePropertyDialog);
    }
}

void FilePropertyDialogManager::insertExtendedControlDeviceProperty(const QUrl &url, int index, ExtendedControlView *widget)
{
    if (devicePropertyDialogs.contains(url) && widget) {
        devicePropertyDialogs.value(url)->insertExtendedControl(index, widget);
    } else if (widget) {
        DevicePropertyDialog *dialog = new DevicePropertyDialog();
        devicePropertyDialogs.insert(url, dialog);
        dialog->insertExtendedControl(index, widget);
        dialog->show();
        connect(dialog, &DevicePropertyDialog::closed, this, &FilePropertyDialogManager::closeDevicePropertyDialog);
    }
}

void FilePropertyDialogManager::addExtendedControlDeviceProperty(const QUrl &url, ExtendedControlView *widget)
{
    if (devicePropertyDialogs.contains(url) && widget) {
        devicePropertyDialogs.value(url)->addExtendedControl(widget);
    } else if (widget) {
        DevicePropertyDialog *dialog = new DevicePropertyDialog();
        devicePropertyDialogs.insert(url, dialog);
        dialog->addExtendedControl(widget);
        dialog->show();
        connect(dialog, &DevicePropertyDialog::closed, this, &FilePropertyDialogManager::closeDevicePropertyDialog);
    }
}

void FilePropertyDialogManager::insertExtendedControlDeviceProperty(const QUrl &url, int index, ExtendedControlDrawerView *widget, bool expansion)
{
    if (devicePropertyDialogs.contains(url) && widget) {
        devicePropertyDialogs.value(url)->insertExtendedControl(index, widget, expansion);
    } else if (widget) {
        DevicePropertyDialog *dialog = new DevicePropertyDialog();
        devicePropertyDialogs.insert(url, dialog);
        dialog->insertExtendedControl(index, widget, expansion);
        dialog->show();
        connect(dialog, &DevicePropertyDialog::closed, this, &FilePropertyDialogManager::closeDevicePropertyDialog);
    }
}

void FilePropertyDialogManager::addExtendedControlDeviceProperty(const QUrl &url, ExtendedControlDrawerView *widget, bool expansion)
{
    if (devicePropertyDialogs.contains(url) && widget) {
        devicePropertyDialogs.value(url)->addExtendedControl(widget, expansion);
    } else if (widget) {
        DevicePropertyDialog *dialog = new DevicePropertyDialog();
        devicePropertyDialogs.insert(url, dialog);
        dialog->addExtendedControl(widget, expansion);
        dialog->show();
        connect(dialog, &DevicePropertyDialog::closed, this, &FilePropertyDialogManager::closeDevicePropertyDialog);
    }
}

void FilePropertyDialogManager::closeDevicePropertyDialog(const QUrl &url)
{
    if (devicePropertyDialogs.contains(url)) {
        devicePropertyDialogs.remove(url);
    }
}

FilePropertyDialogManager *FilePropertyDialogManager::instance()
{
    static FilePropertyDialogManager propertyManager;
    return &propertyManager;
}
