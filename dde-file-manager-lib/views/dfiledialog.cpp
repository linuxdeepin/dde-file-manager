#include "dfiledialog.h"
#include "views/dstatusbar.h"

#include <DTitlebar>
#include <dtextbutton.h>
#include <dthememanager.h>

#include <QEventLoop>
#include <QPointer>
#include <QWhatsThis>
#include <QShowEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

class DFileDialogPrivate
{
public:
    int result = 0;

    DTextButton *rejectButton;
    DTextButton *acceptButton;
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    QEventLoop *eventLoop = Q_NULLPTR;
    QStringList nameFilters;
};

DFileDialog::DFileDialog(QWidget *parent)
    : DFileManagerWindow(parent)
    , d_ptr(new DFileDialogPrivate())
{
    setWindowFlags(windowFlags() | Qt::Dialog);

    DThemeManager::instance()->setTheme("light");

    if (titleBar())
        titleBar()->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

    DStatusBar *statusBar = getFileView()->statusBar();

    if (statusBar->layout()) {
        DTextButton *cancelButton = new DTextButton(tr("Cancel"));
        DTextButton *openButton = new DTextButton(tr("Open"));

        statusBar->layout()->addWidget(cancelButton);
        statusBar->layout()->addWidget(openButton);
        statusBar->layout()->setContentsMargins(10, 10, 10, 10);

        connect(cancelButton, &DTextButton::clicked, this, &DFileDialog::reject);
        connect(openButton, &DTextButton::clicked, this, &DFileDialog::accept);

        d_func()->rejectButton = cancelButton;
        d_func()->acceptButton = openButton;
    }
}

DFileDialog::~DFileDialog()
{

}

void DFileDialog::setDirectory(const QString &directory)
{
    setDirectoryUrl(DUrl::fromLocalFile(directory));
}

void DFileDialog::setDirectory(const QDir &directory)
{
    setDirectoryUrl(DUrl::fromLocalFile(directory.absolutePath()));
}

QDir DFileDialog::directory() const
{
    return QDir(directoryUrl().toLocalFile());
}

void DFileDialog::setDirectoryUrl(const DUrl &directory)
{
    getFileView()->cd(directory);
}

QUrl DFileDialog::directoryUrl() const
{
    return getFileView()->currentUrl();
}

void DFileDialog::selectFile(const QString &filename)
{
    DUrl url = currentUrl();
    QDir dir(url.path());

    url.setPath(dir.absoluteFilePath(filename));

    selectUrl(url);
}

QStringList DFileDialog::selectedFiles() const
{
    QStringList list;

    for (const DUrl &url : selectedUrls())
        list << url.toLocalFile();

    return list;
}

void DFileDialog::selectUrl(const QUrl &url)
{
    getFileView()->select(DUrlList() << url);
}

QList<QUrl> DFileDialog::selectedUrls() const
{
    return DUrl::toQUrlList(getFileView()->selectedUrls());
}

void DFileDialog::setNameFilters(const QStringList &filters)
{
    D_D(DFileDialog);

    d->nameFilters = filters;

    if (selectedNameFilter().isEmpty())
        selectNameFilter(filters.isEmpty() ? QString() : filters.first());
}

QStringList DFileDialog::nameFilters() const
{
    D_DC(DFileDialog);

    return d->nameFilters;
}

void DFileDialog::selectNameFilter(const QString &filter)
{
    if (!filter.isEmpty())
        getFileView()->setNameFilters(QStringList() << filter);
}

QString DFileDialog::selectedNameFilter() const
{
    const QStringList &filters = getFileView()->nameFilters();

    if (filters.isEmpty())
        return QString();

    return filters.first();
}

QDir::Filters DFileDialog::filter() const
{
    return getFileView()->filters();
}

void DFileDialog::setFilter(QDir::Filters filters)
{
    getFileView()->setFilters(filters);
}

void DFileDialog::setViewMode(DFileView::ViewMode mode)
{
    getFileView()->setViewMode(mode);
}

DFileView::ViewMode DFileDialog::viewMode() const
{
    return getFileView()->viewMode();
}

void DFileDialog::setFileMode(QFileDialog::FileMode mode)
{
    switch (mode) {
    case QFileDialog::ExistingFile:
        getFileView()->setSelectionMode(QAbstractItemView::SingleSelection);
        break;
    case QFileDialog::ExistingFiles:
        getFileView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
        break;
    default: break;
    }
}

void DFileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    D_D(DFileDialog);

    d->acceptMode = mode;

    if (mode == QFileDialog::AcceptOpen)
        d->acceptButton->setText(tr("Open"));
    else
        d->acceptButton->setText(tr("Save"));
}

QFileDialog::AcceptMode DFileDialog::acceptMode() const
{
    D_DC(DFileDialog);

    return d->acceptMode;
}

void DFileDialog::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    D_D(DFileDialog);

    switch (label) {
    case QFileDialog::Accept:
        d->acceptButton->setText(text);
        break;
    case QFileDialog::Reject:
        d->rejectButton->setText(text);
        break;
    default:
        break;
    }
}

QString DFileDialog::labelText(QFileDialog::DialogLabel label) const
{
    D_DC(DFileDialog);

    switch (label) {
    case QFileDialog::Accept:
        return d->acceptButton->text();
    case QFileDialog::Reject:
        return d->rejectButton->text();
    default:
        break;
    }

    return QString();
}

void DFileDialog::accept()
{
    done(QDialog::Accepted);
}

void DFileDialog::done(int r)
{
    D_D(DFileDialog);

    if (d->eventLoop)
        d->eventLoop->exit(r);

    hide();

    emit finished(r);
    if (r == QDialog::Accepted)
        emit accepted();
    else if (r == QDialog::Rejected)
        emit rejected();
}

int DFileDialog::exec()
{
    D_D(DFileDialog);

    if (d->eventLoop) {
        qWarning("DFileDialog::exec: Recursive call detected");
        return -1;
    }

    bool deleteOnClose = testAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_DeleteOnClose, false);

    bool wasShowModal = testAttribute(Qt::WA_ShowModal);
    setAttribute(Qt::WA_ShowModal, true);

    show();

    QPointer<DFileDialog> guard = this;
    QEventLoop eventLoop;
    d->eventLoop = &eventLoop;
    int res = eventLoop.exec(QEventLoop::DialogExec);
    if (guard.isNull())
        return QDialog::Rejected;
    d->eventLoop = 0;

    setAttribute(Qt::WA_ShowModal, wasShowModal);

    if (deleteOnClose)
        delete this;
    return res;
}

void DFileDialog::open()
{
    show();
}

void DFileDialog::reject()
{
    done(QDialog::Rejected);
}

void DFileDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous() && !testAttribute(Qt::WA_Moved)) {
        Qt::WindowStates  state = windowState();
        adjustPosition(parentWidget());
        setAttribute(Qt::WA_Moved, false); // not really an explicit position
        if (state != windowState())
            setWindowState(state);
    }
}

void DFileDialog::closeEvent(QCloseEvent *event)
{
#ifndef QT_NO_WHATSTHIS
    if (isModal() && QWhatsThis::inWhatsThisMode())
        QWhatsThis::leaveWhatsThisMode();
#endif
    if (isVisible()) {
        QPointer<QObject> that = this;
        reject();
        if (that && isVisible())
            event->ignore();
    } else {
        event->accept();
    }
}

void DFileDialog::adjustPosition(QWidget *w)
{
    if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme())
        if (theme->themeHint(QPlatformTheme::WindowAutoPlacement).toBool())
            return;
    QPoint p(0, 0);
    int extraw = 0, extrah = 0, scrn = 0;
    if (w)
        w = w->window();
    QRect desk;
    if (w) {
        scrn = QApplication::desktop()->screenNumber(w);
    } else if (QApplication::desktop()->isVirtualDesktop()) {
        scrn = QApplication::desktop()->screenNumber(QCursor::pos());
    } else {
        scrn = QApplication::desktop()->screenNumber(this);
    }
    desk = QApplication::desktop()->availableGeometry(scrn);

    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; (extraw == 0 || extrah == 0) && i < list.size(); ++i) {
        QWidget * current = list.at(i);
        if (current->isVisible()) {
            int framew = current->geometry().x() - current->x();
            int frameh = current->geometry().y() - current->y();

            extraw = qMax(extraw, framew);
            extrah = qMax(extrah, frameh);
        }
    }

    // sanity check for decoration frames. With embedding, we
    // might get extraordinary values
    if (extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40) {
        extrah = 40;
        extraw = 10;
    }


    if (w) {
        // Use pos() if the widget is embedded into a native window
        QPoint pp;
        if (w->windowHandle() && w->windowHandle()->property("_q_embedded_native_parent_handle").value<WId>())
            pp = w->pos();
        else
            pp = w->mapToGlobal(QPoint(0,0));
        p = QPoint(pp.x() + w->width()/2,
                    pp.y() + w->height()/ 2);
    } else {
        // p = middle of the desktop
        p = QPoint(desk.x() + desk.width()/2, desk.y() + desk.height()/2);
    }

    // p = origin of this
    p = QPoint(p.x()-width()/2 - extraw,
                p.y()-height()/2 - extrah);


    if (p.x() + extraw + width() > desk.x() + desk.width())
        p.setX(desk.x() + desk.width() - width() - extraw);
    if (p.x() < desk.x())
        p.setX(desk.x());

    if (p.y() + extrah + height() > desk.y() + desk.height())
        p.setY(desk.y() + desk.height() - height() - extrah);
    if (p.y() < desk.y())
        p.setY(desk.y());

    move(p);
}
