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

#include "dfiledialog.h"
#include "dfilesystemmodel.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dfmeventdispatcher.h"
#include "dfmsidebar.h"
#include "dfmaddressbar.h"
#include "views/dstatusbar.h"
#include "views/filedialogstatusbar.h"

#include <DTitlebar>
#include <ddialog.h>
#include <DPlatformWindowHandle>

#include <QEventLoop>
#include <QPointer>
#include <QWhatsThis>
#include <QShowEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>
#include <qpa/qplatformdialoghelper.h>

class DFileDialogPrivate
{
public:
    int result = 0;
    bool hideOnAccept = true;

    QFileDialog::FileMode fileMode = QFileDialog::AnyFile;
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    QFileDialog::Options options;
    QEventLoop *eventLoop = Q_NULLPTR;
    QStringList nameFilters;

    DFileView *view = Q_NULLPTR;
    int currentNameFilterIndex = -1;
    QDir::Filters filters = 0;
    QString currentInputName;

    FileDialogStatusBar *statusBar;
};

DFileDialog::DFileDialog(QWidget *parent)
    : DFileManagerWindow(parent)
    , d_ptr(new DFileDialogPrivate())
{
    d_ptr->view = qobject_cast<DFileView *>(DFileManagerWindow::getFileView()->widget());

    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::Dialog);

    if (titlebar()) {
        titlebar()->setDisableFlags(Qt::WindowSystemMenuHint);
        titlebar()->setMenuVisible(false);
    }

    d_ptr->statusBar = new FileDialogStatusBar(this);
    centralWidget()->layout()->addWidget(d_ptr->statusBar);

    setAcceptMode(QFileDialog::AcceptOpen);
    handleNewView(DFileManagerWindow::getFileView());

    getLeftSideBar()->setDisableUrlSchemes(QSet<QString>() << "trash" << "network");
    getLeftSideBar()->setContextMenuEnabled(false);
    getLeftSideBar()->setAcceptDrops(false);

    DFMEventDispatcher::instance()->installEventFilter(this);

    QPlatformFileDialogHelper::filterRegExp = "^(.*)\\(([^()]*)\\)$";

    connect(statusBar()->acceptButton(), &QPushButton::clicked, this, &DFileDialog::onAcceptButtonClicked);
    connect(statusBar()->rejectButton(), &QPushButton::clicked, this, &DFileDialog::onRejectButtonClicked);
    connect(statusBar()->comboBox(),
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &DFileDialog::selectNameFilter);
    connect(statusBar()->comboBox(),
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &DFileDialog::selectedNameFilterChanged);
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
    return getFileView()->rootUrl();
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

    for (const DUrl &url : selectedUrls()) {
        list << url.toLocalFile();
    }

    return list;
}

void DFileDialog::selectUrl(const QUrl &url)
{
    getFileView()->select(DUrlList() << url);

    const DAbstractFileInfoPointer &fileInfo = getFileView()->model()->fileInfo(url);

    if (fileInfo && fileInfo->exists()) {
        return;
    }

    setCurrentInputName(QFileInfo(url.path()).fileName());
}

QList<QUrl> DFileDialog::selectedUrls() const
{
    D_DC(DFileDialog);

    DUrlList list = getFileView()->selectedUrls();

    DUrlList::iterator begin = list.begin();

    while (begin != list.end()) {
        const DAbstractFileInfoPointer &fileInfo = getFileView()->model()->fileInfo(*begin);

        if (fileInfo && !fileInfo->toLocalFile().isEmpty()) {
            DUrl newUrl = DUrl::fromLocalFile(fileInfo->toLocalFile());

            if (newUrl.isValid()) {
                *begin = newUrl;
            }
        }

        ++begin;
    }

    if (d->acceptMode == QFileDialog::AcceptSave) {
        DUrl fileUrl = list.isEmpty() ? getFileView()->rootUrl() : list.first();
        const DAbstractFileInfoPointer &fileInfo = getFileView()->model()->fileInfo(fileUrl);

        if (fileInfo) {
            if (list.isEmpty()) {
                fileUrl = fileInfo->getUrlByChildFileName(statusBar()->lineEdit()->text());
            } else {
                fileUrl = fileInfo->getUrlByNewFileName(statusBar()->lineEdit()->text());
            }
        }

        return QList<QUrl>() << fileUrl;
    }

    if (list.isEmpty() && (d->fileMode == QFileDialog::Directory
                           || d->fileMode == QFileDialog::DirectoryOnly)) {
        if (directoryUrl().isLocalFile())
            list << directoryUrl();
    }

    return DUrl::toQUrlList(list);
}

void DFileDialog::addDisableUrlScheme(const QString &scheme)
{
    QSet<QString> schemes = getLeftSideBar()->disableUrlSchemes();

    schemes << scheme;

    getLeftSideBar()->setDisableUrlSchemes(schemes);
}

/*
    Strip the filters by removing the details, e.g. (*.*).
*/
QStringList qt_strip_filters(const QStringList &filters)
{
    QStringList strippedFilters;
    QRegExp r(QString::fromLatin1(QPlatformFileDialogHelper::filterRegExp));
    const int numFilters = filters.count();
    strippedFilters.reserve(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        QString filterName = filters[i];
        int index = r.indexIn(filterName);
        if (index >= 0) {
            filterName = r.cap(1);
        }
        strippedFilters.append(filterName.simplified());
    }
    return strippedFilters;
}

void DFileDialog::setNameFilters(const QStringList &filters)
{
    D_D(DFileDialog);

    d->nameFilters = filters;

    if (testOption(QFileDialog::HideNameFilterDetails)) {
        statusBar()->setComBoxItems(qt_strip_filters(filters));
    } else {
        statusBar()->setComBoxItems(filters);
    }

    if (modelCurrentNameFilter().isEmpty()) {
        selectNameFilter(filters.isEmpty() ? QString() : filters.first());
    }
}

QStringList DFileDialog::nameFilters() const
{
    D_DC(DFileDialog);

    return d->nameFilters;
}

void DFileDialog::selectNameFilter(const QString &filter)
{
    QString key;

    if (testOption(QFileDialog::HideNameFilterDetails)) {
        key = qt_strip_filters(QStringList(filter)).first();
    } else {
        key = filter;
    }

    int index = statusBar()->comboBox()->findText(key);

    selectNameFilterByIndex(index);
}

QString DFileDialog::modelCurrentNameFilter() const
{
    const QStringList &filters = getFileView()->nameFilters();

    if (filters.isEmpty()) {
        return QString();
    }

    return filters.first();
}

QString DFileDialog::selectedNameFilter() const
{
    Q_D(const DFileDialog);

    const QComboBox *box = statusBar()->comboBox();

    return box ? d->nameFilters.value(box->currentIndex()) : QString();
}

void DFileDialog::selectNameFilterByIndex(int index)
{
    D_D(DFileDialog);

    if (index < 0 || index >= statusBar()->comboBox()->count()) {
        return;
    }

    statusBar()->comboBox()->setCurrentIndex(index);

    QStringList nameFilters = d->nameFilters;

    if (index == nameFilters.size()) {
        QAbstractItemModel *comboModel = statusBar()->comboBox()->model();
        nameFilters.append(comboModel->index(comboModel->rowCount() - 1, 0).data().toString());
        setNameFilters(nameFilters);
    }

    QString nameFilter = nameFilters.at(index);
    QStringList newNameFilters = QPlatformFileDialogHelper::cleanFilterList(nameFilter);
    if (d->acceptMode == QFileDialog::AcceptSave && !newNameFilters.isEmpty()) {
        QString newNameFilterExtension;
        QMimeDatabase db;
        QString fileName = statusBar()->lineEdit()->text();
        const QString fileNameExtension = db.suffixForFileName(fileName);

        for (const QString &filter : newNameFilters) {
            newNameFilterExtension = db.suffixForFileName(filter);

            QRegExp  re(newNameFilterExtension, Qt::CaseInsensitive, QRegExp::Wildcard);

            if (re.exactMatch(fileNameExtension)) {
                return getFileView()->setNameFilters(newNameFilters);
            }
        }

        newNameFilterExtension = db.suffixForFileName(newNameFilters.at(0));

        if (!fileNameExtension.isEmpty() && !newNameFilterExtension.isEmpty()) {
            const int fileNameExtensionLength = fileNameExtension.count();
            fileName.replace(fileName.count() - fileNameExtensionLength,
                             fileNameExtensionLength, newNameFilterExtension);
            setCurrentInputName(fileName);
        }
    }

    getFileView()->setNameFilters(newNameFilters);
}

int DFileDialog::selectedNameFilterIndex() const
{
    const QComboBox *box = statusBar()->comboBox();

    return box ? box->currentIndex() : -1;
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
    D_D(DFileDialog);

    if (d->fileMode == QFileDialog::DirectoryOnly
            || d->fileMode == QFileDialog::Directory) {
        // 清理只显示目录时对文件名添加的过滤条件
        getFileView()->setNameFilters(QStringList());
    }

    d->fileMode = mode;

    switch (static_cast<int>(mode)) {
    case QFileDialog::ExistingFiles:
        getFileView()->setEnabledSelectionModes(QSet<DFileView::SelectionMode>() << QAbstractItemView::ExtendedSelection);
        break;
    case QFileDialog::DirectoryOnly:
    case QFileDialog::Directory:
        // 文件名中不可能包含 '/', 此处目的是过滤掉所有文件
        getFileView()->setNameFilters(QStringList("/"));
        // fall through
    default:
        getFileView()->setEnabledSelectionModes(QSet<DFileView::SelectionMode>() << QAbstractItemView::SingleSelection);
        break;
    }
}

void DFileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    D_D(DFileDialog);

    d->acceptMode = mode;

    if (mode == QFileDialog::AcceptOpen) {
        statusBar()->setMode(FileDialogStatusBar::Open);
        setFileMode(d->fileMode);

        disconnect(statusBar()->lineEdit(), &QLineEdit::textChanged,
                   this, &DFileDialog::onCurrentInputNameChanged);
    } else {
        statusBar()->setMode(FileDialogStatusBar::Save);
        statusBar()->acceptButton()->setDisabled(statusBar()->lineEdit()->text().isEmpty());
        getFileView()->setSelectionMode(QAbstractItemView::SingleSelection);

        connect(statusBar()->lineEdit(), &QLineEdit::textChanged,
                this, &DFileDialog::onCurrentInputNameChanged);
    }
}

QFileDialog::AcceptMode DFileDialog::acceptMode() const
{
    D_DC(DFileDialog);

    return d->acceptMode;
}

void DFileDialog::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    switch (static_cast<int>(label)) {
    case QFileDialog::Accept:
        statusBar()->acceptButton()->setText(text);
        break;
    case QFileDialog::Reject:
        statusBar()->rejectButton()->setText(text);
        break;
    default:
        break;
    }
}

QString DFileDialog::labelText(QFileDialog::DialogLabel label) const
{
    switch (static_cast<int>(label)) {
    case QFileDialog::Accept:
        return statusBar()->acceptButton()->text();
    case QFileDialog::Reject:
        return statusBar()->rejectButton()->text();
    default:
        break;
    }

    return QString();
}

void DFileDialog::setOptions(QFileDialog::Options options)
{
    Q_D(DFileDialog);

    d->options = options;

    getFileView()->model()->setReadOnly(options.testFlag(QFileDialog::ReadOnly));

    if (options.testFlag(QFileDialog::ShowDirsOnly)) {
        getFileView()->setFilters(getFileView()->filters() & ~QDir::Files & ~QDir::Drives);
    }
}

void DFileDialog::setOption(QFileDialog::Option option, bool on)
{
    Q_D(DFileDialog);

    QFileDialog::Options options = d->options;

    if (on) {
        options |= option;
    } else {
        options &= ~option;
    }

    setOptions(options);
}

bool DFileDialog::testOption(QFileDialog::Option option) const
{
    Q_D(const DFileDialog);

    return d->options.testFlag(option);
}

QFileDialog::Options DFileDialog::options() const
{
    Q_D(const DFileDialog);

    return d->options;
}

void DFileDialog::setCurrentInputName(const QString &name)
{
    if (!statusBar()->lineEdit()) {
        return;
    }

    statusBar()->lineEdit()->setText(name);

    QMimeDatabase db;

    const QString &suffix = db.suffixForFileName(name);

    if (suffix.isEmpty()) {
        statusBar()->lineEdit()->selectAll();
    } else {
        statusBar()->lineEdit()->setSelection(0, name.length() - suffix.length() - 1);
    }
}

void DFileDialog::addCustomWidget(CustomWidgetType type, const QString &data)
{
    const QJsonDocument &json = QJsonDocument::fromJson(data.toUtf8());
    const QJsonObject &object = json.object();

    if (type == LineEditType) {
        int maxLength = object["maxLength"].toInt();
        QLineEdit::EchoMode echoMode = static_cast<QLineEdit::EchoMode>(object["echoMode"].toInt());
        QString inputMask = object["inputMask"].toString();
        QLabel *label = new QLabel(object["text"].toString());
        QLineEdit *edit = new QLineEdit(object["defaultValue"].toString());

        if (maxLength > 0) {
            edit->setMaxLength(maxLength);
        }

        if (!inputMask.isEmpty()) {
            edit->setInputMask(inputMask);
        }

        edit->setEchoMode(echoMode);
        edit->setPlaceholderText(object["placeholderText"].toString());
        edit->setFixedHeight(24);
        statusBar()->addLineEdit(label, edit);
    } else {
        QStringList data;

        for (const QVariant &v : object["data"].toArray().toVariantList()) {
            data << v.toString();
        }

        QString defaultValue = object["defaultValue"].toString();

        QLabel *label = new QLabel(object["text"].toString());
        QComboBox *comboBox = new QComboBox();

        comboBox->setEditable(object["editable"].toBool());
        comboBox->addItems(data);

        if (!defaultValue.isEmpty()) {
            comboBox->setCurrentText(defaultValue);
        }

        comboBox->setFixedHeight(24);

        statusBar()->addComboBox(label, comboBox);
    }
}

void DFileDialog::beginAddCustomWidget()
{
    statusBar()->beginAddCustomWidget();
}

void DFileDialog::endAddCustomWidget()
{
    statusBar()->endAddCustomWidget();
}

QVariant DFileDialog::getCustomWidgetValue(DFileDialog::CustomWidgetType type, const QString &text) const
{
    if (type == LineEditType) {
        return statusBar()->getLineEditValue(text);
    } else if (type == ComboBoxType) {
        return statusBar()->getComboBoxValue(text);
    }

    return QVariant();
}

QVariantMap DFileDialog::allCustomWidgetsValue(DFileDialog::CustomWidgetType type) const
{
    if (type == LineEditType) {
        return statusBar()->allLineEditsValue();
    } else if (type == ComboBoxType) {
        return statusBar()->allComboBoxsValue();
    }

    return QVariantMap();
}

void DFileDialog::setHideOnAccept(bool enable)
{
    D_D(DFileDialog);

    d->hideOnAccept = enable;
}

bool DFileDialog::hideOnAccept() const
{
    D_DC(DFileDialog);

    return d->hideOnAccept;
}

DFileView *DFileDialog::getFileView() const
{
    Q_D(const DFileDialog);

    return d->view;
}

void DFileDialog::accept()
{
    done(QDialog::Accepted);
}

void DFileDialog::done(int r)
{
    D_D(DFileDialog);

    if (d->eventLoop) {
        d->eventLoop->exit(r);
    }

    if (r != QDialog::Accepted || d->hideOnAccept)
        hide();

    emit finished(r);
    if (r == QDialog::Accepted) {
        emit accepted();
    } else if (r == QDialog::Rejected) {
        emit rejected();
    }
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
    if (guard.isNull()) {
        return QDialog::Rejected;
    }
    d->eventLoop = 0;

    setAttribute(Qt::WA_ShowModal, wasShowModal);

    if (deleteOnClose) {
        delete this;
    }
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
    Q_D(DFileDialog);

    if (!event->spontaneous() && !testAttribute(Qt::WA_Moved)) {
        Qt::WindowStates  state = windowState();
        adjustPosition(parentWidget());
        setAttribute(Qt::WA_Moved, false); // not really an explicit position
        if (state != windowState()) {
            setWindowState(state);
        }
    }

    activateWindow();

    windowHandle()->installEventFilter(this);

    if (windowFlags().testFlag(Qt::WindowSystemMenuHint)) {
        overrideWindowFlags(windowFlags() & ~Qt::WindowSystemMenuHint);
    }

    return DFileManagerWindow::showEvent(event);
}

void DFileDialog::closeEvent(QCloseEvent *event)
{
#ifndef QT_NO_WHATSTHIS
    if (isModal() && QWhatsThis::inWhatsThisMode()) {
        QWhatsThis::leaveWhatsThisMode();
    }
#endif
    if (isVisible()) {
        QPointer<QObject> that = this;
        reject();
        if (that && isVisible()) {
            event->ignore();
        }
    } else {
        event->accept();
    }

    return DFileManagerWindow::closeEvent(event);
}

bool DFileDialog::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DFileDialog);
    if (watched == windowHandle() && event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        if (e->modifiers() == Qt::ControlModifier
                && (e->key() == Qt::Key_T
                    || e->key() == Qt::Key_W)) {
            return true;
        } else if (e->modifiers() == Qt::NoModifier || e->modifiers() == Qt::KeypadModifier) {
            if (e == QKeySequence::Cancel) {
                DFileView *fileView = d->view;
                if (fileView) {
                    if (fileView->state() == 3) {
                        fileView->closePersistentEditor(fileView->currentIndex());
                        return true;
                    }
                }
                close();
            } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
                handleEnterPressed();
            }
        }
    }

    return DFileManagerWindow::eventFilter(watched, event);
}

void DFileDialog::adjustPosition(QWidget *w)
{
    if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme())
        if (theme->themeHint(QPlatformTheme::WindowAutoPlacement).toBool()) {
            return;
        }
    QPoint p(0, 0);
    int extraw = 0, extrah = 0, scrn = 0;
    if (w) {
        w = w->window();
    }
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
        QWidget *current = list.at(i);
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


    if (w && (w->windowFlags() | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint) != w->windowFlags()) {
        // Use pos() if the widget is embedded into a native window
        QPoint pp;
        if (w->windowHandle() && w->windowHandle()->property("_q_embedded_native_parent_handle").value<WId>()) {
            pp = w->pos();
        } else {
            pp = w->mapToGlobal(QPoint(0, 0));
        }
        p = QPoint(pp.x() + w->width() / 2,
                   pp.y() + w->height() / 2);
    } else {
        // p = middle of the desktop
        p = QPoint(desk.x() + desk.width() / 2, desk.y() + desk.height() / 2);
    }

    // p = origin of this
    p = QPoint(p.x() - width() / 2 - extraw,
               p.y() - height() / 2 - extrah);


    if (p.x() + extraw + width() > desk.x() + desk.width()) {
        p.setX(desk.x() + desk.width() - width() - extraw);
    }
    if (p.x() < desk.x()) {
        p.setX(desk.x());
    }

    if (p.y() + extrah + height() > desk.y() + desk.height()) {
        p.setY(desk.y() + desk.height() - height() - extrah);
    }
    if (p.y() < desk.y()) {
        p.setY(desk.y());
    }

    move(p);
}

bool DFileDialog::fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target, QVariant *resultData)
{
    Q_UNUSED(target)
    Q_UNUSED(resultData)

    if (!isActiveWindow()) {
        return false;
    }

    if (event->type() == DFMEvent::OpenFile) {
        onAcceptButtonClicked();

        return true;
    }

    switch (event->type()) {
    case DFMEvent::OpenFile:
    case DFMEvent::OpenFileByApp:
    case DFMEvent::CompressFiles:
    case DFMEvent::DecompressFile:
    case DFMEvent::DecompressFileHere:
//    case DFMEvent::DeleteFiles:
//    case DFMEvent::RestoreFromTrash:
    case DFMEvent::OpenFileLocation:
    case DFMEvent::CreateSymlink:
    case DFMEvent::FileShare:
    case DFMEvent::CancelFileShare:
    case DFMEvent::OpenInTerminal:
        return true;
    default:
        break;
    }

    return false;
}

void DFileDialog::handleNewView(DFMBaseView *view)
{
    Q_D(DFileDialog);

    DFileView *fileView = qobject_cast<DFileView *>(view->widget());

    if (!fileView) {
        // sava data
        d->currentNameFilterIndex = selectedNameFilterIndex();
        d->filters = filter();
        d->currentInputName = statusBar()->lineEdit()->text();

        return;
    }

    d->view = fileView;

    QSet<DFMGlobal::MenuAction> whitelist;

    whitelist << DFMGlobal::NewFolder << DFMGlobal::NewDocument << DFMGlobal::DisplayAs
              << DFMGlobal::SortBy << DFMGlobal::Open << DFMGlobal::Rename << DFMGlobal::Delete
              << DFMGlobal::ListView << DFMGlobal::IconView << DFMGlobal::ExtendView << DFMGlobal::NewWord
              << DFMGlobal::NewExcel << DFMGlobal::NewPowerpoint << DFMGlobal::NewText << DFMGlobal::Name
              << DFMGlobal::Size << DFMGlobal::Type << DFMGlobal::CreatedDate << DFMGlobal::LastModifiedDate
              << DFMGlobal::DeletionDate << DFMGlobal::SourcePath << DFMGlobal::AbsolutePath << DFMGlobal::Copy
              << DFMGlobal::Paste << DFMGlobal::Cut;

    fileView->setMenuActionWhitelist(whitelist);
    fileView->setDragEnabled(false);
    fileView->setDragDropMode(QAbstractItemView::NoDragDrop);

    connect(fileView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &DFileDialog::selectionFilesChanged);
    connect(fileView, &DFileView::rootUrlChanged,
            this, &DFileDialog::currentUrlChanged);

    connect(fileView, static_cast<void (DFileView::*)(const QModelIndex &)>(&DFileView::currentChanged),
    this, [this, fileView] {
        Q_D(const DFileDialog);

        if (d->acceptMode != QFileDialog::AcceptSave)
        {
            return;
        }

        const QModelIndex &index = fileView->currentIndex();

        const DAbstractFileInfoPointer &fileInfo = fileView->model()->fileInfo(index);

        if (!fileInfo)
            return;

        if (fileInfo->isFile())
            setCurrentInputName(fileInfo->fileName());
    });

    if (!d->nameFilters.isEmpty()) {
        setNameFilters(d->nameFilters);
    }

    if (d->filters != 0) {
        setFilter(d->filters);
    }

    if (d->currentNameFilterIndex >= 0) {
        selectNameFilterByIndex(d->currentNameFilterIndex);
    }

    if (!d->currentInputName.isEmpty()) {
        setCurrentInputName(d->currentInputName);
    }
}

FileDialogStatusBar *DFileDialog::statusBar() const
{
    D_DC(DFileDialog);

    return d->statusBar;
}

#if DTK_VERSION > DTK_VERSION_CHECK(2, 0, 5, 0)
static bool pwPluginVersionGreaterThen(const QString &v)
{
    const QStringList &version_list = DPlatformWindowHandle::pluginVersion().split(".");
    const QStringList &v_v_list = v.split(".");

    for (int i = 0; i < version_list.count(); ++i) {
        if (v.count() <= i)
            return true;

        if (version_list[i].toInt() > v_v_list[i].toInt())
            return true;
    }

    return false;
}
#endif

void DFileDialog::onAcceptButtonClicked()
{
    D_DC(DFileDialog);

    if (d->acceptMode == QFileDialog::AcceptSave) {
        if (!directoryUrl().isLocalFile()) {
            return;
        }

        if (!directory().exists()) {
            return;
        }

        const QString &file_name = statusBar()->lineEdit()->text();

        if (!file_name.isEmpty()) {
            if (!d->options.testFlag(QFileDialog::DontConfirmOverwrite)) {
                QFileInfo info(directory().absoluteFilePath(file_name));

                if (info.exists() || info.isSymLink()) {
                    DDialog dialog(this);

                    // NOTE(zccrs): dxcb bug
                    if (!DPlatformWindowHandle::isEnabledDXcb(this)
#if DTK_VERSION > DTK_VERSION_CHECK(2, 0, 5, 0)
                            || pwPluginVersionGreaterThen("1.1.8.3")
#endif
                       ) {
                        dialog.setWindowModality(Qt::WindowModal);
                    }

                    dialog.setIcon(QIcon::fromTheme("dialog-warning"), QSize(64, 64));
                    dialog.setTitle(tr("%1 already exists, do you want to replace it?").arg(file_name));
                    dialog.addButton(tr("Cancel"), true);
                    dialog.addButton(tr("Replace"), false, DDialog::ButtonWarning);

                    if (dialog.exec() != DDialog::Accepted) {
                        return;
                    }
                }
            }

            accept();
        }

        return;
    }

    const DUrlList &urls = getFileView()->selectedUrls();

    switch (d->fileMode) {
    case QFileDialog::AnyFile:
    case QFileDialog::ExistingFile:
        if (urls.count() == 1) {
            const DAbstractFileInfoPointer &fileInfo = getFileView()->model()->fileInfo(urls.first());

            if (fileInfo->isDir()) {
                getFileView()->cd(urls.first());
            } else {
                accept();
            }
        }
        break;
    case QFileDialog::ExistingFiles: {

        bool doCdWhenPossible = urls.count() == 1;

        for (const DUrl &url : urls) {
            const DAbstractFileInfoPointer &fileInfo = getFileView()->model()->fileInfo(url);

            if (!fileInfo->isFile()) {
                if (doCdWhenPossible && fileInfo->isDir()) {
                    // blumia: it's possible to select more than one file/dirs, we only do cd when select a single directory.
                    getFileView()->cd(urls.first());
                }
                return;
            }
        }

        if (!urls.isEmpty()) {
            accept();
        }
        break;
    }
    default:
        for (const DUrl &url : urls) {
            const DAbstractFileInfoPointer &fileInfo = getFileView()->model()->fileInfo(url);

            if (!fileInfo->isDir()) {
                return;
            }
        }
        accept();
        break;
    }
}

void DFileDialog::onRejectButtonClicked()
{
    reject();
}

void DFileDialog::onCurrentInputNameChanged()
{
    Q_D(DFileDialog);

    d->currentInputName = statusBar()->lineEdit()->text();
    statusBar()->acceptButton()->setDisabled(d->currentInputName.isEmpty());
}

void DFileDialog::handleEnterPressed()
{
    if (!qobject_cast<DFMAddressBar*>(qApp->focusWidget())) {
        for (const QModelIndex &index : getFileView()->selectedIndexes()) {
            const DAbstractFileInfoPointer &info = getFileView()->model()->fileInfo(index);
            if (info->isDir()) {
                return;
            }
        }
        statusBar()->acceptButton()->animateClick();
    }
}
