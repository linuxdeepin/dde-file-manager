// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedialogstatusbar.h"
#include "filedialog.h"

#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/event/event.h>

#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif
#include <DLineEdit>
#include <DLabel>
#include <DComboBox>
#include <DSuggestButton>
#include <DFrame>
#include <DListView>

#include <QHBoxLayout>
#include <QWindow>
#include <QTimer>
#include <QDebug>
#include <QFontMetrics>
#include <QAbstractItemView>
#include <QListView>
#include <QScrollBar>

using namespace filedialog_core;
DWIDGET_USE_NAMESPACE

FileDialogStatusBar::FileDialogStatusBar(QWidget *parent)
    : QFrame(parent), mainWindow(qobject_cast<FileDialog *>(parent))
{
    initializeUi();
    initializeConnect();
}

void FileDialogStatusBar::setMode(FileDialogStatusBar::Mode mode)
{
    if (curMode == mode)
        return;

    curMode = mode;

    QString acButton = mode == kSave ? tr("Save", "button") : tr("Open", "button");
    if (curAcceptButton->text().isEmpty())
        curAcceptButton->setText(acButton);
    curAcceptButton->setObjectName(acButton);

    updateLayout();

    if (titleLabel->text().isEmpty()) {
        QString text = mode == kSave ? tr("Save File", "button") : tr("Open File", "button");
        titleLabel->setText(text);
        titleLabel->setObjectName(text);
    }
}

void FileDialogStatusBar::setComBoxItems(const QStringList &list)
{
    bool visible = filtersComboBox->isVisible();

    filtersComboBox->clear();
    filtersComboBox->addItems(list);

    if (curMode == kOpen) {
        filtersComboBox->setHidden(list.isEmpty());
        filtersLabel->setHidden(list.isEmpty());
    } else {
        if (visible == list.isEmpty()) {
            updateLayout();
        }
    }
}

DComboBox *FileDialogStatusBar::comboBox() const
{
    return filtersComboBox;
}

DLineEdit *FileDialogStatusBar::lineEdit() const
{
    return fileNameEdit;
}

DSuggestButton *FileDialogStatusBar::acceptButton() const
{
    return curAcceptButton;
}

DPushButton *FileDialogStatusBar::rejectButton() const
{
    return curRejectButton;
}

void FileDialogStatusBar::addLineEdit(DLabel *label, DLineEdit *edit)
{
    customLineEditList << qMakePair(label, edit);
}

QString FileDialogStatusBar::getLineEditValue(const QString &text) const
{
    auto iter = std::find_if(customLineEditList.begin(), customLineEditList.end(),
                             [text](const QPair<DLabel *, DLineEdit *> &i) {
                                 return i.first->text() == text;
                             });

    if (iter != customLineEditList.end()) {
        return iter->second->text();
    }

    return QString();
}

QVariantMap FileDialogStatusBar::allLineEditsValue() const
{
    QVariantMap map;

    for (auto i : customLineEditList)
        map[i.first->text()] = i.second->text();

    return map;
}

void FileDialogStatusBar::addComboBox(DLabel *label, DComboBox *box)
{
    customComboBoxList << qMakePair(label, box);
}

QString FileDialogStatusBar::getComboBoxValue(const QString &text) const
{
    auto iter = std::find_if(customComboBoxList.begin(), customComboBoxList.end(),
                             [text](const QPair<QLabel *, QComboBox *> &i) {
                                 return i.first->text() == text;
                             });

    if (iter != customComboBoxList.end()) {
        return iter->second->currentText();
    }

    return QString();
}

QVariantMap FileDialogStatusBar::allComboBoxsValue() const
{
    QVariantMap map;

    for (auto i : customComboBoxList)
        map[i.first->text()] = i.second->currentText();

    return map;
}

void FileDialogStatusBar::beginAddCustomWidget()
{
    for (auto i : customLineEditList) {
        i.first->deleteLater();
        i.second->deleteLater();
    }

    for (auto i : customLineEditList) {
        i.first->deleteLater();
        i.second->deleteLater();
    }

    customComboBoxList.clear();
    customLineEditList.clear();
}

void FileDialogStatusBar::endAddCustomWidget()
{
    updateLayout();
}

void FileDialogStatusBar::changeFileNameEditText(const QString &fileName)
{
    if (!fileNameEdit)
        return;

    QMimeDatabase db;
    QString newFileName;
    QString suffix = db.suffixForFileName(fileNameEdit->text());
    if (!suffix.isEmpty())
        newFileName = fileName + "." + suffix;
    else
        newFileName = fileName;

    fileNameEdit->setText(newFileName);
}

void FileDialogStatusBar::onWindowTitleChanged(const QString &title)
{
    if (title.isEmpty())
        return;

    QFontMetrics fontWidth(titleLabel->font());
    QString elideTitle = fontWidth.elidedText(title, Qt::ElideMiddle, UISize::kTitleMaxWidth);
    titleLabel->setText(elideTitle);

    titleLabel->setObjectName(title);
}

void FileDialogStatusBar::onFileNameTextEdited(const QString &text)
{
    QString dstText = DFMBASE_NAMESPACE::FileUtils::preprocessingFileName(text);
    QString suffix { "" };
    mainWindow->checkFileSuffix(dstText, suffix);
    int maxLength = suffix.isEmpty() ? NAME_MAX : NAME_MAX - suffix.length() - 1;
    while (DFMBASE_NAMESPACE::FileUtils::getFileNameLength(mainWindow->getcurrenturl(), dstText) > maxLength) {
        dstText.chop(1);
    }
    if (text != dstText) {
        int currPos = fileNameEdit->lineEdit()->cursorPosition();
        fileNameEdit->setText(dstText);
        currPos += dstText.length() - text.length();
        fileNameEdit->lineEdit()->setCursorPosition(currPos);
    }
}

void FileDialogStatusBar::initializeUi()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFrameShape(QFrame::NoFrame);

    DFrame *line = new DFrame(this);
    line->setLineWidth(0);
    line->setMidLineWidth(0);
    line->setFrameShape(QFrame::HLine);

    titleLabel = new DLabel(this);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(titleLabel), AcName::kAcFDStatusBarTitleLabel);
#endif
    QString labelName = tr("File Name");
    QString labelFilters = tr("Format");
    fileNameLabel = new DLabel(labelName, this);
    filtersLabel = new DLabel(labelFilters, this);

    fileNameLabel->setObjectName(labelName);
    filtersLabel->setObjectName(labelFilters);

    fileNameEdit = new DLineEdit(this);
    filtersComboBox = new DComboBox(this);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(fileNameEdit), AcName::kAcFDStatusBarFileNameEdit);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(filtersComboBox), AcName::kAcFDStatusBarFilters);
#endif

    fileNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    fileNameEdit->installEventFilter(this);
    fileNameEdit->setClearButtonEnabled(false);

    filtersComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QScrollBar *scrollBar = new QScrollBar(filtersComboBox);
    DListView *itemView = new DListView();
    itemView->setItemRadius(0);
    itemView->setItemSpacing(0);
    itemView->setHorizontalScrollBar(scrollBar);
    filtersComboBox->setView(itemView);
    itemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    curAcceptButton = new DSuggestButton(this);
    curRejectButton = new DPushButton(tr("Cancel", "button"), this);

    curRejectButton->setObjectName(tr("Cancel", "button"));

    curAcceptButton->setMinimumWidth(130);
    curRejectButton->setMinimumWidth(130);

    curAcceptButton->setObjectName("FileDialogStatusBarAcceptButton");

    contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(10, 0, 10, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 10);
    mainLayout->addWidget(line);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addLayout(contentLayout);

    // 在布局未更新前，界面不应该被显示
    setVisible(false);
}

void FileDialogStatusBar::initializeConnect()
{
    connect(fileNameEdit, &DLineEdit::textEdited, this, &FileDialogStatusBar::onFileNameTextEdited);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        updateLayout();
    });
#endif
}

void FileDialogStatusBar::updateLayout()
{
    if (curMode == kUnknow)
        return;

    if (!mainWindow || !mainWindow->centralWidget())
        return;

    setVisible(true);
    while (contentLayout->count() > 0)
        delete contentLayout->takeAt(0);

    fileNameLabel->hide();
    filtersLabel->hide();

    fileNameEdit->hide();
    filtersComboBox->hide();

    int widgetCount = customComboBoxList.count() + customLineEditList.count();

    if (curMode == kSave) {
        ++widgetCount;
    }

    if (filtersComboBox->count() > 0)
        ++widgetCount;

    if (widgetCount <= 1) {
        int addedWidgetCount = 1;

        if (!customLineEditList.isEmpty()) {
            contentLayout->addWidget(customLineEditList.first().first);
            contentLayout->addWidget(customLineEditList.first().second, 1);
        } else if (!customComboBoxList.isEmpty()) {
            contentLayout->addWidget(customComboBoxList.first().first);
            contentLayout->addWidget(customComboBoxList.first().second, 1);
        } else {
            addedWidgetCount = 0;
        }

        if (curMode == kOpen) {
            if (addedWidgetCount == 0) {
                contentLayout->addWidget(filtersLabel);
                contentLayout->addWidget(filtersComboBox, 1);
            }

            if (filtersComboBox->count() > 0) {
                filtersLabel->show();
                filtersComboBox->show();
            } else {
                contentLayout->addStretch();
            }

            contentLayout->addWidget(curRejectButton);
            contentLayout->addWidget(curAcceptButton);

            mainWindow->centralWidget()->layout()->addWidget(this);
            return;
        }

        if (filtersComboBox->count() <= 0) {
            if (addedWidgetCount == 0) {
                contentLayout->addWidget(fileNameLabel);
                contentLayout->addWidget(fileNameEdit);
            }
            contentLayout->addWidget(curRejectButton);
            contentLayout->addWidget(curAcceptButton);

            fileNameLabel->show();
            fileNameEdit->show();
            mainWindow->centralWidget()->layout()->addWidget(this);
            return;
        }
    }

    QVBoxLayout *labelLayout = new QVBoxLayout();
    labelLayout->setSpacing(10);
    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setSpacing(10);

    if (curMode == kSave) {
        labelLayout->addWidget(fileNameLabel);
        centerLayout->addWidget(fileNameEdit);
        fileNameLabel->show();
        fileNameEdit->show();
    }

    for (auto i : customLineEditList) {
        labelLayout->addWidget(i.first);
        centerLayout->addWidget(i.second);
    }

    if (filtersComboBox->count() > 0) {
        labelLayout->addWidget(filtersLabel);
        centerLayout->addWidget(filtersComboBox);
        filtersLabel->show();
        filtersComboBox->show();
    }

    for (auto i : customComboBoxList) {
        labelLayout->addWidget(i.first);
        centerLayout->addWidget(i.second);
    }

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(10);

    buttonLayout->addStretch();
    buttonLayout->addWidget(curRejectButton);
    buttonLayout->addWidget(curAcceptButton);
    buttonLayout->addStretch();

    contentLayout->addLayout(labelLayout);
    contentLayout->addLayout(centerLayout);
    contentLayout->addLayout(buttonLayout);
    mainWindow->centralWidget()->layout()->addWidget(this);
}

void FileDialogStatusBar::updateComboxViewWidth()
{
    QListView *listView = qobject_cast<QListView *>(filtersComboBox->view());
    if (listView) {
        QWidget *parent = qobject_cast<QWidget *>(listView->parent());
        if (parent)
            parent->setFixedWidth(filtersComboBox->width());
    }
}

void FileDialogStatusBar::setAppropriateWidgetFocus()
{
    // INFO: [FileDialogStatusBar::setAppropriateWidgetFocus] Setting focus to filename edit for better input experience.
    if (fileNameEdit && fileNameEdit->isVisible()) {
        // 无论是保存模式还是打开模式，都将焦点设置到文件名编辑框
        // 这样用户可以直接输入修改文件名，同时Enter键会触发相应的操作
        fileNameEdit->setFocus();
    }
}

void FileDialogStatusBar::showEvent(QShowEvent *event)
{
    const QString &title = window()->windowTitle();

    if (!title.isEmpty()) {
        titleLabel->setText(title);

        titleLabel->setObjectName(title);
    }
    connect(window(), &QWidget::windowTitleChanged, this, &FileDialogStatusBar::onWindowTitleChanged);

    setAppropriateWidgetFocus();

    updateComboxViewWidth();

    return QFrame::showEvent(event);
}

void FileDialogStatusBar::hideEvent(QHideEvent *event)
{
    disconnect(window(), &QWidget::windowTitleChanged, this, &FileDialogStatusBar::onWindowTitleChanged);

    return QFrame::hideEvent(event);
}

bool FileDialogStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != fileNameEdit)
        return false;

    if (event->type() == QEvent::FocusIn) {
        QTimer::singleShot(10, this, [this]() {
            DFMBASE_NAMESPACE::DMimeDatabase db;
            const QString &name = fileNameEdit->text();
            const QString &suffix = db.suffixForFileName(name);

            if (suffix.isEmpty())
                fileNameEdit->lineEdit()->selectAll();
            else
                fileNameEdit->lineEdit()->setSelection(0, name.length() - suffix.length() - 1);
        });
    } else if (event->type() == QEvent::Show) {
        QTimer::singleShot(500, this, [this]() {
            setAppropriateWidgetFocus();
        });
    }

    return false;
}
