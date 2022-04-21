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

#include "dfm-base/utils/windowutils.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QPointer>
#include <QPushButton>
#include <QComboBox>

#include <DTitlebar>
#include <DWidgetUtil>

DFMBASE_USE_NAMESPACE
DIALOGCORE_USE_NAMESPACE

FileDialogPrivate::FileDialogPrivate(FileDialog *qq)
    : QObject(nullptr),
      q(qq)
{
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
}

FileDialog::~FileDialog()
{
}

bool FileDialog::saveClosedSate() const
{
    return false;
}

void FileDialog::updateAsDefaultSize()
{
    resize(d->kDefaultWindowWidth, d->kDefaultWindowHeight);
}

void FileDialog::setFileMode(QFileDialog::FileMode mode)
{
    // TODO(zhangs):
}

void FileDialog::setAllowMixedSelection(bool on)
{
    // TODO(zhangs):
}

void FileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    d->acceptMode = mode;
    updateAcceptButtonState();

    if (mode == QFileDialog::AcceptOpen) {
        statusBar()->setMode(FileDialogStatusBar::kOpen);
        setFileMode(d->fileMode);

        disconnect(statusBar()->lineEdit(), &QLineEdit::textChanged,
                   this, &FileDialog::onCurrentInputNameChanged);
    } else {
        statusBar()->setMode(FileDialogStatusBar::kSave);
        // TODO(zhangs):
        // getFileView()->setSelectionMode(QAbstractItemView::SingleSelection);
        // getLeftSideBar()->setDisableUrlSchemes({ "recent" });   // save mode disable recent
        setFileMode(QFileDialog::DirectoryOnly);

        connect(statusBar()->lineEdit(), &QLineEdit::textChanged,
                this, &FileDialog::onCurrentInputNameChanged);
    }
}

QFileDialog::AcceptMode FileDialog::acceptMode() const
{
    // TODO(zhangs):
    return {};
}

void FileDialog::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    // TODO(zhangs):
}

QString FileDialog::labelText(QFileDialog::DialogLabel label) const
{
    // TODO(zhangs):
    return {};
}

void FileDialog::setOptions(QFileDialog::Options options)
{
    // TODO(zhangs):
}

void FileDialog::setOption(QFileDialog::Option option, bool on)
{
    // TODO(zhangs):
}

bool FileDialog::testOption(QFileDialog::Option option) const
{
    // TODO(zhangs):
    return {};
}

QFileDialog::Options FileDialog::options() const
{
    // TODO(zhangs):
    return {};
}

void FileDialog::setCurrentInputName(const QString &name)
{
    // TODO(zhangs):
}

void FileDialog::addCustomWidget(FileDialog::CustomWidgetType type, const QString &data)
{
    // TODO(zhangs):
}

QVariant FileDialog::getCustomWidgetValue(FileDialog::CustomWidgetType type, const QString &text) const
{
    // TODO(zhangs):
    return {};
}

QVariantMap FileDialog::allCustomWidgetsValue(FileDialog::CustomWidgetType type) const
{
    // TODO(zhangs):
    return {};
}

void FileDialog::beginAddCustomWidget()
{
    // TODO(zhangs):
}

void FileDialog::endAddCustomWidget()
{
    // TODO(zhangs):
}

void FileDialog::setHideOnAccept(bool enable)
{
    // TODO(zhangs):
}

bool FileDialog::hideOnAccept() const
{
    // TODO(zhangs):
    return {};
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
}

void FileDialog::onRejectButtonClicked()
{
    reject();
}

void FileDialog::onCurrentInputNameChanged()
{
    // TODO(zhangs):
}

void FileDialog::selectNameFilter(const QString &filter)
{
    // TODO(zhangs):
}

void FileDialog::updateAcceptButtonState()
{
    // TODO(zhangs):
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
    statusBar()->lineEdit()->setMaxLength(FileDialogPrivate::kMaxFileCharCount);

    setAcceptMode(QFileDialog::AcceptOpen);

    // 修复bug-45176
    // 如果是wanyland平台，将弹出的文件框居中
    if (WindowUtils::isWayLand())
        Dtk::Widget::moveToCenter(this);
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
}

FileDialogStatusBar *FileDialog::statusBar() const
{
    return d->statusBar;
}
