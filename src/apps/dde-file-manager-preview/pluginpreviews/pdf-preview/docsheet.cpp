// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docsheet.h"

#include "sheetsidebar.h"
#include "sheetbrowser.h"
#include "encryptionpage.h"
#include "pdfmodel.h"
#include "sheetrenderer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QStackedWidget>
#include <QMimeData>
#include <QUuid>
#include <QClipboard>
#include <QFileInfo>
#include <QPropertyAnimation>
#include <QDebug>
#include <QTemporaryDir>
#include <QApplication>
#include <QChildEvent>
#include <QScreen>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
QReadWriteLock DocSheet::lockReadWrite;
QStringList DocSheet::uuidList;
QList<DocSheet *> DocSheet::sheetList;
DocSheet::DocSheet(const FileType &fileType, const QString &filePath, QWidget *parent)
    : DSplitter(parent), currentFilePath(filePath), currentFileType(fileType)
{
    setAlive(true);
    setHandleWidth(5);
    setChildrenCollapsible(false);   //!  子部件不可拉伸到 0

    sheetRenderer = new SheetRenderer(this);
    connect(sheetRenderer, &SheetRenderer::sigOpened, this, &DocSheet::onOpened);

    sheetBrowser = new SheetBrowser(this);
    sheetBrowser->setMinimumWidth(700);

    if (kPDF == fileType)
        sheetSidebar = new SheetSidebar(this, PREVIEW_THUMBNAIL);
    else
        sheetSidebar = new SheetSidebar(this, PREVIEW_NULL);

    sheetSidebar->setFixedWidth(96);

    connect(sheetBrowser, SIGNAL(sigPageChanged(int)), this, SLOT(onBrowserPageChanged(int)));

    resetChildParent();
    this->insertWidget(0, sheetBrowser);
    this->insertWidget(0, sheetSidebar);

    setSidebarVisible(true);
}

DocSheet::~DocSheet()
{
    setAlive(false);

    delete sheetBrowser;

    delete sheetSidebar;

    delete sheetRenderer;

    if (nullptr != temporaryDir)
        delete temporaryDir;
}

bool DocSheet::existSheet(DocSheet *sheet)
{
    lockReadWrite.lockForRead();

    bool result = sheetList.contains(sheet);

    lockReadWrite.unlock();

    return result;
}

DocSheet *DocSheet::getSheetByFilePath(QString filePath)
{
    lockReadWrite.lockForRead();

    DocSheet *result = nullptr;

    foreach (DocSheet *sheet, sheetList) {
        if (sheet->filePath() == filePath) {
            result = sheet;
            break;
        }
    }

    lockReadWrite.unlock();

    return result;
}

QList<DocSheet *> DocSheet::getSheets()
{
    return DocSheet::sheetList;
}

bool DocSheet::openFileExec(const QString &password)
{
    docPassword = password;

    return sheetRenderer->openFileExec(password);
}

void DocSheet::openFileAsync(const QString &password)
{
    docPassword = password;

    sheetRenderer->openFileAsync(docPassword);
}

void DocSheet::jumpToPage(int page)
{
    sheetBrowser->setCurrentPage(page);
}

void DocSheet::jumpToIndex(int index)
{
    sheetBrowser->setCurrentPage(index + 1);
}

int DocSheet::pageCount()
{
    return sheetRenderer->getPageCount();
}

int DocSheet::currentPage()
{
    if (sheetOperation.currentPage < 1 || sheetOperation.currentPage > pageCount())
        return 1;

    return sheetOperation.currentPage;
}

int DocSheet::currentIndex()
{
    if (sheetOperation.currentPage < 1 || sheetOperation.currentPage > pageCount())
        return 0;

    return sheetOperation.currentPage - 1;
}

void DocSheet::setLayoutMode(LayoutMode mode)
{
    if (mode == sheetOperation.layoutMode)
        return;

    if (mode >= SinglePageMode && mode < NumberOfLayoutModes) {
        sheetOperation.layoutMode = mode;
        sheetBrowser->deform(sheetOperation);
    }
}

QPixmap DocSheet::thumbnail(int index)
{
    return thumbnailMap.value(index);
}

void DocSheet::setThumbnail(int index, QPixmap pixmap)
{
    thumbnailMap[index] = pixmap;
}

QImage DocSheet::getImage(int index, int width, int height, const QRect &slice)
{
    return sheetRenderer->getImage(index, width, height, slice);
}

QList<qreal> DocSheet::scaleFactorList()
{
    QList<qreal> dataList = { 0.1, 0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2, 3, 4, 5 };
    QList<qreal> factorList;

    qreal maxFactor = maxScaleFactor();

    foreach (qreal factor, dataList) {
        if (maxFactor - factor > -0.0001)
            factorList.append(factor);
    }

    return factorList;
}

qreal DocSheet::maxScaleFactor()
{
    qreal maxScaleFactor = 20000 / (sheetBrowser->maxHeight() * qApp->devicePixelRatio());

    maxScaleFactor = qBound(0.1, maxScaleFactor, 5.0);

    return maxScaleFactor;
}

QString DocSheet::format()
{
    return QString("PDF");
}

SheetOperation DocSheet::operation() const
{
    return sheetOperation;
}

FileType DocSheet::fileType()
{
    return currentFileType;
}

SheetOperation &DocSheet::operationRef()
{
    return sheetOperation;
}

QString DocSheet::filePath()
{
    return currentFilePath;
}

QString DocSheet::openedFilePath()
{
    return filePath();
}

void DocSheet::setSidebarVisible(bool isVisible, bool notify)
{
    if (notify) {
        //! 左侧栏是否需要隐藏
        sheetSidebar->setVisible(isVisible);
        sheetOperation.sidebarVisible = isVisible;

        if (isVisible) {
            this->insertWidget(0, sheetSidebar);
        } else if (isFullScreen()) {
            resetChildParent();
            this->insertWidget(0, sheetBrowser);

            sheetSidebar->resize(sheetSidebar->width(), qApp->primaryScreen()->size().height());
            sheetSidebar->move(-sheetSidebar->width(), 0);
            sheetSidebar->setVisible(false);
        }
    }
}

void DocSheet::onBrowserPageChanged(int page)
{
    if (sheetOperation.currentPage != page) {
        sheetOperation.currentPage = page;
        if (sheetSidebar)
            sheetSidebar->setCurrentPage(page);
    }
}

void DocSheet::resizeEvent(QResizeEvent *event)
{
    DSplitter::resizeEvent(event);
    if (encrytionPage) {
        encrytionPage->setGeometry(0, 0, this->width(), this->height());
    }

    if (isFullScreen()) {
        sheetSidebar->resize(sheetSidebar->width(), this->height());
    }
}

void DocSheet::childEvent(QChildEvent *event)
{
    if (event->removed()) {
        return DSplitter::childEvent(event);
    }
}

void DocSheet::setAlive(bool alive)
{
    if (alive) {
        if (!uuidFile.isEmpty())
            setAlive(false);

        uuidFile = QUuid::createUuid().toString();

        lockReadWrite.lockForWrite();

        uuidList.append(uuidFile);

        sheetList.append(this);

        lockReadWrite.unlock();

    } else {
        if (uuidFile.isEmpty())
            return;

        lockReadWrite.lockForWrite();

        int index = uuidList.indexOf(uuidFile);

        sheetList.removeAt(index);

        uuidList.removeAt(index);

        uuidFile.clear();

        lockReadWrite.unlock();
    }
}

bool DocSheet::opened()
{
    return sheetRenderer->opened();
}

void DocSheet::onExtractPassword(const QString &password)
{
    docPassword = password;

    sheetRenderer->openFileAsync(docPassword);
}

SheetRenderer *DocSheet::renderer()
{
    return sheetRenderer;
}

void DocSheet::showEncryPage()
{
    if (encrytionPage == nullptr) {
        encrytionPage = new EncryptionPage(this);
        connect(encrytionPage, &EncryptionPage::sigExtractPassword, this, &DocSheet::onExtractPassword);
        this->stackUnder(encrytionPage);
    }

    //! 密码输入框在的时候,先暂时屏蔽掉browser的焦点
    sheetBrowser->setFocusPolicy(Qt::NoFocus);
    encrytionPage->setGeometry(0, 0, this->width(), this->height());
    encrytionPage->raise();
    encrytionPage->show();
}

QSizeF DocSheet::pageSizeByIndex(int index)
{
    return sheetRenderer->getPageSize(index);
}

void DocSheet::resetChildParent()
{
    sheetSidebar->setParent(nullptr);
    sheetSidebar->setParent(this);

    sheetBrowser->setParent(nullptr);
    sheetBrowser->setParent(this);
}

void DocSheet::onOpened(Document::Error error)
{
    if (Document::kNeedPassword == error) {
        showEncryPage();
    } else if (Document::kWrongPassword == error) {
        showEncryPage();

        encrytionPage->wrongPassWordSlot();
    } else if (Document::kNoError == error) {
        if (!docPassword.isEmpty()) {
            sheetBrowser->setFocusPolicy(Qt::StrongFocus);

            if (encrytionPage) {
                encrytionPage->hide();
                encrytionPage->deleteLater();
            }
            encrytionPage = nullptr;
        }

        sheetBrowser->init(sheetOperation);

        sheetSidebar->handleOpenSuccess();
    }

    //! 交给父窗口控制自己是否删除
    emit sigFileOpened(this, error);
}
