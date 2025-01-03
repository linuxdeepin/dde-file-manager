// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    stackedLayout = new QStackedLayout;

    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    mainLayout->addItem(stackedLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addSpacing(30);

    this->setLayout(mainLayout);
}

PdfWidget::~PdfWidget()
{
    PageRenderThread::destroyForever();
    for (DocSheet *sheet : sheetMap.getSheets())
        closeSheet(sheet);
}

void PdfWidget::addFileAsync(const QString &filePath)
{
    //! 判断在打开的文档中是否有filePath，如果有则切到相应的sheet，反之执行打开操作
    int index = sheetMap.indexOfFilePath(filePath);
    if (index >= 0) {
        leaveSheet(static_cast<DocSheet *>(stackedLayout->currentWidget()));
        DocSheet *sheet = DocSheet::getSheetByFilePath(filePath);
        stackedLayout->addWidget(sheet);
        stackedLayout->setCurrentWidget(sheet);
        this->activateWindow();
        return;
    }

    FileType fType = fileType(filePath);

    if (kPDF != fType) {
        return;
    }

    DocSheet *sheet = new DocSheet(fType, filePath, this);

    leaveSheet(static_cast<DocSheet *>(stackedLayout->currentWidget()));

    connect(sheet, &DocSheet::sigFileOpened, this, &PdfWidget::onOpened);

    addSheet(sheet);

    this->activateWindow();

    sheet->openFileAsync("");
}

void PdfWidget::addSheet(DocSheet *sheet)
{
    sheetMap.insertSheet(sheet);
    enterSheet(sheet);
}

void PdfWidget::enterSheet(DocSheet *sheet)
{
    if (nullptr == sheet)
        return;

    sheet->setParent(this);

    stackedLayout->addWidget(sheet);

    stackedLayout->setCurrentWidget(sheet);
}

void PdfWidget::leaveSheet(DocSheet *sheet)
{
    if (nullptr == sheet)
        return;

    stackedLayout->removeWidget(sheet);
}

bool PdfWidget::closeSheet(DocSheet *sheet)
{
    if (nullptr == sheet)
        return false;

    if (!DocSheet::existSheet(sheet))
        return false;

    stackedLayout->removeWidget(sheet);

    sheetMap.removeSheet(sheet);

    delete sheet;

    return true;
}

bool PdfWidget::closeAllSheets()
{
    const auto &sheets = sheetMap.getSheets();
    return std::accumulate(sheets.begin(), sheets.end(),
                           true, [this](bool result, DocSheet *sheet) {
                               return result = closeSheet(sheet);
                           });
}

void PdfWidget::onOpened(DocSheet *sheet, Document::Error error)
{
    if (nullptr == sheet)
        return;

    if (Document::kFileError == error || Document::kFileDamaged == error || Document::kConvertFailed == error) {
        stackedLayout->removeWidget(sheet);

        sheet->deleteLater();

        if (Document::kFileError == error)
            fmWarning() << "PDF Preview: Open failed!";
        else if (Document::kFileDamaged == error)
            fmWarning() << "PDF Preview: Please check if the file is damaged!";
        else if (Document::kConvertFailed == error)
            fmWarning() << "PDF Preview: Conversion failed, please check if the file is damaged!";
    }
}

void recordSheetPath::insertSheet(DocSheet *sheet)
{
    docSheetMap.insert(sheet, sheet->filePath());
}

int recordSheetPath::indexOfFilePath(const QString &filePath)
{
    int index = 0;
    for (QString &path : docSheetMap.values()) {
        if (filePath == path)
            return index;
        index++;
    }

    return -1;
}

void recordSheetPath::removeSheet(DocSheet *sheet)
{
    docSheetMap.remove(sheet);
}

QList<DocSheet *> recordSheetPath::getSheets()
{
    return docSheetMap.keys();
}
