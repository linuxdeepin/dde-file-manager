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
#include "filedialoghandle.h"
#include "views/filedialog.h"
#include "events/coreeventscaller.h"
#include "utils/corehelper.h"

#include "services/filemanager/windows/windowsservice.h"

#include "dfm-base/base/urlroute.h"

#include <QPointer>
#include <QWindow>
#include <QTimer>

#include <mutex>

DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

DIALOGCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class FileDialogHandlePrivate
{
public:
    explicit FileDialogHandlePrivate(FileDialogHandle *qq)
        : q_ptr(qq) {}

    QPointer<FileDialog> dialog;

    FileDialogHandle *q_ptr;

    Q_DECLARE_PUBLIC(FileDialogHandle)
};

FileDialogHandle::FileDialogHandle(QWidget *parent)
    : QObject(parent),
      d_ptr(new FileDialogHandlePrivate(this))
{
    d_func()->dialog = qobject_cast<FileDialog *>(WindowsService::service()->createWindow({}, true));
    if (!d_func()->dialog) {
        qCritical() << "Create window failed";
        abort();
    }
    d_func()->dialog->hide();

    connect(d_func()->dialog, &FileDialog::accepted, this, &FileDialogHandle::accepted);
    connect(d_func()->dialog, &FileDialog::rejected, this, &FileDialogHandle::rejected);
    connect(d_func()->dialog, &FileDialog::finished, this, &FileDialogHandle::finished);
    connect(d_func()->dialog, &FileDialog::selectionFilesChanged,
            this, &FileDialogHandle::selectionFilesChanged);
    connect(d_func()->dialog, &FileDialog::currentUrlChanged,
            this, &FileDialogHandle::currentUrlChanged);
    connect(d_func()->dialog, &FileDialog::selectedNameFilterChanged,
            this, &FileDialogHandle::selectedNameFilterChanged);
}

FileDialogHandle::~FileDialogHandle()
{
}

void FileDialogHandle::setParent(QWidget *parent)
{
    D_D(FileDialogHandle);

    d->dialog->setParent(parent);

    QObject::setParent(parent);
}

QWidget *FileDialogHandle::widget() const
{
    D_DC(FileDialogHandle);

    return d->dialog;
}

void FileDialogHandle::setDirectory(const QString &directory)
{
    D_D(FileDialogHandle);

    d->dialog->setDirectory(directory);
}

void FileDialogHandle::setDirectory(const QDir &directory)
{
    D_D(FileDialogHandle);

    d->dialog->setDirectory(directory);
}

QDir FileDialogHandle::directory() const
{
    D_DC(FileDialogHandle);

    return d->dialog->directory();
}

void FileDialogHandle::setDirectoryUrl(const QUrl &directory)
{
    D_D(FileDialogHandle);

    d->dialog->setDirectoryUrl(directory);
}

QUrl FileDialogHandle::directoryUrl() const
{
    D_DC(FileDialogHandle);

    return d->dialog->directoryUrl();
}

void FileDialogHandle::selectFile(const QString &filename)
{
    D_D(FileDialogHandle);

    CoreHelper::delayInvokeProxy(
            [d, filename] {
                d->dialog->selectFile(filename);
            },
            d->dialog->internalWinId(), this);
}

QStringList FileDialogHandle::selectedFiles() const
{
    D_DC(FileDialogHandle);

    return d->dialog->selectedFiles();
}

void FileDialogHandle::selectUrl(const QUrl &url)
{
    D_D(FileDialogHandle);

    CoreHelper::delayInvokeProxy(
            [d, url] {
                d->dialog->selectUrl(url);
            },
            d->dialog->internalWinId(), this);
}

QList<QUrl> FileDialogHandle::selectedUrls() const
{
    D_DC(FileDialogHandle);

    return d->dialog->selectedUrls();
}

void FileDialogHandle::addDisableUrlScheme(const QString &scheme)
{
    D_D(FileDialogHandle);

    CoreHelper::delayInvokeProxy(
            [d, scheme] {
                d->dialog->urlSchemeEnable(scheme, false);
            },
            d->dialog->internalWinId(), this);
}

void FileDialogHandle::setNameFilters(const QStringList &filters)
{
    D_D(FileDialogHandle);

    CoreHelper::delayInvokeProxy(
            [d, filters] {
                d->dialog->setNameFilters(filters);
            },
            d->dialog->internalWinId(), this);
}

QStringList FileDialogHandle::nameFilters() const
{
    D_DC(FileDialogHandle);

    return d->dialog->nameFilters();
}

void FileDialogHandle::selectNameFilter(const QString &filter)
{
    D_D(FileDialogHandle);

    d->dialog->selectNameFilter(filter);
}

QString FileDialogHandle::selectedNameFilter() const
{
    D_DC(FileDialogHandle);

    return d->dialog->selectedNameFilter();
}

void FileDialogHandle::selectNameFilterByIndex(int index)
{
    D_DC(FileDialogHandle);

    d->dialog->selectNameFilterByIndex(index);
}

int FileDialogHandle::selectedNameFilterIndex() const
{
    D_DC(FileDialogHandle);

    return d->dialog->selectedNameFilterIndex();
}

QDir::Filters FileDialogHandle::filter() const
{
    D_DC(FileDialogHandle);

    return d->dialog->filter();
}

void FileDialogHandle::setFilter(QDir::Filters filters)
{
    D_D(FileDialogHandle);

    CoreHelper::delayInvokeProxy(
            [d, filters]() {
                d->dialog->setFilter(filters);
            },
            d->dialog->internalWinId(), this);
}

void FileDialogHandle::setViewMode(QFileDialog::ViewMode mode)
{
    D_D(FileDialogHandle);

    if (mode == QFileDialog::Detail)
        CoreEventsCaller::sendViewMode(d->dialog, DFMBASE_NAMESPACE::Global::ViewMode::kListMode);
    else
        CoreEventsCaller::sendViewMode(d->dialog, DFMBASE_NAMESPACE::Global::ViewMode::kIconMode);
}

QFileDialog::ViewMode FileDialogHandle::viewMode() const
{
    D_DC(FileDialogHandle);

    return d->dialog->currentViewMode();
}

void FileDialogHandle::setFileMode(QFileDialog::FileMode mode)
{
    D_D(FileDialogHandle);

    CoreHelper::delayInvokeProxy(
            [d, mode]() {
                d->dialog->setFileMode(mode);
            },
            d->dialog->internalWinId(), this);
}

void FileDialogHandle::setAcceptMode(QFileDialog::AcceptMode mode)
{
    D_D(FileDialogHandle);
    CoreHelper::delayInvokeProxy(
            [d, mode]() {
                d->dialog->setAcceptMode(mode);
            },
            d->dialog->internalWinId(), this);
}

QFileDialog::AcceptMode FileDialogHandle::acceptMode() const
{
    D_DC(FileDialogHandle);

    return d->dialog->acceptMode();
}

void FileDialogHandle::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    D_D(FileDialogHandle);

    d->dialog->setLabelText(label, text);
}

QString FileDialogHandle::labelText(QFileDialog::DialogLabel label) const
{
    D_DC(FileDialogHandle);

    return d->dialog->labelText(label);
}

void FileDialogHandle::setOptions(QFileDialog::Options options)
{
    D_DC(FileDialogHandle);

    return d->dialog->setOptions(options);
}

void FileDialogHandle::setOption(QFileDialog::Option option, bool on)
{
    D_DC(FileDialogHandle);

    d->dialog->setOption(option, on);
}

QFileDialog::Options FileDialogHandle::options() const
{
    D_DC(FileDialogHandle);

    return d->dialog->options();
}

bool FileDialogHandle::testOption(QFileDialog::Option option) const
{
    D_DC(FileDialogHandle);

    return d->dialog->testOption(option);
}

void FileDialogHandle::setCurrentInputName(const QString &name)
{
    D_DC(FileDialogHandle);

    d->dialog->setCurrentInputName(name);
}

void FileDialogHandle::addCustomWidget(int type, const QString &data)
{
    D_D(FileDialogHandle);

    d->dialog->addCustomWidget(static_cast<FileDialog::CustomWidgetType>(type), data);
}

QDBusVariant FileDialogHandle::getCustomWidgetValue(int type, const QString &text) const
{
    D_DC(FileDialogHandle);

    return QDBusVariant(d->dialog->getCustomWidgetValue(static_cast<FileDialog::CustomWidgetType>(type), text));
}

QVariantMap FileDialogHandle::allCustomWidgetsValue(int type) const
{
    D_DC(FileDialogHandle);

    return d->dialog->allCustomWidgetsValue(static_cast<FileDialog::CustomWidgetType>(type));
}

void FileDialogHandle::beginAddCustomWidget()
{
    D_D(FileDialogHandle);

    d->dialog->beginAddCustomWidget();
}

void FileDialogHandle::endAddCustomWidget()
{
    D_D(FileDialogHandle);

    d->dialog->endAddCustomWidget();
}

void FileDialogHandle::setAllowMixedSelection(bool on)
{
    D_D(FileDialogHandle);

    d->dialog->setAllowMixedSelection(on);
}

void FileDialogHandle::setHideOnAccept(bool enable)
{
    D_D(FileDialogHandle);

    d->dialog->setHideOnAccept(enable);
}

bool FileDialogHandle::hideOnAccept() const
{
    D_DC(FileDialogHandle);

    return d->dialog->hideOnAccept();
}

void FileDialogHandle::show()
{
    D_D(FileDialogHandle);

    WindowsService::service()->showWindow(d->dialog);
    d->dialog->updateAsDefaultSize();
}

void FileDialogHandle::hide()
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->hide();
}

void FileDialogHandle::accept()
{
    D_D(FileDialogHandle);

    d->dialog->accept();
}

void FileDialogHandle::done(int r)
{
    D_D(FileDialogHandle);

    d->dialog->done(r);
}

int FileDialogHandle::exec()
{
    D_D(FileDialogHandle);

    return d->dialog->exec();
}

void FileDialogHandle::open()
{
    D_D(FileDialogHandle);

    d->dialog->open();
}

void FileDialogHandle::reject()
{
    D_D(FileDialogHandle);

    d->dialog->reject();
}
