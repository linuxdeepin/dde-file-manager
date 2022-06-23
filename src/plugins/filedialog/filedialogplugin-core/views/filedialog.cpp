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
#include "filedialog.h"
#include "filedialog_p.h"
#include "events/coreeventscaller.h"
#include "utils/corehelper.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/common/delegate/delegateservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/mimetype/dmimedatabase.h"

#include <DDialog>
#include <DPlatformWindowHandle>
#include <DTitlebar>
#include <DWidgetUtil>

#include <QApplication>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QPointer>
#include <QPushButton>
#include <QComboBox>
#include <QWindow>
#include <QShowEvent>
#include <QDesktopWidget>
#include <QWhatsThis>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonArray>

#include <qpa/qplatformtheme.h>
#include <qpa/qplatformdialoghelper.h>

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DIALOGCORE_USE_NAMESPACE

FileDialogPrivate::FileDialogPrivate(FileDialog *qq)
    : QObject(nullptr),
      q(qq)
{
}

void FileDialogPrivate::handleSaveAcceptBtnClicked()
{
    if (acceptCanOpenOnSave) {
        auto &&urls = WorkspaceService::service()->selectedUrls(q->internalWinId());
        if (!urls.isEmpty())
            q->cd(urls.first());
        return;
    }

    if (!q->directoryUrl().isLocalFile())
        return;

    if (!q->directory().exists())
        return;

    QString fileName = q->statusBar()->lineEdit()->text();   //文件名
    // TODO(zhangs): 检查是否要补充后缀
    if (!fileName.isEmpty()) {
        if (fileName.startsWith(".") && CoreHelper::askHiddenFile(q))
            return;
        if (!options.testFlag(QFileDialog::DontConfirmOverwrite)) {
            QFileInfo info(q->directory().absoluteFilePath(fileName));
            if ((info.exists() || info.isSymLink()) && CoreHelper::askReplaceFile(fileName, q))
                return;
        }

        q->accept();
    }
}

void FileDialogPrivate::handleOpenAcceptBtnClicked()
{
    QList<QUrl> urls { WorkspaceService::service()->selectedUrls(q->internalWinId()) };
    CoreHelper::urlTransform(&urls);

    switch (fileMode) {
    case QFileDialog::AnyFile:
    case QFileDialog::ExistingFile:
        if (urls.count() == 1) {
            auto fileInfo = InfoFactory::create<AbstractFileInfo>(urls.first());
            if (fileInfo->isDir()) {
                q->cd(urls.first());
            } else {
                q->accept();
            }
        }
        break;
    case QFileDialog::ExistingFiles: {
        bool doCdWhenPossible = urls.count() == 1;
        for (const QUrl &url : urls) {
            auto fileInfo = InfoFactory::create<AbstractFileInfo>(url);
            if (!fileInfo)
                continue;

            if (!fileInfo->isFile() && !allowMixedSelection) {
                if (doCdWhenPossible && fileInfo->isDir()) {
                    // blumia: it's possible to select more than one file/dirs, we only do cd when select a single directory.
                    q->cd(urls.first());
                }
                return;
            }
        }

        if (!urls.isEmpty()) {
            q->accept();
        }
        break;
    }
    default: {
        for (const QUrl &url : urls) {
            auto fileInfo = InfoFactory::create<AbstractFileInfo>(url);

            if (!fileInfo->isDir())
                return;
        }
        q->accept();
        break;
    }
    }
}

void FileDialogPrivate::handleOpenNewWindow(const QUrl &url)
{
    if (url.isValid() && !url.isEmpty() && !UniversalUtils::urlEquals(url, q->currentUrl()))
        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, q->internalWinId(), url);
}

/*!
 * \class FileDialog
 */

FileDialog::FileDialog(const QUrl &url, QWidget *parent)
    : FileManagerWindow(url, parent),
      d(new FileDialogPrivate(this))
{
    initializeUi();
    initConnect();
    initEventsConnect();
    initEventsFilter();
}

FileDialog::~FileDialog()
{
    dpfSignalDispatcher->unsubscribe(Workspace::EventType::kViewSelectionChanged, this,
                                     &FileDialog::onViewSelectionChanged);
    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_RenameStartEdit", this, &FileDialog::handleRenameStartAcceptBtn);
    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_RenameEndEdit", this, &FileDialog::handleRenameEndAcceptBtn);

    dpfSignalDispatcher->removeGlobalEventFilter(this);
}

void FileDialog::cd(const QUrl &url)
{
    DSB_FM_USE_NAMESPACE
    FileManagerWindow::cd(url);

    auto window = WindowsService::service()->findWindowById(this->internalWinId());
    if (!window)
        return;

    if (window->workSpace())
        handleUrlChanged(url);
    else
        connect(window, &FileManagerWindow::workspaceInstallFinished, this, [this, url] { handleUrlChanged(url); }, Qt::DirectConnection);
}

bool FileDialog::saveClosedSate() const
{
    return false;
}

void FileDialog::updateAsDefaultSize()
{
    resize(d->kDefaultWindowWidth, d->kDefaultWindowHeight);
}

QFileDialog::ViewMode FileDialog::currentViewMode() const
{
    auto mode = WorkspaceService::service()->currentViewMode(internalWinId());
    if (mode == Global::ViewMode::kListMode)
        return QFileDialog::Detail;

    return QFileDialog::List;
}

void FileDialog::setDirectory(const QString &directory)
{
    setDirectoryUrl(UrlRoute::fromLocalFile(directory));
}

void FileDialog::setDirectory(const QDir &directory)
{
    setDirectoryUrl(UrlRoute::fromLocalFile(directory.absolutePath()));
}

QDir FileDialog::directory() const
{
    return QDir(directoryUrl().toLocalFile());
}

void FileDialog::setDirectoryUrl(const QUrl &directory)
{
    cd(directory);
}

QUrl FileDialog::directoryUrl() const
{
    QUrl url { currentUrl() };
    if (DelegateService::service()->isRegisterUrlTransform(url.scheme()))
        url = DelegateService::service()->urlTransform(url);

    return url;
}

void FileDialog::selectFile(const QString &filename)
{
    QUrl url { currentUrl() };
    QDir dir(url.path());
    url.setPath(dir.absoluteFilePath(filename));
    selectUrl(url);
}

QStringList FileDialog::selectedFiles() const
{
    QStringList list;

    for (const QUrl &url : selectedUrls()) {
        QUrl fileUrl(url);
        list << fileUrl.toLocalFile();
    }

    return list;
}

void FileDialog::selectUrl(const QUrl &url)
{
    if (!d->isFileView)
        return;

    CoreEventsCaller::sendSelectFiles(this->internalWinId(), { url });
    setCurrentInputName(QFileInfo(url.path()).fileName());
}

QList<QUrl> FileDialog::selectedUrls() const
{
    if (!d->isFileView)
        return {};
    // TODO(zhangs): orderedSelectedUrls
    QList<QUrl> list { WorkspaceService::service()->selectedUrls(internalWinId()) };
    QList<QUrl>::iterator begin = list.begin();
    while (begin != list.end()) {
        QUrl curUrl { *begin };
        if (DelegateService::service()->isRegisterUrlTransform(curUrl.scheme()))
            curUrl = DelegateService::service()->urlTransform(curUrl);

        if (!curUrl.isEmpty() && curUrl.isValid())
            *begin = curUrl;

        ++begin;
    }

    if (d->acceptMode == QFileDialog::AcceptSave) {
        QUrl fileUrl = list.isEmpty() ? currentUrl() : list.first();
        auto fileInfo = InfoFactory::create<AbstractFileInfo>(fileUrl);

        if (fileInfo) {
            if (list.isEmpty()) {
                fileUrl = fileInfo->getUrlByChildFileName(statusBar()->lineEdit()->text());
            } else {
                fileUrl = fileInfo->getUrlByNewFileName(statusBar()->lineEdit()->text());
            }
        }

        return QList<QUrl>() << fileUrl;
    }

    if (list.isEmpty() && (d->fileMode == QFileDialog::Directory || d->fileMode == QFileDialog::DirectoryOnly)) {
        if (directoryUrl().isLocalFile())
            list << QUrl(directoryUrl());
    }
    return list;
}

void FileDialog::setNameFilters(const QStringList &filters)
{
    d->nameFilters = filters;

    if (testOption(QFileDialog::HideNameFilterDetails)) {
        statusBar()->setComBoxItems(CoreHelper::stripFilters(filters));
    } else {
        statusBar()->setComBoxItems(filters);
    }

    if (modelCurrentNameFilter().isEmpty())
        selectNameFilter(filters.isEmpty() ? QString() : filters.first());
}

QStringList FileDialog::nameFilters() const
{
    return d->nameFilters;
}

void FileDialog::setFileMode(QFileDialog::FileMode mode)
{
    if (!d->isFileView)
        return;

    if (d->fileMode == QFileDialog::DirectoryOnly
        || d->fileMode == QFileDialog::Directory) {
        // 清理只显示目录时对文件名添加的过滤条件
        // getFileView()->setNameFilters(QStringList());
        dpfSignalDispatcher->publish(Workspace::EventType::kSetNameFilter, internalWinId(), QStringList());
    }

    d->fileMode = mode;
    updateAcceptButtonState();

    switch (static_cast<int>(mode)) {
    case QFileDialog::ExistingFiles:
        CoreEventsCaller::setEnabledSelectionModes(this, { QAbstractItemView::ExtendedSelection });
        break;
    case QFileDialog::DirectoryOnly:
    case QFileDialog::Directory:
        // 文件名中不可能包含 '/', 此处目的是过滤掉所有文件
        dpfSignalDispatcher->publish(Workspace::EventType::kSetNameFilter, internalWinId(), QStringList("/"));
        urlSchemeEnable("recent", false);
        // fall through
        [[fallthrough]];
    default:
        CoreEventsCaller::setEnabledSelectionModes(this, { QAbstractItemView::SingleSelection });
        break;
    }
}

void FileDialog::setAllowMixedSelection(bool on)
{
    d->allowMixedSelection = on;
}

void FileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    if (!d->isFileView)
        return;

    d->acceptMode = mode;
    updateAcceptButtonState();

    if (mode == QFileDialog::AcceptOpen) {
        statusBar()->setMode(FileDialogStatusBar::kOpen);
        setFileMode(d->fileMode);
        urlSchemeEnable("recent", true);

        disconnect(statusBar()->lineEdit(), &QLineEdit::textChanged,
                   this, &FileDialog::onCurrentInputNameChanged);
    } else {
        statusBar()->setMode(FileDialogStatusBar::kSave);
        CoreEventsCaller::setSelectionMode(this, QAbstractItemView::SingleSelection);
        urlSchemeEnable("recent", false);
        setFileMode(QFileDialog::DirectoryOnly);

        connect(statusBar()->lineEdit(), &QLineEdit::textChanged,
                this, &FileDialog::onCurrentInputNameChanged);
    }
}

QFileDialog::AcceptMode FileDialog::acceptMode() const
{
    return d->acceptMode;
}

void FileDialog::setLabelText(QFileDialog::DialogLabel label, const QString &text)
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

QString FileDialog::labelText(QFileDialog::DialogLabel label) const
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

void FileDialog::setOptions(QFileDialog::Options options)
{
    if (!d->isFileView)
        return;

    // （此处修改比较特殊，临时方案）与产品沟通后，使用uos文管保存框保存文件时，如果当前目录下有同名文件，
    // 必须要弹出提示框 “是否覆盖重名文件”。
    // 所以options中的QFileDialog::DontConfirmOverwrite标志位将失去意义，
    // 所以此处直接将该标志位赋值为0。
    options &= ~QFileDialog::DontConfirmOverwrite;

    d->options = options;

    dpfSignalDispatcher->publish(Workspace::EventType::kSetReadOnly,
                                 internalWinId(), options.testFlag(QFileDialog::ReadOnly));

    if (options.testFlag(QFileDialog::ShowDirsOnly)) {
        // TODO(liuyangming):
        // getFileView()->setFilters(getFileView()->filters() & ~QDir::Files & ~QDir::Drives);
    }
}

void FileDialog::setOption(QFileDialog::Option option, bool on)
{
    QFileDialog::Options options = d->options;

    if (on) {
        options |= option;
    } else {
        options &= ~option;
    }

    setOptions(options);
}

bool FileDialog::testOption(QFileDialog::Option option) const
{
    return d->options.testFlag(option);
}

QFileDialog::Options FileDialog::options() const
{
    return d->options;
}

void FileDialog::urlSchemeEnable(const QString &scheme, bool enable)
{
    QUrl url;
    url.setScheme(scheme);
    url.setPath("/");
    CoreEventsCaller::setSidebarItemVisible(url, enable);
}

void FileDialog::setCurrentInputName(const QString &name)
{
    if (!statusBar()->lineEdit()) {
        return;
    }

    statusBar()->lineEdit()->setText(name);

    DFMBASE_NAMESPACE::DMimeDatabase db;

    const QString &suffix = db.suffixForFileName(name);

    if (suffix.isEmpty()) {
        statusBar()->lineEdit()->selectAll();
    } else {
        statusBar()->lineEdit()->setSelection(0, name.length() - suffix.length() - 1);
    }
}

void FileDialog::addCustomWidget(FileDialog::CustomWidgetType type, const QString &data)
{
    const QJsonDocument &json = QJsonDocument::fromJson(data.toUtf8());
    const QJsonObject &object = json.object();

    if (type == kLineEditType) {
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

        statusBar()->addComboBox(label, comboBox);
    }
}

QVariant FileDialog::getCustomWidgetValue(FileDialog::CustomWidgetType type, const QString &text) const
{
    if (type == kLineEditType) {
        return statusBar()->getLineEditValue(text);
    } else if (type == kComboBoxType) {
        return statusBar()->getComboBoxValue(text);
    }

    return QVariant();
}

QVariantMap FileDialog::allCustomWidgetsValue(FileDialog::CustomWidgetType type) const
{
    if (type == kLineEditType) {
        return statusBar()->allLineEditsValue();
    } else if (type == kComboBoxType) {
        return statusBar()->allComboBoxsValue();
    }

    return QVariantMap();
}

void FileDialog::beginAddCustomWidget()
{
    statusBar()->beginAddCustomWidget();
}

void FileDialog::endAddCustomWidget()
{
    statusBar()->endAddCustomWidget();
}

void FileDialog::setHideOnAccept(bool enable)
{
    d->hideOnAccept = enable;
}

bool FileDialog::hideOnAccept() const
{
    return d->hideOnAccept;
}

void FileDialog::accept()
{
    done(QDialog::Accepted);
}

void FileDialog::done(int r)
{
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

int FileDialog::exec()
{
    if (d->eventLoop) {
        qWarning("DFileDialog::exec: Recursive call detected");
        return -1;
    }

    bool deleteOnClose = testAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_DeleteOnClose, false);

    bool wasShowModal = testAttribute(Qt::WA_ShowModal);
    setAttribute(Qt::WA_ShowModal, true);

    show();

    QPointer<FileDialog> guard { this };
    QEventLoop eventLoop;
    d->eventLoop = &eventLoop;
    int res = eventLoop.exec(QEventLoop::DialogExec);
    if (guard.isNull()) {
        return QDialog::Rejected;
    }
    d->eventLoop = nullptr;

    setAttribute(Qt::WA_ShowModal, wasShowModal);

    if (deleteOnClose) {
        delete this;
    }
    return res;
}

void FileDialog::open()
{
    show();
}

void FileDialog::reject()
{
    done(QDialog::Rejected);
}

void FileDialog::onAcceptButtonClicked()
{
    if (!d->isFileView)
        return;
    if (selectedUrls().isEmpty())
        return;

    if (d->acceptMode == QFileDialog::AcceptSave)
        d->handleSaveAcceptBtnClicked();
    else
        d->handleOpenAcceptBtnClicked();
}

void FileDialog::onRejectButtonClicked()
{
    reject();
}

void FileDialog::onCurrentInputNameChanged()
{
    if (!d->isFileView)
        return;
    // TODO(zhangs):
    updateAcceptButtonState();
}

void FileDialog::selectNameFilter(const QString &filter)
{
    QString key;

    if (testOption(QFileDialog::HideNameFilterDetails)) {
        key = CoreHelper::stripFilters(QStringList(filter)).first();
    } else {
        key = filter;
    }

    int index = statusBar()->comboBox()->findText(key);

    selectNameFilterByIndex(index);
}

QString FileDialog::selectedNameFilter() const
{
    const QComboBox *box = statusBar()->comboBox();

    return box ? d->nameFilters.value(box->currentIndex()) : QString();
}

void FileDialog::selectNameFilterByIndex(int index)
{
    if (index < 0 || index >= statusBar()->comboBox()->count() || !d->isFileView)
        return;

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
        DFMBASE_NAMESPACE::DMimeDatabase db;
        QString fileName = statusBar()->lineEdit()->text();
        const QString fileNameExtension = db.suffixForFileName(fileName);
        QString newNameFilterExtension { CoreHelper::findExtensioName(fileName, newNameFilters, &db) };

        if (!newNameFilters.isEmpty())
            newNameFilterExtension = db.suffixForFileName(newNameFilters.at(0));
        if (!fileNameExtension.isEmpty() && !newNameFilterExtension.isEmpty()) {
            const int fileNameExtensionLength = fileNameExtension.count();
            fileName.replace(fileName.count() - fileNameExtensionLength,
                             fileNameExtensionLength, newNameFilterExtension);
            setCurrentInputName(fileName);
        } else if (fileNameExtension.isEmpty() && !fileName.isEmpty() && !newNameFilterExtension.isEmpty()) {
            fileName.append('.' + newNameFilterExtension);
            setCurrentInputName(fileName);
        }
    }

    // when d->fileMode == QFileDialog::DirectoryOnly or d->fileMode == QFileDialog::Directory
    // we use setNameFilters("/") to filter files (can't select file, select dir is ok)
    if ((d->fileMode == QFileDialog::DirectoryOnly || d->fileMode == QFileDialog::Directory) && QStringList("/") != newNameFilters)
        newNameFilters = QStringList("/");

    dpfSlotChannel->push("dfmplugin_workspace", "slot_SetNameFilter", internalWinId(), newNameFilters);
}

int FileDialog::selectedNameFilterIndex() const
{
    const QComboBox *box = statusBar()->comboBox();

    return box ? box->currentIndex() : -1;
}

QDir::Filters FileDialog::filter() const
{
    // TODO(liuyangming):
    // return getFileView() ? getFileView()->filters() : QDir::Filters();

    return {};
}

void FileDialog::setFilter(QDir::Filters filters)
{
    dpfSignalDispatcher->publish(Workspace::EventType::kSetViewFilter, internalWinId(), filters);
}

void FileDialog::updateAcceptButtonState()
{
    if (!d->isFileView)
        return;
    QUrl url = currentUrl();
    auto fileInfo = InfoFactory::create<AbstractFileInfo>(url);
    if (!fileInfo)
        return;

    bool isDirMode = d->fileMode == QFileDialog::Directory || d->fileMode == QFileDialog::DirectoryOnly;
    bool dialogShowMode = d->acceptMode;
    bool isVirtual = UrlRoute::isVirtual(fileInfo->url().scheme());
    if (dialogShowMode == QFileDialog::AcceptOpen) {
        auto size = WorkspaceService::service()->selectedUrls(internalWinId()).size();
        bool isSelectFiles = size > 0;
        // 1.打开目录（非虚拟目录） 2.打开文件（选中文件）
        statusBar()->acceptButton()->setDisabled((isDirMode && isVirtual) || (!isDirMode && !isSelectFiles));
        return;
    }

    if (dialogShowMode == QFileDialog::AcceptSave) {
        statusBar()->acceptButton()->setDisabled(isVirtual || statusBar()->lineEdit()->text().trimmed().isEmpty());
        return;
    }
}

void FileDialog::handleEnterPressed()
{
    if (!statusBar()->acceptButton()->isEnabled() || !d->isFileView)
        return;

    // TODO(zhangs): titlebar edit status
    bool exit { false };
    auto &&urls = WorkspaceService::service()->selectedUrls(internalWinId());
    for (const QUrl &url : urls) {
        auto info = InfoFactory::create<AbstractFileInfo>(url);
        if (!info || info->isDir()) {
            exit = true;
            break;
        }
    }

    if (!exit)
        statusBar()->acceptButton()->animateClick();
}

void FileDialog::handleUrlChanged(const QUrl &url)
{
    QString scheme { url.scheme() };

    DSB_FM_USE_NAMESPACE
    d->lastIsFileView = d->isFileView;
    d->isFileView = WorkspaceService::service()->schemeViewIsFileView(scheme);

    // init accept mode, worskapce must initialized
    bool isFirst { false };
    static std::once_flag flag;
    std::call_once(flag, [this, &isFirst]() {
        isFirst = true;
        setAcceptMode(QFileDialog::AcceptOpen);
        updateViewState();
    });

    // view changed
    if (!isFirst && (d->lastIsFileView != d->isFileView))
        updateViewState();   // old: handleNewView

    updateAcceptButtonState();

    if (d->acceptMode == QFileDialog::AcceptSave) {
        setLabelText(QFileDialog::Accept, tr("Save", "button"));
        d->acceptCanOpenOnSave = false;
        onCurrentInputNameChanged();
    }
    emit initialized();
}

void FileDialog::onViewSelectionChanged(const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    if (windowID == internalWinId()) {
        emit selectionFilesChanged();
        updateAcceptButtonState();
    }
}

void FileDialog::handleRenameStartAcceptBtn(const quint64 windowID, const QUrl &url)
{
    Q_UNUSED(url)
    if (windowID == internalWinId()) {
        statusBar()->acceptButton()->setEnabled(false);
    }
}

void FileDialog::handleRenameEndAcceptBtn(const quint64 windowID, const QUrl &url)
{
    Q_UNUSED(url)
    if (windowID == internalWinId()) {
        statusBar()->acceptButton()->setEnabled(true);
    }
}

void FileDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous() && !testAttribute(Qt::WA_Moved)) {
        Qt::WindowStates state = windowState();
        adjustPosition(parentWidget());
        setAttribute(Qt::WA_Moved, false);   // not really an explicit position
        if (state != windowState()) {
            setWindowState(state);
        }
    }

    windowHandle()->installEventFilter(this);

    windowHandle()->installEventFilter(this);

    if (windowFlags().testFlag(Qt::WindowSystemMenuHint)) {
        overrideWindowFlags(windowFlags() & ~Qt::WindowSystemMenuHint);
    }

    const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(currentUrl());
    if (info)
        setWindowTitle(info->fileDisplayName());

    return FileManagerWindow::showEvent(event);
}

void FileDialog::closeEvent(QCloseEvent *event)
{
#ifndef QT_NO_WHATSTHIS
    if (isModal() && QWhatsThis::inWhatsThisMode()) {
        QWhatsThis::leaveWhatsThisMode();
    }
#endif
    if (isVisible()) {
        QPointer<QObject> that = this;
        if (that) {
            reject();
            if (isVisible())
                event->ignore();
        }
    } else {
        event->accept();
    }
    return FileManagerWindow::closeEvent(event);
}

bool FileDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == windowHandle() && event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        if (e->modifiers() == Qt::ControlModifier
            && (e->key() == Qt::Key_T
                || e->key() == Qt::Key_W)) {
            return true;
        } else if (e->modifiers() == Qt::NoModifier || e->modifiers() == Qt::KeypadModifier) {
            if (e == QKeySequence::Cancel) {
                // TODO(liuyangming):
                //                DFileView *fileView = d->view;
                //                if (fileView) {
                //                    if (fileView->state() == 3) {
                //                        fileView->closePersistentEditor(fileView->currentIndex());
                //                        return true;
                //                    }
                //                }
                close();
            } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
                handleEnterPressed();
            }
        }
    }

    if (watched == windowHandle() && event->type() == QEvent::Show) {
        emit windowShowed();
    }

    return FileManagerWindow::eventFilter(watched, event);
}

void FileDialog::initializeUi()
{
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::Dialog);

    if (titlebar()) {
        titlebar()->setDisableFlags(Qt::WindowSystemMenuHint);
        titlebar()->setMenuVisible(false);
    }

    // init status bar
    d->statusBar = new FileDialogStatusBar(this);
    centralWidget()->layout()->addWidget(d->statusBar);
    statusBar()->lineEdit()->setMaxLength(NAME_MAX);

    // 修复bug-45176
    // 如果是wanyland平台，将弹出的文件框居中
    if (WindowUtils::isWayLand())
        Dtk::Widget::moveToCenter(this);

    CoreEventsCaller::setMenuDisbaled();
}

void FileDialog::initConnect()
{
    connect(statusBar()->acceptButton(), &QPushButton::clicked, this, &FileDialog::onAcceptButtonClicked);
    connect(statusBar()->rejectButton(), &QPushButton::clicked, this, &FileDialog::onRejectButtonClicked);
    connect(statusBar()->comboBox(),
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &FileDialog::selectNameFilter);
    connect(statusBar()->comboBox(),
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &FileDialog::selectedNameFilterChanged);
    connect(this, &FileDialog::selectionFilesChanged, &FileDialog::updateAcceptButtonState);
}

void FileDialog::initEventsConnect()
{
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_RenameStartEdit", this, &FileDialog::handleRenameStartAcceptBtn);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_RenameEndEdit", this, &FileDialog::handleRenameEndAcceptBtn);
}

void FileDialog::initEventsFilter()
{
    dpfSignalDispatcher->installGlobalEventFilter(this, [this](DPF_NAMESPACE::EventType type, const QVariantList &params) -> bool {
        if (type == GlobalEventType::kOpenFiles) {
            onAcceptButtonClicked();
            return true;
        }

        if (type == GlobalEventType::kOpenNewWindow && params.size() > 0) {
            d->handleOpenNewWindow(params.at(0).toUrl());
            return true;
        }

        static QList<DPF_NAMESPACE::EventType> filterTypeGroup { GlobalEventType::kOpenNewTab,
                                                                 GlobalEventType::kOpenAsAdmin,
                                                                 GlobalEventType::kOpenFilesByApp,
                                                                 GlobalEventType::kCreateSymlink,
                                                                 GlobalEventType::kOpenInTerminal,
                                                                 GlobalEventType::kHideFiles };
        if (filterTypeGroup.contains(type))
            return true;

        return false;
    });
}

/*!
 * \brief compiter -> fileview, fileview -> computerview
 * \param scheme
 */
void FileDialog::updateViewState()
{
    statusBar()->acceptButton()->setDisabled(!d->isFileView);

    if (!d->isFileView) {
        // sava data
        d->currentNameFilterIndex = selectedNameFilterIndex();
        d->filters = filter();
        d->currentInputName = statusBar()->lineEdit()->text();

        // TODO(zhangs);
        // d->orderedSelectedList.clear();
        return;
    }

    dpfSignalDispatcher->publish(Workspace::EventType::kSetViewDragEnabled, internalWinId(), false);
    dpfSignalDispatcher->publish(Workspace::EventType::kSetViewDragDropMode, internalWinId(), QAbstractItemView::NoDragDrop);

    // TODO(liuyangming): currentChanged
    dpfSignalDispatcher->subscribe(Workspace::EventType::kViewSelectionChanged, this,
                                   &FileDialog::onViewSelectionChanged);

    if (!d->nameFilters.isEmpty())
        setNameFilters(d->nameFilters);

    if (d->filters != 0)
        setFilter(d->filters);

    if (d->currentNameFilterIndex >= 0)
        selectNameFilterByIndex(d->currentNameFilterIndex);

    if (!d->currentInputName.isEmpty())
        setCurrentInputName(d->currentInputName);

    setFileMode(d->fileMode);
}

FileDialogStatusBar *FileDialog::statusBar() const
{
    return d->statusBar;
}

void FileDialog::adjustPosition(QWidget *w)
{
    // TODO(zhangs): QPlatformTheme
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

QString FileDialog::modelCurrentNameFilter() const
{
    if (!d->isFileView)
        return "";

    // TODO(liuyangming):
    QStringList filters;
    //  const QStringList &filters = getFileView()->nameFilters();

    if (filters.isEmpty()) {
        return QString();
    }

    return filters.first();
}
