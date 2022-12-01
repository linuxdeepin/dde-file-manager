/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "basicwidget.h"
#include "events/propertyeventcall.h"
#include "utils/propertydialogmanager.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/event/event.h>

#include <QFileInfo>
#include <QDateTime>
#include <QApplication>
#include <QSet>

Q_DECLARE_METATYPE(QList<QUrl> *)

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

BasicWidget::BasicWidget(QWidget *parent)
    : DArrowLineDrawer(parent)
{
    initUI();
    fileCalculationUtils = new FileStatisticsJob;
}

BasicWidget::~BasicWidget()
{
    fileCalculationUtils->deleteLater();
}

void BasicWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    setTitle(QString(tr("BasicInfo")));

    setExpand(true);

    frameMain = new QFrame(this);

    fileSize = new KeyValueLabel(frameMain);
    fileSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSize->setLeftValue(tr("Size"), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileCount = new KeyValueLabel(frameMain);
    fileCount->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCount->setLeftValue(tr("Contains"), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileCount->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileType = new KeyValueLabel(frameMain);
    fileType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileType->setLeftValue(tr("Type"), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileType->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    filePosition = new KeyValueLabel(frameMain);
    filePosition->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    filePosition->setLeftValue(tr("Location"), Qt::ElideMiddle, Qt::AlignLeft, true);
    filePosition->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileCreated = new KeyValueLabel(frameMain);
    fileCreated->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCreated->setLeftValue(tr("Time created"), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileCreated->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileAccessed = new KeyValueLabel(frameMain);
    fileAccessed->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileAccessed->setLeftValue(tr("Time accessed"), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileAccessed->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    fileModified = new KeyValueLabel(frameMain);
    fileModified->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileModified->setLeftValue(tr("Time modified"), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileModified->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);

    hideFile = new QCheckBox(frameMain);
    hideFile->setText(tr("Hide this file"));
}

void BasicWidget::basicExpand(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> fieldCondition = PropertyDialogManager::instance().createBasicViewExtensionField(url);

    QList<BasicExpandType> keys = fieldCondition.keys();
    for (BasicExpandType key : keys) {
        BasicExpandMap expand = fieldCondition.value(key);
        QList<BasicFieldExpandEnum> filterEnumList = expand.keys();
        switch (key) {
        case kFieldInsert: {
            for (BasicFieldExpandEnum k : filterEnumList) {
                QList<QPair<QString, QString>> fieldlist = expand.values(k);
                for (QPair<QString, QString> field : fieldlist) {
                    KeyValueLabel *expandLabel = new KeyValueLabel(this);
                    expandLabel->setLeftValue(field.first, Qt::ElideMiddle, Qt::AlignLeft, true);
                    expandLabel->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
                    expandLabel->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignVCenter, true);
                    expandLabel->setRightFontSizeWeight(DFontSizeManager::SizeType::T8);
                    fieldMap.insert(k, expandLabel);
                }
            }
        } break;
        case kFieldReplace: {
            for (BasicFieldExpandEnum k : filterEnumList) {
                QPair<QString, QString> field = expand.value(k);
                fieldMap.value(k)->setLeftValue(field.first, Qt::ElideMiddle, Qt::AlignLeft, true);
                fieldMap.value(k)->setRightValue(field.second, Qt::ElideMiddle, Qt::AlignVCenter, true);
            }
        } break;
        }
    }

    QLabel *label = new QLabel(frameMain);
    label->setFixedSize(hideFile->size());
    QGridLayout *gl = new QGridLayout;
    gl->setMargin(0);
    gl->addWidget(label, 0, 0, 1, 2);
    gl->addWidget(hideFile, 0, 1, 1, 5);
    gl->setColumnStretch(0, 2);
    gl->setColumnStretch(1, 3);
    QFrame *tempFrame = new QFrame(frameMain);
    tempFrame->setLayout(gl);

    QGridLayout *glayout = new QGridLayout;
    glayout->setContentsMargins(15, 15, 5, 10);
    glayout->setSpacing(16);
    int row = 0;
    QList<BasicFieldExpandEnum> fields = fieldMap.keys();
    QSet<BasicFieldExpandEnum> fieldset = QSet<BasicFieldExpandEnum>::fromList(fields);
    fields = fieldset.toList();
    qSort(fields.begin(), fields.end());
    for (BasicFieldExpandEnum &key : fields) {
        QList<KeyValueLabel *> kvls = fieldMap.values(key);
        for (int i = kvls.count() - 1; i >= 0; --i) {
            KeyValueLabel *kvl = kvls[i];
            glayout->addWidget(kvl, row, 0, 1, 6);
            row++;
        }
    }
    QStringList list = url.path().split("/", QString::SkipEmptyParts);
    if (!list.isEmpty() && url.isValid() && list.last().startsWith("."))
        tempFrame->hide();
    else
        glayout->addWidget(tempFrame, row, 0, 1, 6);
    glayout->setColumnStretch(0, 1);

    frameMain->setLayout(glayout);
    setContent(frameMain);
}

void BasicWidget::basicFieldFilter(const QUrl &url)
{
    PropertyFilterType fieldFilter = PropertyDialogManager::instance().basicFiledFiltes(url);
    if (fieldFilter & PropertyFilterType::kFileSizeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileSize);
        fileSize->deleteLater();
        fileSize = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileTypeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileType);
        fileType->deleteLater();
        fileType = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileCountFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileCount);
        fileCount->deleteLater();
        fileCount = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFilePositionFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFilePosition);
        filePosition->deleteLater();
        filePosition = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileCreateTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileCreateTime);
        fileCreated->deleteLater();
        fileCreated = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileAccessedTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileAccessedTime);
        fileAccessed->deleteLater();
        fileAccessed = nullptr;
    } else if (fieldFilter & PropertyFilterType::kFileModifiedTimeFiled) {
        fieldMap.remove(BasicFieldExpandEnum::kFileModifiedTime);
        fileModified->deleteLater();
        fileModified = nullptr;
    }
}

void BasicWidget::basicFill(const QUrl &url)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;
    if (!info->canHidden())
        hideFile->setEnabled(false);

    if (info->isHidden())
        hideFile->setChecked(true);

    connect(hideFile, &QCheckBox::stateChanged, this, &BasicWidget::slotFileHide);

    if (filePosition && filePosition->RightValue().isEmpty()) {
        filePosition->setRightValue(info->isSymLink() ? info->symLinkTarget() : url.path(), Qt::ElideMiddle, Qt::AlignVCenter, true);
        if (info->isSymLink()) {
            auto &&symlink = info->symLinkTarget();
            connect(filePosition, &KeyValueLabel::valueAreaClicked, this, [symlink] {
                const QUrl &url = QUrl::fromLocalFile(symlink);
                const auto &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
                QUrl parentUrl = fileInfo->parentUrl();
                parentUrl.setQuery("selectUrl=" + url.toString());
                dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, parentUrl);
            });
        }
    }

    if (fileCreated && fileCreated->RightValue().isEmpty()) {
        info->birthTime().isValid() ? fileCreated->setRightValue(info->birthTime().toString(FileUtils::dateTimeFormat()), Qt::ElideNone, Qt::AlignVCenter, true)
                                    : fileCreated->setVisible(false);
    }
    if (fileAccessed && fileAccessed->RightValue().isEmpty()) {
        info->lastRead().isValid() ? fileAccessed->setRightValue(info->lastRead().toString(FileUtils::dateTimeFormat()), Qt::ElideNone, Qt::AlignVCenter, true)
                                   : fileAccessed->setVisible(false);
    }
    if (fileModified && fileModified->RightValue().isEmpty()) {
        info->lastModified().isValid() ? fileModified->setRightValue(info->lastModified().toString(FileUtils::dateTimeFormat()), Qt::ElideNone, Qt::AlignVCenter, true)
                                       : fileModified->setVisible(false);
    }
    if (fileSize && fileSize->RightValue().isEmpty()) {
        fSize = info->size();
        fCount = 1;
        fileSize->setRightValue(FileUtils::formatSize(fSize), Qt::ElideNone, Qt::AlignVCenter, true);
    }
    if (fileCount && fileCount->RightValue().isEmpty())
        fileCount->setVisible(false);

    if (fileType && fileType->RightValue().isEmpty()) {
        QUrl localUrl = url;
        QList<QUrl> urls {};
        bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << localUrl, &urls);
        if (ok && !urls.isEmpty())
            localUrl = urls.first();

        QMimeType mimeType = MimeDatabase::mimeTypeForUrl(localUrl);
        MimeDatabase::FileType type = MimeDatabase::mimeFileTypeNameToEnum(mimeType.name());
        switch (type) {
        case MimeDatabase::FileType::kDirectory: {
            fileType->setRightValue(tr("Directory") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
            if (fileCount && fileCount->RightValue().isEmpty()) {
                fileCount->setVisible(true);
                fileCount->setRightValue(tr("%1 item").arg(0), Qt::ElideNone, Qt::AlignVCenter, true);
                fileCalculationUtils->start(QList<QUrl>() << localUrl);
                connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &BasicWidget::slotFileCountAndSizeChange);
            }
        } break;
        case MimeDatabase::FileType::kDocuments: {
            fileType->setRightValue(tr("Documents") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kVideos: {
            fileType->setRightValue(tr("Videos") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kImages: {
            fileType->setRightValue(tr("Images") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kAudios: {
            fileType->setRightValue(tr("Audios") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kExecutable: {
            fileType->setRightValue(tr("Executable") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kArchives: {
            fileType->setRightValue(tr("Archives") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kDesktopApplication: {
            fileType->setRightValue(tr("application") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        case MimeDatabase::FileType::kUnknown: {
            fileType->setRightValue(tr("Unknown") + "(" + mimeType.name() + ")", Qt::ElideMiddle, Qt::AlignVCenter, true);
        } break;
        default:
            break;
        }
    }
}

void BasicWidget::initFileMap()
{
    fieldMap.insert(BasicFieldExpandEnum::kFileSize, fileSize);
    fieldMap.insert(BasicFieldExpandEnum::kFileCount, fileCount);
    fieldMap.insert(BasicFieldExpandEnum::kFileType, fileType);
    fieldMap.insert(BasicFieldExpandEnum::kFilePosition, filePosition);
    fieldMap.insert(BasicFieldExpandEnum::kFileCreateTime, fileCreated);
    fieldMap.insert(BasicFieldExpandEnum::kFileAccessedTime, fileAccessed);
    fieldMap.insert(BasicFieldExpandEnum::kFileModifiedTime, fileModified);
}

void BasicWidget::selectFileUrl(const QUrl &url)
{
    currentUrl = url;

    initFileMap();

    basicFieldFilter(url);

    basicExpand(url);

    basicFill(url);
}

qint64 BasicWidget::getFileSize()
{
    return fSize;
}

int BasicWidget::getFileCount()
{
    return fCount;
}

void BasicWidget::slotFileCountAndSizeChange(qint64 size, int filesCount, int directoryCount)
{
    fSize = size;
    fileSize->setRightValue(FileUtils::formatSize(size), Qt::ElideNone, Qt::AlignVCenter, true);

    fCount = filesCount + (directoryCount > 1 ? directoryCount - 1 : 0);
    QString txt = fCount > 1 ? tr("%1 items") : tr("%1 item");
    fileCount->setRightValue(txt.arg(fCount), Qt::ElideNone, Qt::AlignVCenter, true);
}

void BasicWidget::slotFileHide(int state)
{
    Q_UNUSED(state)
    quint64 winIDs = QApplication::activeWindow()->winId();
    PropertyEventCall::sendFileHide(winIDs, { currentUrl });
}

void BasicWidget::closeEvent(QCloseEvent *event)
{
    DArrowLineDrawer::closeEvent(event);
}
