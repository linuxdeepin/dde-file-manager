// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialog.h"
#include "filedialog_p.h"
#include "events/coreeventscaller.h"
#include "utils/corehelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/event/event.h>

#include <DDialog>
#include <DPlatformWindowHandle>
#include <DTitlebar>
#include <DWidgetUtil>
#include <DLineEdit>
#include <DLabel>
#include <DComboBox>
#include <DSuggestButton>
#include <DPushButton>

#include <QApplication>
#include <QVBoxLayout>
#include <QDialog>
#include <QPointer>
#include <QWindow>
#include <QShowEvent>
#include <QWhatsThis>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(QDir::Filters);

DFMBASE_USE_NAMESPACE
using namespace filedialog_core;

inline constexpr int kFileDialogDirectoryOnly { 4 };

FileDialogPrivate::FileDialogPrivate(FileDialog *qq)
    : QObject(nullptr),
      q(qq)
{
    //! fix: FileDialog no needs to restore window state on creating.
    //! see FileManagerWindowsManager::createWindow
    q->setProperty("_dfm_Disable_RestoreWindowState_", true);

    QSettings qtSets(QSettings::UserScope, QLatin1String("QtProject"));
    lastVisitedDir = qtSets.value("FileDialog/lastVisited").toUrl();

    delaySaveTimer = new QTimer(this);
    delaySaveTimer->setInterval(3000);
    connect(delaySaveTimer, &QTimer::timeout, this, &FileDialogPrivate::saveLastVisited);
}

FileDialogPrivate::~FileDialogPrivate()
{
    saveLastVisited();
}

void FileDialogPrivate::handleSaveAcceptBtnClicked()
{
    if (acceptCanOpenOnSave) {
        auto &&urls = CoreEventsCaller::sendGetSelectedFiles(q->internalWinId());
        if (!urls.isEmpty())
            q->cd(urls.first());
        return;
    }

    if (!q->directoryUrl().isLocalFile())
        return;

    if (!q->directory().exists())
        return;

    QString fileName = q->statusBar()->lineEdit()->text();   // 文件名
    // Check whether the suffix needs to be added
    QString suffix { "" };
    if (checkFileSuffix(fileName, suffix)) {
        fileName.append('.' + suffix);
        q->setCurrentInputName(fileName);
    }

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
    QList<QUrl> urls { CoreEventsCaller::sendGetSelectedFiles(q->internalWinId()) };

    QList<QUrl> urlsTrans {};
    bool ok = UniversalUtils::urlsTransformToLocal(urls, &urlsTrans);
    if (ok && !urlsTrans.isEmpty())
        urls = urlsTrans;

    switch (fileMode) {
    case QFileDialog::AnyFile:
    case QFileDialog::ExistingFile:
        if (urls.count() == 1) {
            auto fileInfo = InfoFactory::create<FileInfo>(urls.first());
            if (fileInfo->isAttributes(OptInfoType::kIsDir)) {
                q->cd(urls.first());
            } else {
                q->accept();
            }
        }
        break;
    case QFileDialog::ExistingFiles: {
        bool doCdWhenPossible = urls.count() == 1;
        for (const QUrl &url : urls) {
            auto fileInfo = InfoFactory::create<FileInfo>(url);
            if (!fileInfo)
                continue;

            if (!fileInfo->isAttributes(OptInfoType::kIsFile) && !allowMixedSelection) {
                if (doCdWhenPossible && fileInfo->isAttributes(OptInfoType::kIsDir)) {
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
            auto fileInfo = InfoFactory::create<FileInfo>(url);

            if (!fileInfo->isAttributes(OptInfoType::kIsDir))
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

bool FileDialogPrivate::checkFileSuffix(const QString &filename, QString &suffix)
{
    if (nameFilters.isEmpty())
        return false;

    for (const QString &nameFilterList : nameFilters) {
        for (const QString &nameFilter : cleanFilterList(nameFilterList)) {
            QString realFilter = nameFilter;
            realFilter.replace(".", "\\.");
            realFilter.replace("*", ".*");
            realFilter.append('$');
            QRegularExpression re(realFilter);
            if (re.match(filename).hasMatch())
                return false;
        }
    }

    QMimeDatabase mdb;
    int index = q->selectedNameFilterIndex();
    QString filter = nameFilters[index];
    QStringList newNameFilters = cleanFilterList(filter);
    if (newNameFilters.isEmpty())
        return false;

    for (const QString &newFilter : newNameFilters) {
        suffix = mdb.suffixForFileName(newFilter);
        if (suffix.isEmpty()) {
            if (!newFilter.startsWith("*."))
                continue;
            QString realFilter = newFilter.mid(2);
            if (realFilter.isEmpty())
                continue;
            realFilter.prepend('^');
            realFilter.append('$');
            QRegularExpression regExp(realFilter);
            for (const QMimeType &m : mdb.allMimeTypes()) {
                for (const QString &suffixe : m.suffixes()) {
                    if (regExp.match(suffixe).hasMatch()) {
                        suffix = suffixe;
                        return true;
                    }
                }
            }
        } else {
            return true;
        }
    }

    return false;
}

void FileDialogPrivate::setLastVisited(const QUrl &dir)
{
    lastVisitedDir = dir;
    delaySaveTimer->start();
}

void FileDialogPrivate::saveLastVisited()
{
    QSettings qtSets(QSettings::UserScope, QLatin1String("QtProject"));
    qtSets.setValue("FileDialog/lastVisited", lastVisitedDir.toString());
}

/*!
 * \class FileDialog
 */

FileDialog::FileDialog(const QUrl &url, QWidget *parent)
    : FileManagerWindow(url, parent),
      d(new FileDialogPrivate(this))
{
    setProperty("WINDOW_DISABLE_TITLEBAR_MENU", true);

    initializeUi();
    initConnect();
    initEventsConnect();
}

FileDialog::~FileDialog()
{
    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_View_SelectionChanged", this,
                                     &FileDialog::onViewSelectionChanged);
    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_View_RenameStartEdit", this, &FileDialog::handleRenameStartAcceptBtn);
    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_View_RenameEndEdit", this, &FileDialog::handleRenameEndAcceptBtn);
    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_View_ItemClicked", this,
                                     &FileDialog::onViewItemClicked);
    dpfSignalDispatcher->removeGlobalEventFilter(this);
}

void FileDialog::cd(const QUrl &url)
{
    FileManagerWindow::cd(url);

    d->lastVisitedDir = url;

    auto window = FMWindowsIns.findWindowById(this->internalWinId());
    if (!window)
        return;

    if (window->workSpace())
        handleUrlChanged(url);
    else
        connect(
                window, &FileManagerWindow::workspaceInstallFinished, this, [this, url] { handleUrlChanged(url); }, Qt::DirectConnection);
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
    int viewMode = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetDefaultViewMode", internalWinId()).toInt();
    auto mode = static_cast<DFMGLOBAL_NAMESPACE::ViewMode>(viewMode);
    if (mode == Global::ViewMode::kListMode)
        return QFileDialog::Detail;

    return QFileDialog::List;
}

QUrl FileDialog::lastVisitedUrl() const
{
    return d->lastVisitedDir;
}

void FileDialog::setDirectory(const QString &directory)
{
    QUrl url = UrlRoute::fromLocalFile(directory);
    QString errorString { "" };
    const FileInfoPointer &info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync, &errorString);
    if (!info) {
        fmCritical() << "File Dialog: can not create file info, the error is: " << errorString;
        return;
    }
    bool isSymLink = info->isAttributes(dfmbase::FileInfo::FileIsType::kIsSymLink);
    if (isSymLink)
        url = info->urlOf(dfmbase::FileInfo::FileUrlInfoType::kRedirectedFileUrl);
    setDirectoryUrl(url);
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

    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ url }, &urls);
    if (ok && !urls.empty())
        url = urls.first();

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
    QList<QUrl> list { CoreEventsCaller::sendGetSelectedFiles(internalWinId()) };

    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(list, &urls);

    if (ok && !urls.isEmpty())
        list = urls;

    if (d->acceptMode == QFileDialog::AcceptSave) {
        QUrl fileUrl = list.isEmpty() ? currentUrl() : list.first();
        auto fileInfo = InfoFactory::create<FileInfo>(fileUrl);

        if (fileInfo) {
            if (list.isEmpty()) {
                fileUrl = fileInfo->getUrlByType(UrlInfoType::kGetUrlByChildFileName, statusBar()->lineEdit()->text());
            } else {
                fileUrl = fileInfo->getUrlByType(UrlInfoType::kGetUrlByNewFileName, statusBar()->lineEdit()->text());
            }
        }

        return QList<QUrl>() << fileUrl;
    }

    if (list.isEmpty() && (d->fileMode == QFileDialog::Directory || static_cast<int>(d->fileMode) == kFileDialogDirectoryOnly)) {
        if (directoryUrl().isLocalFile())
            list << QUrl(directoryUrl());
    }
    return list;
}

void FileDialog::setNameFilters(const QStringList &filters)
{
    d->nameFilters = filters;

    QVariant isGtk = qApp->property("GTK");
    if (isGtk.isValid() && isGtk.toBool()) {
        statusBar()->setComBoxItems(CoreHelper::stripFilters(filters));
    } else {
        if (testOption(QFileDialog::HideNameFilterDetails)) {
            statusBar()->setComBoxItems(CoreHelper::stripFilters(filters));
        } else {
            statusBar()->setComBoxItems(filters);
        }
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

    if (d->fileMode == QFileDialog::Directory || static_cast<int>(d->fileMode) == kFileDialogDirectoryOnly) {
        // 清理只显示目录时对文件名添加的过滤条件
        dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetNameFilter", internalWinId(), QStringList());
        curNameFilters.clear();
    }

    d->fileMode = mode;
    updateAcceptButtonState();

    switch (static_cast<int>(mode)) {
    case QFileDialog::ExistingFiles:
        CoreEventsCaller::setEnabledSelectionModes(this, { QAbstractItemView::ExtendedSelection });
        break;
    case kFileDialogDirectoryOnly:
    case QFileDialog::Directory:
        // 文件名中不可能包含 '/', 此处目的是过滤掉所有文件
        dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetNameFilter", internalWinId(), QStringList("/"));
        curNameFilters = QStringList("/");
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
        QVariantMap vMap = DConfigManager::instance()->value("org.deepin.dde.file-manager.sidebar", "itemVisiable").toMap();
        urlSchemeEnable("recent", vMap.value("recent", true).toBool());

        disconnect(statusBar()->lineEdit(), &DLineEdit::textChanged,
                   this, &FileDialog::onCurrentInputNameChanged);
    } else {
        statusBar()->setMode(FileDialogStatusBar::kSave);
        CoreEventsCaller::setSelectionMode(this, QAbstractItemView::SingleSelection);
        urlSchemeEnable("recent", false);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        setFileMode(QFileDialog::DirectoryOnly);
#else
        setFileMode(QFileDialog::Directory);
#endif
        connect(statusBar()->lineEdit(), &DLineEdit::textChanged,
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
    // （此处修改比较特殊，临时方案）与产品沟通后，使用uos文管保存框保存文件时，如果当前目录下有同名文件，
    // 必须要弹出提示框 "是否覆盖重名文件"。
    // 所以options中的QFileDialog::DontConfirmOverwrite标志位将失去意义，
    // 所以此处直接将该标志位赋值为0。
    options &= ~QFileDialog::DontConfirmOverwrite;

    d->options = options;

    if (!d->isFileView)
        return;

    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetReadOnly",
                         internalWinId(), options.testFlag(QFileDialog::ReadOnly));

    if (options.testFlag(QFileDialog::ShowDirsOnly)) {
        QDir::Filters filters = filter() & ~QDir::Files & ~QDir::Drives;
        dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetFilter",
                             internalWinId(), filters);
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
    url.setHost("");
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
        statusBar()->lineEdit()->lineEdit()->selectAll();
    } else {
        statusBar()->lineEdit()->lineEdit()->setSelection(0, name.length() - suffix.length() - 1);
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
        DLabel *label = new DLabel(object["text"].toString());
        DLineEdit *edit = new DLineEdit();
        edit->setText(object["defaultValue"].toString());
#ifdef ENABLE_TESTING
        dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                             qobject_cast<QWidget *>(label), AcName::kAcFDStatusBarContentLabel);
        dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                             qobject_cast<QWidget *>(edit), AcName::kAcFDStatusBarContentEdit);
#endif

        if (maxLength > 0) {
            edit->lineEdit()->setMaxLength(maxLength);
        }

        if (!inputMask.isEmpty()) {
            edit->lineEdit()->setInputMask(inputMask);
        }

        edit->setEchoMode(echoMode);
        edit->setPlaceholderText(object["placeholderText"].toString());
        statusBar()->addLineEdit(label, edit);
    } else {
        QStringList dataList;
        for (const QVariant &v : object["data"].toArray().toVariantList()) {
            dataList << v.toString();
        }
        QString defaultValue = object["defaultValue"].toString();

        DLabel *label = new DLabel(object["text"].toString());
        DComboBox *comboBox = new DComboBox();
#ifdef ENABLE_TESTING
        dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                             qobject_cast<QWidget *>(label), AcName::kAcFDStatusBarContentLabel);
        dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                             qobject_cast<QWidget *>(comboBox), AcName::kAcFDStatusBarContentBox);
#endif

        comboBox->setEditable(object["editable"].toBool());
        comboBox->addItems(dataList);

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

QUrl FileDialog::getcurrenturl() const
{
    return d->currentUrl;
}

bool FileDialog::checkFileSuffix(const QString &filename, QString &suffix)
{
    return d->checkFileSuffix(filename, suffix);
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

    // DO NOT CLOSE the dialog here, may cause QFileDialog::getExsitingDirectory returns an empty path if CLOSE
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
        fmWarning("File Dialog: DFileDialog::exec: Recursive call detected");
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

    QVariant isGtk = qApp->property("GTK");
    if (isGtk.isValid() && isGtk.toBool()) {
        key = CoreHelper::stripFilters(QStringList(filter)).first();
    } else {
        if (testOption(QFileDialog::HideNameFilterDetails)) {
            key = CoreHelper::stripFilters(QStringList(filter)).first();
        } else {
            key = filter;
        }
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
    QStringList newNameFilters = FileDialogPrivate::cleanFilterList(nameFilter);

    if (d->acceptMode == QFileDialog::AcceptSave && !newNameFilters.isEmpty()) {
        QString fileName = statusBar()->lineEdit()->text();
        DFMBASE_NAMESPACE::DMimeDatabase db;
        int dotIndex = fileName.lastIndexOf(".");
        const QString &fileNameExtension = dotIndex > 0 ? fileName.mid(dotIndex + 1) : db.suffixForFileName(fileName);
        QString newNameFilterExtension { CoreHelper::findExtensionName(fileName, newNameFilters, &db) };

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
    if ((static_cast<int>(d->fileMode) == kFileDialogDirectoryOnly || d->fileMode == QFileDialog::Directory) && QStringList("/") != newNameFilters)
        newNameFilters = QStringList("/");

    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetNameFilter", internalWinId(), newNameFilters);
    curNameFilters = newNameFilters;
}

int FileDialog::selectedNameFilterIndex() const
{
    const QComboBox *box = statusBar()->comboBox();

    return box ? box->currentIndex() : -1;
}

QDir::Filters FileDialog::filter() const
{
    return static_cast<QDir::Filters>(dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetFilter", internalWinId()).toInt());
}

void FileDialog::setFilter(QDir::Filters filters)
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetFilter", internalWinId(), filters);
}

void FileDialog::updateAcceptButtonState()
{
    if (!d->isFileView)
        return;
    QUrl url = currentUrl();
    auto fileInfo = InfoFactory::create<FileInfo>(url);
    if (fileInfo == nullptr && statusBar() && statusBar()->acceptButton()) {
        statusBar()->acceptButton()->setDisabled(true);
        return;
    }
    if (fileInfo == nullptr)
        return;

    bool isDirMode = d->fileMode == QFileDialog::Directory || static_cast<int>(d->fileMode) == kFileDialogDirectoryOnly;
    bool dialogShowMode = d->acceptMode;
    bool isVirtual = UrlRoute::isVirtual(fileInfo->urlOf(UrlInfoType::kUrl).scheme());
    if (dialogShowMode == QFileDialog::AcceptOpen) {
        auto size = CoreEventsCaller::sendGetSelectedFiles(internalWinId()).size();
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

    // INFO: [FileDialog::handleEnterPressed] Processing Enter key based on accept mode.
    
    if (d->acceptMode == QFileDialog::AcceptSave) {
        handleEnterInSaveMode();
    } else {
        handleEnterInOpenMode();
    }
}

bool FileDialog::isFileNameEditFocused() const
{
    if (!statusBar() || !statusBar()->lineEdit())
        return false;
    
    // INFO: [FileDialog::isFileNameEditFocused] Checking focus state of filename edit.
    return statusBar()->lineEdit()->hasFocus() || 
           statusBar()->lineEdit()->lineEdit()->hasFocus();
}

void FileDialog::handleEnterInSaveMode()
{
    // INFO: [FileDialog::handleEnterInSaveMode] Processing Enter key in save mode.
    
    // 保存模式下的逻辑：
    // 1. 如果焦点在文件名编辑框，直接触发保存
    // 2. 如果焦点在文件视图且选中目录，让文件视图处理（进入目录）
    // 3. 如果焦点在文件视图且选中文件，触发保存操作
    // 4. 如果焦点在文件视图且没有选中任何内容，触发保存操作
    
    if (isFileNameEditFocused()) {
        // INFO: [FileDialog::handleEnterInSaveMode] Enter pressed in filename edit, triggering save.
        statusBar()->acceptButton()->animateClick();
        return;
    }
    
    // 检查选中的内容
    auto &&urls = CoreEventsCaller::sendGetSelectedFiles(internalWinId());
    if (urls.size() == 1) {
        auto info = InfoFactory::create<FileInfo>(urls.first());
        if (info && info->isAttributes(OptInfoType::kIsDir)) {
            // INFO: [FileDialog::handleEnterInSaveMode] Enter pressed on directory, let file view handle navigation.
            // 选中目录时，不做任何操作，让文件视图的默认处理逻辑处理（进入目录）
            return;
        }
    }
    
    // 选中文件或没有选中任何内容时，触发保存操作
    // INFO: [FileDialog::handleEnterInSaveMode] Enter pressed on file or empty selection, triggering save.
    statusBar()->acceptButton()->animateClick();
}

void FileDialog::handleEnterInOpenMode()
{
    // INFO: [FileDialog::handleEnterInOpenMode] Processing Enter key in open mode.
    
    // 打开模式下的逻辑：
    // 1. 如果选中目录，进入目录
    // 2. 如果选中文件，触发打开操作
    
    // 检查选中的文件/目录
    bool shouldEnterDirectory = false;
    auto &&urls = CoreEventsCaller::sendGetSelectedFiles(internalWinId());
    for (const QUrl &url : urls) {
        auto info = InfoFactory::create<FileInfo>(url);
        if (!info || info->isAttributes(OptInfoType::kIsDir)) {
            shouldEnterDirectory = true;
            break;
        }
    }

    if (!shouldEnterDirectory) {
        // INFO: [FileDialog::handleEnterInOpenMode] Enter pressed on files, triggering open.
        statusBar()->acceptButton()->animateClick();
    }
    // 如果选中目录，不做任何操作，让默认的文件视图处理逻辑处理
}

void FileDialog::handleUrlChanged(const QUrl &url)
{
    QString scheme { url.scheme() };

    d->lastIsFileView = d->isFileView;
    d->isFileView = dpfSlotChannel->push("dfmplugin_workspace", "slot_CheckSchemeViewIsFileView", scheme).toBool();

    // init accept mode, worskapce must initialized
    bool isFirst { false };
    static std::once_flag flag;
    std::call_once(flag, [this, &isFirst]() {
        isFirst = true;
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
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetNameFilter", internalWinId(), curNameFilters);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetAlwaysOpenInCurrentWindow", internalWinId());
    d->currentUrl = url;
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

void FileDialog::onViewItemClicked(const QVariantMap &data)
{
    if (!statusBar() || (acceptMode() != QFileDialog::AcceptSave))
        return;

    if (!data.contains("displayName") || !data.contains("url"))
        return;

    QString displayName = data["displayName"].toString();
    QUrl url = data["url"].toUrl();
    if (!url.isValid() || url.isEmpty() || displayName.isEmpty())
        return;

    const auto &fileInfo = InfoFactory::create<FileInfo>(url);
    if (fileInfo && !fileInfo->isAttributes(OptInfoType::kIsDir)) {
        QMimeDatabase db;
        // TODO(gongheng): Encapsulate get true suffix interface to fileinfo like QMimeDatabase::suffix.
        int suffixLength = db.suffixForFileName(displayName).count();
        if (suffixLength != 0)
            suffixLength++;   // decimal point
        QString displayNameWithoutSuffix = displayName.mid(0, displayName.count() - suffixLength);
        statusBar()->changeFileNameEditText(displayNameWithoutSuffix);
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

    if (windowFlags().testFlag(Qt::WindowSystemMenuHint)) {
        overrideWindowFlags(windowFlags() & ~Qt::WindowSystemMenuHint);
    }

    const FileInfoPointer &info = InfoFactory::create<FileInfo>(currentUrl());
    if (info)
        setWindowTitle(info->displayOf(DisPlayInfoType::kFileDisplayName));

    FileManagerWindow::showEvent(event);
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
    FileManagerWindow::closeEvent(event);
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
                dpfSlotChannel->push("dfmplugin_workspace", "slot_View_ClosePersistentEditor", internalWinId());
                close();
            } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
                handleEnterPressed();
            }
        }
    }

    return FileManagerWindow::eventFilter(watched, event);
}

void FileDialog::initializeUi()
{
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::Dialog);

    // init status bar
    d->statusBar = new FileDialogStatusBar(this);
    d->statusBar->setAutoFillBackground(true);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(d->statusBar), AcName::kAcFDStautsBar);
#endif
    statusBar()->lineEdit()->lineEdit()->setMaxLength(NAME_MAX);

    CoreEventsCaller::setMenuDisbaled();
}

void FileDialog::initConnect()
{
    connect(statusBar()->acceptButton(), &QPushButton::clicked, this, &FileDialog::onAcceptButtonClicked);
    connect(statusBar()->rejectButton(), &QPushButton::clicked, this, &FileDialog::onRejectButtonClicked);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    connect(statusBar()->comboBox(),
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &FileDialog::selectNameFilter);
    connect(statusBar()->comboBox(),
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &FileDialog::selectedNameFilterChanged);
#else
    connect(statusBar()->comboBox(), &QComboBox::textActivated,
            this, &FileDialog::selectNameFilter);
    connect(statusBar()->comboBox(), &QComboBox::textActivated,
            this, &FileDialog::selectedNameFilterChanged);
#endif
    connect(this, &FileDialog::selectionFilesChanged, &FileDialog::updateAcceptButtonState);
    connect(this, &FileDialog::currentUrlChanged, this, [this](auto url) {
        d->lastVisitedDir = url;
    });
}

void FileDialog::initEventsConnect()
{
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_View_RenameStartEdit", this, &FileDialog::handleRenameStartAcceptBtn);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_View_RenameEndEdit", this, &FileDialog::handleRenameEndAcceptBtn);
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

    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetDragEnabled", internalWinId(), false);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetDragDropMode", internalWinId(), QAbstractItemView::NoDragDrop);

    // TODO(liuyangming): currentChanged
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_View_SelectionChanged", this,
                                   &FileDialog::onViewSelectionChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_View_ItemClicked", this,
                                   &FileDialog::onViewItemClicked);

    if (!d->nameFilters.isEmpty())
        setNameFilters(d->nameFilters);

    if (d->filters != QDir::NoFilter)
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

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (w) {
        scrn = QApplication::desktop()->screenNumber(w);
    } else if (QApplication::desktop()->isVirtualDesktop()) {
        scrn = QApplication::desktop()->screenNumber(QCursor::pos());
    } else {
        scrn = QApplication::desktop()->screenNumber(this);
    }
    desk = QApplication::desktop()->availableGeometry(scrn);
#else
    QScreen *screen = nullptr;

    if (w) {
        screen = w->windowHandle()->screen();
    } else {
        screen = QGuiApplication::screenAt(QCursor::pos());
    }

    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    desk = screen->availableGeometry();
#endif

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

    const QStringList &filters = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_GetNameFilter", internalWinId()).toStringList();

    if (filters.isEmpty()) {
        return QString();
    }

    return filters.first();
}
