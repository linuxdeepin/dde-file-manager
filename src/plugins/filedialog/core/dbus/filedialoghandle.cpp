// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialoghandle.h"
#include "views/filedialogstatusbar.h"
#include "events/coreeventscaller.h"
#include "utils/corehelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QPointer>
#include <QWindow>
#include <QTimer>
#include <QEventLoop>
#include <QGuiApplication>
#include <QScreen>

#include <mutex>

DFMBASE_USE_NAMESPACE
using namespace filedialog_core;
DWIDGET_USE_NAMESPACE

class FileDialogHandlePrivate
{
public:
    explicit FileDialogHandlePrivate(FileDialogHandle *qq)
        : q_ptr(qq) { }

    QPointer<FileDialog> dialog;
    QStringList lastFilterGroup;
    QString lastFilter;

    FileDialogHandle *q_ptr;

    Q_DECLARE_PUBLIC(FileDialogHandle)
};

FileDialogHandle::FileDialogHandle(QWidget *parent)
    : QObject(parent),
      d_ptr(new FileDialogHandlePrivate(this))
{
    d_func()->dialog = qobject_cast<FileDialog *>(FMWindowsIns.createWindow({}, true));
    if (!d_func()->dialog) {
        fmCritical() << "File Dialog: Create window failed";
        abort();
    }
    auto defaultUrl = d_func()->dialog->lastVisitedUrl();
    if (!defaultUrl.isValid())
        defaultUrl = QUrl::fromLocalFile(DFMBASE_NAMESPACE::StandardPaths::location(StandardPaths::kHomePath));
    d_func()->dialog->cd(defaultUrl);

    //! no need to hide, if the dialog is showed in creating, it must be bug.
    //! see bug#22564
    // d_func()->dialog->hide();

    connect(d_func()->dialog, &FileDialog::accepted, this, &FileDialogHandle::accepted);
    connect(d_func()->dialog, &FileDialog::rejected, this, &FileDialogHandle::rejected);
    connect(d_func()->dialog, &FileDialog::finished, this, &FileDialogHandle::finished);
    connect(d_func()->dialog, &FileDialog::selectionFilesChanged,
            this, &FileDialogHandle::selectionFilesChanged);
    connect(d_func()->dialog, &FileDialog::currentUrlChanged,
            this, &FileDialogHandle::currentUrlChanged);
    connect(d_func()->dialog, &FileDialog::selectedNameFilterChanged,
            this, &FileDialogHandle::selectedNameFilterChanged);

    auto window = qobject_cast<FileDialog *>(FMWindowsIns.findWindowById(d_func()->dialog->internalWinId()));
    if (window) {
        QObject::connect(window, &FileDialog::initialized, this, [this]() {
            if (!d_func()->lastFilterGroup.isEmpty())
                d_func()->dialog->setNameFilters(d_func()->lastFilterGroup);
            if (!d_func()->lastFilter.isEmpty())
                d_func()->dialog->selectNameFilter(d_func()->lastFilter);
        });
    }
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

    if (d->dialog)
        d->dialog->setDirectory(directory);
}

void FileDialogHandle::setDirectory(const QDir &directory)
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->setDirectory(directory);
}

QDir FileDialogHandle::directory() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->directory();

    return {};
}

void FileDialogHandle::setDirectoryUrl(const QUrl &directory)
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->setDirectoryUrl(directory);
}

QUrl FileDialogHandle::directoryUrl() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->directoryUrl();
    return {};
}

void FileDialogHandle::selectFile(const QString &filename)
{
    D_D(FileDialogHandle);

    if (!d->dialog)
        return;

    CoreHelper::delayInvokeProxy(
            [dialog = d->dialog, filename] {
                if (dialog)
                    dialog->selectFile(filename);
            },
            d->dialog->internalWinId(), this);
}

QStringList FileDialogHandle::selectedFiles() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->selectedFiles();
    return {};
}

void FileDialogHandle::selectUrl(const QUrl &url)
{
    D_D(FileDialogHandle);

    if (!d->dialog)
        return;

    CoreHelper::delayInvokeProxy(
            [dialog = d->dialog, url] {
                if (dialog)
                    dialog->selectUrl(url);
            },
            d->dialog->internalWinId(), this);
}

QList<QUrl> FileDialogHandle::selectedUrls() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->selectedUrls();
    return {};
}

void FileDialogHandle::addDisableUrlScheme(const QString &scheme)
{
    D_D(FileDialogHandle);

    if (!d->dialog)
        return;

    CoreHelper::delayInvokeProxy(
            [dialog = d->dialog, scheme] {
                if (dialog)
                    dialog->urlSchemeEnable(scheme, false);
            },
            d->dialog->internalWinId(), this);
}

void FileDialogHandle::setNameFilters(const QStringList &filters)
{
    D_D(FileDialogHandle);

    if (d->dialog == nullptr)
        return;

    auto window = qobject_cast<FileDialog *>(FMWindowsIns.findWindowById(d->dialog->internalWinId()));
    Q_ASSERT(window);

    isSetNameFilters = true;

    if (window && window->workSpace()) {
        if (d->dialog)
            d->dialog->setNameFilters(filters);
        d->lastFilterGroup.clear();
    } else {
        d->lastFilterGroup = filters;
    }
}

QStringList FileDialogHandle::nameFilters() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->nameFilters();
    return {};
}

void FileDialogHandle::selectNameFilter(const QString &filter)
{
    D_D(FileDialogHandle);

    if (d->dialog == nullptr)
        return;

    auto window = qobject_cast<FileDialog *>(FMWindowsIns.findWindowById(d->dialog->internalWinId()));
    Q_ASSERT(window);

    if (window && window->workSpace()) {
        if (d->dialog)
            d->dialog->selectNameFilter(filter);
        d->lastFilter.clear();
    } else {
        d->lastFilter = filter;
    }
}

QString FileDialogHandle::selectedNameFilter() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->selectedNameFilter();

    return {};
}

void FileDialogHandle::selectNameFilterByIndex(int index)
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        d->dialog->selectNameFilterByIndex(index);
}

int FileDialogHandle::selectedNameFilterIndex() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->selectedNameFilterIndex();

    return {};
}

qulonglong FileDialogHandle::winId() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->internalWinId();
    return 0;
}

QDir::Filters FileDialogHandle::filter() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->filter();

    return {};
}

void FileDialogHandle::setFilter(QDir::Filters filters)
{
    D_D(FileDialogHandle);

    if (!d->dialog)
        return;

    CoreHelper::delayInvokeProxy(
            [dialog = d->dialog, filters]() {
                if (dialog)
                    dialog->setFilter(filters);
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

    if (d->dialog)
        return d->dialog->currentViewMode();

    return {};
}

void FileDialogHandle::setFileMode(QFileDialog::FileMode mode)
{
    D_D(FileDialogHandle);

    if (!d->dialog)
        return;

    CoreHelper::delayInvokeProxy(
            [dialog = d->dialog, mode]() {
                if (dialog)
                    dialog->setFileMode(mode);
            },
            d->dialog->internalWinId(), this);
}

void FileDialogHandle::setAcceptMode(QFileDialog::AcceptMode mode)
{
    D_D(FileDialogHandle);
    
    if (!d->dialog)
        return;
    
    isSetAcceptMode = true;
    CoreHelper::delayInvokeProxy(
            [dialog = d->dialog, mode]() {
                if (dialog)
                    dialog->setAcceptMode(mode);
            },
            d->dialog->internalWinId(), this);
}

QFileDialog::AcceptMode FileDialogHandle::acceptMode() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->acceptMode();

    return {};
}

void FileDialogHandle::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->setLabelText(label, text);
}

QString FileDialogHandle::labelText(QFileDialog::DialogLabel label) const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->labelText(label);

    return {};
}

void FileDialogHandle::setOptions(QFileDialog::Options options)
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        d->dialog->setOptions(options);
}

void FileDialogHandle::setOption(QFileDialog::Option option, bool on)
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        d->dialog->setOption(option, on);
}

QFileDialog::Options FileDialogHandle::options() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->options();

    return {};
}

bool FileDialogHandle::testOption(QFileDialog::Option option) const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->testOption(option);

    return {};
}

void FileDialogHandle::setCurrentInputName(const QString &name)
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        d->dialog->setCurrentInputName(name);
}

void FileDialogHandle::addCustomWidget(int type, const QString &data)
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->addCustomWidget(static_cast<FileDialog::CustomWidgetType>(type), data);
}

QDBusVariant FileDialogHandle::getCustomWidgetValue(int type, const QString &text) const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return QDBusVariant(d->dialog->getCustomWidgetValue(static_cast<FileDialog::CustomWidgetType>(type), text));

    return {};
}

QVariantMap FileDialogHandle::allCustomWidgetsValue(int type) const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->allCustomWidgetsValue(static_cast<FileDialog::CustomWidgetType>(type));

    return {};
}

void FileDialogHandle::beginAddCustomWidget()
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->beginAddCustomWidget();
}

void FileDialogHandle::endAddCustomWidget()
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->endAddCustomWidget();
}

void FileDialogHandle::setAllowMixedSelection(bool on)
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->setAllowMixedSelection(on);
}

void FileDialogHandle::setHideOnAccept(bool enable)
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->setHideOnAccept(enable);
}

bool FileDialogHandle::hideOnAccept() const
{
    D_DC(FileDialogHandle);

    if (d->dialog)
        return d->dialog->hideOnAccept();

    return {};
}

void FileDialogHandle::show()
{
    D_D(FileDialogHandle);
    if (d->dialog) {
        addDefaultSettingForWindow(d->dialog);
        d->dialog->updateAsDefaultSize();
        d->dialog->moveCenter();
        setWindowStayOnTop();
        fmDebug() << QString("Select Dialog Info: befor show size is (%1, %2)").arg(d->dialog->width()).arg(d->dialog->height());
        FMWindowsIns.showWindow(d->dialog);
        fmDebug() << QString("Select Dialog Info: after show size is (%1, %2)").arg(d->dialog->width()).arg(d->dialog->height());
    }
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

    if (d->dialog)
        d->dialog->accept();
}

void FileDialogHandle::done(int r)
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->done(r);
}

int FileDialogHandle::exec()
{
    D_D(FileDialogHandle);

    if (d->dialog)
        return d->dialog->exec();

    return {};
}

void FileDialogHandle::open()
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->open();
}

void FileDialogHandle::reject()
{
    D_D(FileDialogHandle);

    if (d->dialog)
        d->dialog->reject();
}

void FileDialogHandle::addDefaultSettingForWindow(QPointer<FileDialog> dialog)
{
    if (!dialog)
        return;

    QVariant isGtk = qApp->property("GTK");
    if (!isGtk.isValid() || !isGtk.toBool())
        return;

    if (!isSetAcceptMode && dialog->statusBar())
        dialog->statusBar()->setMode(FileDialogStatusBar::Mode::kOpen);

    if (!isSetNameFilters && dialog->acceptMode() == QFileDialog::AcceptOpen) {
        QStringList filters { tr("All Files ") + "(*)" };
        dialog->setNameFilters(filters);
    }
}

void FileDialogHandle::setWindowStayOnTop()
{
    D_D(FileDialogHandle);
    QVariant isGtk = qApp->property("GTK");
    if (isGtk.isValid() && isGtk.toBool()) {
        if (WindowUtils::isWayLand()) {
            QFunctionPointer setWindowProperty = qApp->platformFunction("_d_setWindowProperty");
            // set window stay on top
            if (setWindowProperty && d->dialog)
                reinterpret_cast<void (*)(QWindow *, const char *, const QVariant &)>(setWindowProperty)(d->dialog->windowHandle(), "_d_dwayland_staysontop", true);
        } else {
            // BUG: 217379
            // BUG: 302951
            // TODO(zhangs): Maybe a bug in Qt6
            // The winId() interface fails to return correctly when WindowStaysOnTopHint is used.

            // if (d->dialog)
            //     d->dialog->setWindowFlag(Qt::WindowStaysOnTopHint, true);
        }
    }
}
