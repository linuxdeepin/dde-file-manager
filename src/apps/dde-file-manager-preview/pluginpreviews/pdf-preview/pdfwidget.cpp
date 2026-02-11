// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfwidget.h"
#include "docsheet.h"
#include "pagerenderthread.h"

#include <QVBoxLayout>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDebug>

using namespace plugin_filepreview;
PdfWidget::PdfWidget(QWidget *parent)
    : QWidget(parent)
{
    fmDebug() << "PDF preview: PdfWidget constructor called";
    
    stackedLayout = new QStackedLayout;

    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    mainLayout->addItem(stackedLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addSpacing(30);

    this->setLayout(mainLayout);
    
    fmDebug() << "PDF preview: PdfWidget layout initialized";
}

PdfWidget::~PdfWidget()
{
    fmInfo() << "PDF preview: PdfWidget destructor called, cleaning up" << sheetMap.getSheets().size() << "sheets";
    
    PageRenderThread::destroyForever();
    for (DocSheet *sheet : sheetMap.getSheets())
        closeSheet(sheet);
        
    fmDebug() << "PDF preview: PdfWidget cleanup completed";
}

void PdfWidget::addFileAsync(const QString &filePath)
{
    fmInfo() << "PDF preview: adding file asynchronously:" << filePath;
    
    // Check if the document is already open, if so switch to the corresponding sheet, otherwise perform open operation
    int index = sheetMap.indexOfFilePath(filePath);
    if (index >= 0) {
        fmDebug() << "PDF preview: file already open, switching to existing sheet:" << filePath;
        leaveSheet(static_cast<DocSheet *>(stackedLayout->currentWidget()));
        DocSheet *sheet = DocSheet::getSheetByFilePath(filePath);
        stackedLayout->addWidget(sheet);
        stackedLayout->setCurrentWidget(sheet);
        this->activateWindow();
        return;
    }

    FileType fType = fileType(filePath);

    if (kPDF != fType) {
        fmWarning() << "PDF preview: file is not a PDF type:" << filePath << "detected type:" << fType;
        return;
    }

    fmDebug() << "PDF preview: creating new DocSheet for:" << filePath;
    DocSheet *sheet = new DocSheet(fType, filePath, this);

    leaveSheet(static_cast<DocSheet *>(stackedLayout->currentWidget()));

    connect(sheet, &DocSheet::sigFileOpened, this, &PdfWidget::onOpened);

    addSheet(sheet);

    this->activateWindow();

    fmDebug() << "PDF preview: starting async file open for:" << filePath;
    sheet->openFileAsync("");
}

void PdfWidget::addSheet(DocSheet *sheet)
{
    if (!sheet) {
        fmWarning() << "PDF preview: attempted to add null sheet";
        return;
    }
    
    fmDebug() << "PDF preview: adding sheet for file:" << sheet->filePath();
    sheetMap.insertSheet(sheet);
    enterSheet(sheet);
}

void PdfWidget::enterSheet(DocSheet *sheet)
{
    if (nullptr == sheet) {
        fmWarning() << "PDF preview: attempted to enter null sheet";
        return;
    }

    fmDebug() << "PDF preview: entering sheet for file:" << sheet->filePath();
    sheet->setParent(this);

    stackedLayout->addWidget(sheet);
    stackedLayout->setCurrentWidget(sheet);
}

void PdfWidget::leaveSheet(DocSheet *sheet)
{
    if (nullptr == sheet) {
        fmDebug() << "PDF preview: no sheet to leave (null sheet)";
        return;
    }

    fmDebug() << "PDF preview: leaving sheet for file:" << sheet->filePath();
    stackedLayout->removeWidget(sheet);
}

bool PdfWidget::closeSheet(DocSheet *sheet)
{
    if (nullptr == sheet) {
        fmWarning() << "PDF preview: attempted to close null sheet";
        return false;
    }

    if (!DocSheet::existSheet(sheet)) {
        fmWarning() << "PDF preview: attempted to close non-existent sheet";
        return false;
    }

    fmDebug() << "PDF preview: closing sheet for file:" << sheet->filePath();
    stackedLayout->removeWidget(sheet);
    sheetMap.removeSheet(sheet);
    delete sheet;

    return true;
}

bool PdfWidget::closeAllSheets()
{
    fmInfo() << "PDF preview: closing all sheets, count:" << sheetMap.getSheets().size();
    
    const auto &sheets = sheetMap.getSheets();
    bool result = std::accumulate(sheets.begin(), sheets.end(),
                           true, [this](bool result, DocSheet *sheet) {
                               return result = closeSheet(sheet);
                           });
                           
    fmDebug() << "PDF preview: close all sheets result:" << result;
    return result;
}

void PdfWidget::onOpened(DocSheet *sheet, Document::Error error)
{
    if (nullptr == sheet) {
        fmWarning() << "PDF preview: onOpened called with null sheet";
        return;
    }

    const QString filePath = sheet->filePath();
    
    if (Document::kFileError == error || Document::kFileDamaged == error || Document::kConvertFailed == error) {
        fmWarning() << "PDF preview: failed to open file:" << filePath << "error:" << error;
        
        stackedLayout->removeWidget(sheet);
        sheet->deleteLater();

        if (Document::kFileError == error)
            fmWarning() << "PDF preview: file open failed:" << filePath;
        else if (Document::kFileDamaged == error)
            fmWarning() << "PDF preview: file appears to be damaged:" << filePath;
        else if (Document::kConvertFailed == error)
            fmWarning() << "PDF preview: file conversion failed, possibly damaged:" << filePath;
    } else {
        fmInfo() << "PDF preview: file opened successfully:" << filePath;
    }
}

void recordSheetPath::insertSheet(DocSheet *sheet)
{
    if (!sheet) {
        fmWarning() << "PDF preview: attempted to insert null sheet into record";
        return;
    }
    
    fmDebug() << "PDF preview: recording sheet path:" << sheet->filePath();
    docSheetMap.insert(sheet, sheet->filePath());
}

int recordSheetPath::indexOfFilePath(const QString &filePath)
{
    int index = 0;
    for (QString &path : docSheetMap.values()) {
        if (filePath == path) {
            fmDebug() << "PDF preview: found existing sheet at index" << index << "for path:" << filePath;
            return index;
        }
        index++;
    }

    fmDebug() << "PDF preview: no existing sheet found for path:" << filePath;
    return -1;
}

void recordSheetPath::removeSheet(DocSheet *sheet)
{
    if (!sheet) {
        fmWarning() << "PDF preview: attempted to remove null sheet from record";
        return;
    }
    
    fmDebug() << "PDF preview: removing sheet record for path:" << sheet->filePath();
    docSheetMap.remove(sheet);
}

QList<DocSheet *> recordSheetPath::getSheets()
{
    return docSheetMap.keys();
}
