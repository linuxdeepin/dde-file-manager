#include "dfiledialoghandle.h"
#include "views/dfiledialog.h"

#include <dthememanager.h>

#include <QPointer>

DWIDGET_USE_NAMESPACE

class DFileDialogHandlePrivate
{
public:
    DFileDialogHandlePrivate(DFileDialogHandle *qq)
        : q_ptr(qq) {}

    ~DFileDialogHandlePrivate()
    {
        if (dialog)
            dialog->deleteLater();
    }

    QPointer<DFileDialog> dialog;

    DFileDialogHandle *q_ptr;

    Q_DECLARE_PUBLIC(DFileDialogHandle)
};

DFileDialogHandle::DFileDialogHandle(QWidget *parent)
    : QObject(parent)
    , d_ptr(new DFileDialogHandlePrivate(this))
{
    //bug: https://tower.im/projects/1d2977ef6b194727a97f96409f77038c/todos/f5937609238b46f1b9d1d769d2a08fad/
    QString oldThemeName = DThemeManager::instance()->theme();
    QSignalBlocker blocker(DThemeManager::instance());
    DThemeManager::instance()->setTheme("light");
    d_func()->dialog = new DFileDialog(parent);
    DThemeManager::instance()->setTheme(oldThemeName);
    blocker.unblock();

    connect(d_func()->dialog, &DFileDialog::accepted, this, &DFileDialogHandle::accepted);
    connect(d_func()->dialog, &DFileDialog::rejected, this, &DFileDialogHandle::rejected);
    connect(d_func()->dialog, &DFileDialog::finished, this, &DFileDialogHandle::finished);
    connect(d_func()->dialog, &DFileDialog::selectionFilesChanged,
            this, &DFileDialogHandle::selectionFilesChanged);
    connect(d_func()->dialog, &DFileDialog::currentUrlChanged,
            this, &DFileDialogHandle::currentUrlChanged);
    connect(d_func()->dialog, &DFileDialog::selectedNameFilterChanged,
            this, &DFileDialogHandle::selectedNameFilterChanged);
}

DFileDialogHandle::~DFileDialogHandle()
{

}

void DFileDialogHandle::setParent(QWidget *parent)
{
    D_D(DFileDialogHandle);

    d->dialog->setParent(parent);

    QObject::setParent(parent);
}

QWidget *DFileDialogHandle::widget() const
{
    D_DC(DFileDialogHandle);

    return d->dialog;
}

void DFileDialogHandle::setDirectory(const QString &directory)
{
    D_D(DFileDialogHandle);

    d->dialog->setDirectory(directory);
}

void DFileDialogHandle::setDirectory(const QDir &directory)
{
    D_D(DFileDialogHandle);

    d->dialog->setDirectory(directory);
}

QDir DFileDialogHandle::directory() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->directory();
}

void DFileDialogHandle::setDirectoryUrl(const QUrl &directory)
{
    D_D(DFileDialogHandle);

    d->dialog->setDirectoryUrl(directory);
}

QUrl DFileDialogHandle::directoryUrl() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->directoryUrl();
}

void DFileDialogHandle::selectFile(const QString &filename)
{
    D_D(DFileDialogHandle);

    d->dialog->selectFile(filename);
}

QStringList DFileDialogHandle::selectedFiles() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->selectedFiles();
}

void DFileDialogHandle::selectUrl(const QUrl &url)
{
    D_D(DFileDialogHandle);

    d->dialog->selectUrl(url);
}

QList<QUrl> DFileDialogHandle::selectedUrls() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->selectedUrls();
}

void DFileDialogHandle::setNameFilters(const QStringList &filters)
{
    D_D(DFileDialogHandle);

    d->dialog->setNameFilters(filters);
}

QStringList DFileDialogHandle::nameFilters() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->nameFilters();
}

void DFileDialogHandle::selectNameFilter(const QString &filter)
{
    D_D(DFileDialogHandle);

    d->dialog->selectNameFilter(filter);
}

QString DFileDialogHandle::selectedNameFilter() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->selectedNameFilter();
}

void DFileDialogHandle::selectNameFilterByIndex(int index)
{
    D_DC(DFileDialogHandle);

    d->dialog->selectNameFilterByIndex(index);
}

int DFileDialogHandle::selectedNameFilterIndex() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->selectedNameFilterIndex();
}

QDir::Filters DFileDialogHandle::filter() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->filter();
}

void DFileDialogHandle::setFilter(QDir::Filters filters)
{
    D_D(DFileDialogHandle);

    d->dialog->setFilter(filters);
}

void DFileDialogHandle::setViewMode(QFileDialog::ViewMode mode)
{
    D_D(DFileDialogHandle);

    if (mode == QFileDialog::Detail)
        d->dialog->setViewMode(DFileView::ListMode);
    else
        d->dialog->setViewMode(DFileView::IconMode);
}

QFileDialog::ViewMode DFileDialogHandle::viewMode() const
{
    D_DC(DFileDialogHandle);

    if (d->dialog->viewMode() == DFileView::ListMode)
        return QFileDialog::Detail;

    return QFileDialog::List;
}

void DFileDialogHandle::setFileMode(QFileDialog::FileMode mode)
{
    D_D(DFileDialogHandle);

    d->dialog->setFileMode(mode);
}

void DFileDialogHandle::setAcceptMode(QFileDialog::AcceptMode mode)
{
    D_D(DFileDialogHandle);

    d->dialog->setAcceptMode(mode);
}

QFileDialog::AcceptMode DFileDialogHandle::acceptMode() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->acceptMode();
}

void DFileDialogHandle::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    D_D(DFileDialogHandle);

    d->dialog->setLabelText(label, text);
}

QString DFileDialogHandle::labelText(QFileDialog::DialogLabel label) const
{
    D_DC(DFileDialogHandle);

    return d->dialog->labelText(label);
}

void DFileDialogHandle::setOptions(QFileDialog::Options options)
{
    D_DC(DFileDialogHandle);

    return d->dialog->setOptions(options);
}

void DFileDialogHandle::setOption(QFileDialog::Option option, bool on)
{
    D_DC(DFileDialogHandle);

    d->dialog->setOption(option, on);
}

QFileDialog::Options DFileDialogHandle::options() const
{
    D_DC(DFileDialogHandle);

    return d->dialog->options();
}

bool DFileDialogHandle::testOption(QFileDialog::Option option) const
{
    D_DC(DFileDialogHandle);

    return d->dialog->testOption(option);
}

void DFileDialogHandle::setCurrentInputName(const QString &name)
{
    D_DC(DFileDialogHandle);

    d->dialog->setCurrentInputName(name);
}

void DFileDialogHandle::show()
{
    D_D(DFileDialogHandle);

    d->dialog->show();
}

void DFileDialogHandle::hide()
{
    D_D(DFileDialogHandle);

    d->dialog->hide();
}

void DFileDialogHandle::accept()
{
    D_D(DFileDialogHandle);

    d->dialog->accept();
}

void DFileDialogHandle::done(int r)
{
    D_D(DFileDialogHandle);

    d->dialog->done(r);
}

int DFileDialogHandle::exec()
{
    D_D(DFileDialogHandle);

    return d->dialog->exec();
}

void DFileDialogHandle::open()
{
    D_D(DFileDialogHandle);

    d->dialog->open();
}

void DFileDialogHandle::reject()
{
    D_D(DFileDialogHandle);

    d->dialog->reject();
}
