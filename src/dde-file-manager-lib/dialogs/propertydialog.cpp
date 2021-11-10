/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

//fixed:CD display size error
#include "views/dfmopticalmediawidget.h"

#include "propertydialog.h"
#include "dabstractfilewatcher.h"
#include "dfileinfo.h"
#include <sys/stat.h>

#include "app/define.h"

#include "dfileservices.h"
#include "dfilestatisticsjob.h"

#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "shutil/dfmfilelistfile.h"

#include "dialogs/dialogmanager.h"
#include "app/filesignalmanager.h"

#include "models/dfmrootfileinfo.h"
#include "deviceinfo/udisklistener.h"

#include "controllers/vaultcontroller.h"
#include "vaultglobaldefine.h"

#include "utils.h"

#include "singleton.h"

#include "shareinfoframe.h"
#include "views/dfilemanagerwindow.h"
#include "views/windowmanager.h"
#include "views/dfileview.h"
#include "interfaces/dfilesystemmodel.h"

#include "plugins/pluginmanager.h"
#include "../plugininterfaces/menu/menuinterface.h"
#include "dfmeventdispatcher.h"
#include "views/dfmsidebar.h"
#include "dfmapplication.h"
#include "dstorageinfo.h"
#include "dfmsettings.h"

#include <DDrawer>
#include <DDrawerGroup>
#include <DArrowLineDrawer>
#include <dblockdevice.h>
#include <denhancedwidget.h>
#include <DColoredProgressBar>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <QScrollBar>
#include <QWindow>
#include <QTextEdit>
#include <QFormLayout>
#include <QDateTime>
#include <QCheckBox>
#include <QComboBox>
#include <QTimer>
#include <QThread>
#include <QListWidget>
#include <QButtonGroup>
#include <QProgressBar>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStackedWidget>
#include <QStorageInfo>
#include <QPainterPath>
//#include <QVariantAnimation>
#include <QScrollArea>
#include <ddiskmanager.h>
#include <QGuiApplication>
#include <unistd.h>
#include <models/trashfileinfo.h>
#include <views/dfmtagwidget.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <dgiosettings.h>

#define ArrowLineExpand_HIGHT   30
#define ArrowLineExpand_SPACING 10

bool DFMRoundBackground::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent() && event->type() == QEvent::Paint) {
        QWidget *w = dynamic_cast<QWidget *>(watched);
        if (!w) {
            return false;
        }
        int radius = property("radius").toInt();

        QPainter painter(w);
        QRectF bgRect;
        bgRect.setSize(w->size());
        const QPalette pal = QGuiApplication::palette();
        QColor bgColor = pal.color(QPalette::Base);

        QPainterPath path;
        path.addRoundedRect(bgRect, radius, radius);
        // drawbackground color
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, bgColor);
        painter.setRenderHint(QPainter::Antialiasing, false);
        return true;
    }

    return QObject::eventFilter(watched, event);
}

NameTextEdit::NameTextEdit(const QString &text, QWidget *parent):
    QTextEdit(text, parent)
{
    setObjectName("NameTextEdit");
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setFixedSize(200, 60);
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, &QTextEdit::textChanged, this, [this] {
        QSignalBlocker blocker(this);
        Q_UNUSED(blocker)

        QString text = this->toPlainText();
        const QString old_text = text;

        int text_length = text.length();

        text.remove('/');
        text.remove(QChar(0));

        QVector<uint> list = text.toUcs4();
        int cursor_pos = this->textCursor().position() - text_length + text.length();

        while (text.toLocal8Bit().count() > MAX_FILE_NAME_CHAR_COUNT)
        {
            text.chop(1);
        }

        if (text.count() != old_text.count())
        {
            this->setText(text);
        }

        QTextCursor cursor = this->textCursor();

        cursor.movePosition(QTextCursor::Start);

        do
        {
            QTextBlockFormat format = cursor.blockFormat();

            format.setLineHeight(TEXT_LINE_HEIGHT, QTextBlockFormat::FixedHeight);
            cursor.setBlockFormat(format);
        } while (cursor.movePosition(QTextCursor::NextBlock));

        cursor.setPosition(cursor_pos);

        this->setTextCursor(cursor);
        this->setAlignment(Qt::AlignHCenter);

        if (this->isReadOnly())
            this->setFixedHeight(this->document()->size().height());
    });
}

void NameTextEdit::setPlainText(const QString &text)
{
    QTextEdit::setPlainText(text);
    setAlignment(Qt::AlignCenter);
}

void NameTextEdit::focusOutEvent(QFocusEvent *event)
{
    QTextEdit::focusOutEvent(event);
}

void NameTextEdit::keyPressEvent(QKeyEvent *event)
{
    QEvent::Type ty = event->type();
    Q_UNUSED(ty)
    Qt::KeyboardModifiers modifiers = event->modifiers();
    Q_UNUSED(modifiers)
    if (event->key() == Qt::Key_Escape) {
        setIsCanceled(true);
        emit editFinished();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        setIsCanceled(false);
        emit editFinished();
    }
    QTextEdit::keyPressEvent(event);
}

bool NameTextEdit::isCanceled() const
{
    return m_isCanceled;
}

void NameTextEdit::setIsCanceled(bool isCanceled)
{
    m_isCanceled = isCanceled;
}


GroupTitleLabel::GroupTitleLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("GroupTitleLabel");
}

SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionKeyLabel");
    setFixedWidth(120);
    QFont font = this->font();
    font.setWeight(QFont::Bold - 8);
    font.setPixelSize(13);
    setFont(font);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}


SectionValueLabel::SectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionValueLabel");
    setFixedWidth(150);
    QFont font = this->font();
    font.setPixelSize(12);
    setFont(font);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setWordWrap(true);
}

LinkSectionValueLabel::LinkSectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    SectionValueLabel(text, parent, f)
{

}

void LinkSectionValueLabel::mouseReleaseEvent(QMouseEvent *event)
{
    DFMEventDispatcher::instance()->processEvent<DFMOpenFileLocation>(Q_NULLPTR, linkTargetUrl());
    SectionValueLabel::mouseReleaseEvent(event);
}

DUrl LinkSectionValueLabel::linkTargetUrl() const
{
    return m_linkTargetUrl;
}

void LinkSectionValueLabel::setLinkTargetUrl(const DUrl &linkTargetUrl)
{
    m_linkTargetUrl = linkTargetUrl;
}


PropertyDialog::PropertyDialog(const DFMEvent &event, const DUrl url, QWidget *parent)
    : DDialog(parent)
    , m_fmevent(event)
    , m_url(url)
    , m_icon(new QLabel)
    , m_edit(new NameTextEdit)
    , m_platformWindowHandle(new DPlatformWindowHandle(this, this))
{
    //允许窗口拖拽缩放
    m_platformWindowHandle->setEnableSystemResize(true);

    if (DFMGlobal::isWayLand()) {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    } else {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowFlags(windowFlags()
                       & ~ Qt::WindowMaximizeButtonHint
                       & ~ Qt::WindowMinimizeButtonHint
                       & ~ Qt::WindowSystemMenuHint);
    }

    QString basicInfo = tr("Basic info");
    QString openWith = tr("Open with");
    QString shareManager = tr("Sharing");
    QString authManager = tr("Permissions");
    initUI();
    QString query = m_url.query();

    if (m_url.scheme() == DFMROOT_SCHEME) {
        //dgvfsinfo有缓存的才去刷新缓存的信息
        DAbstractFileInfoPointer fi = DAbstractFileInfo::getFileInfo(m_url);
        if (fi) {
            fi->refresh(fi->isGvfsMountFile());
        } else {
            fi = fileService->createFileInfo(this, m_url);
        }
        Q_ASSERT(fi);

        QString name = fi->fileDisplayName();
        QIcon icon = QIcon::fromTheme(fi->iconName());

        m_icon->setPixmap(icon.pixmap(128, 128));
        m_edit->setPlainText(name);
        m_editDisbaled = true;
        const QList<QPair<QString, QString> > &properties = createLocalDeviceInfoWidget(fi);
        m_deviceInfoFrame = createInfoFrame(properties);

        QStringList titleList;
        titleList << basicInfo;
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup.at(0)->setContent(m_deviceInfoFrame);
        m_expandGroup.at(0)->setExpand(true);

        uint64_t dskspace = fi->extraProperties()["fsSize"].toULongLong();
        uint64_t dskinuse = fi->extraProperties()["fsUsed"].toULongLong();
        QString devid(fi->suffix() == SUFFIX_GVFSMP ? fi->fileDisplayName() : fi->baseName());
        if (devid == name) {
            devid.clear();
        }

        DColoredProgressBar *progbdf = new DTK_WIDGET_NAMESPACE::DColoredProgressBar();
        // fix bug#47111 由于addThreshold接口不支持渐变色类（QLinearGradient），暂时采用固定颜色
        progbdf->addThreshold(0, QColor(0xFF0080FF));
        progbdf->addThreshold(7000, QColor(0xFFFFAE00));
        progbdf->addThreshold(9000, QColor(0xFFFF0000));

        //fixed:CD display size error
        if (static_cast<DFMRootFileInfo::ItemType>(fi->fileType()) == DFMRootFileInfo::ItemType::UDisksOptical) {
            DFMRootFileInfo *pFileInfo = dynamic_cast<DFMRootFileInfo *>(fi.data());
            QString strVolTag;
            if (pFileInfo)
                strVolTag = pFileInfo->getVolTag();
            dskspace = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nTotal;
            dskinuse = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nUsage;
        }

        progbdf->setMaximum(10000);
        progbdf->setValue(dskspace && ~dskinuse ? int(10000. * dskinuse / dskspace) : 0);
        progbdf->setMaximumHeight(8);
        progbdf->setTextVisible(false);

        // fix bug#47111 在浅色模式下，手动设置进度条背景色
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            DPalette palette = progbdf->palette();
            palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
            DApplicationHelper::instance()->setPalette(progbdf, palette);
        }

        // 进度条背景色跟随主题变化而变化
        connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ](DGuiApplicationHelper::ColorType type) {
            DPalette palette = progbdf->palette();
            if (type == DGuiApplicationHelper::LightType) {
                palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
                DApplicationHelper::instance()->setPalette(progbdf, palette);
            } else {
                palette.setBrush(DPalette::ObviousBackground, QColor("#4e4e4e"));
                DApplicationHelper::instance()->setPalette(progbdf, palette);
            }
        });

        //fix 没有devid则只显示名称
        QString text = devid.isEmpty() ? tr("%1").arg(name) : tr("%1 (%2)").arg(name).arg(devid);
        //end

        QLabel *lbdf_l = new SectionKeyLabel();
        QString fullText(text);
        text = lbdf_l->fontMetrics().elidedText(text, Qt::ElideMiddle, 150);
        lbdf_l->setText(text);
        if (text != fullText) {
            lbdf_l->setToolTip(fullText);
        }
        QLabel *lbdf_r = new SectionKeyLabel(tr("%1 / %2").arg(FileUtils::formatSize(dskinuse)).arg(FileUtils::formatSize(dskspace)));
        if (!~dskinuse) {
            lbdf_r->setText(FileUtils::formatSize(dskspace));
        }
        lbdf_l->setAlignment(Qt::AlignLeft);
        lbdf_r->setAlignment(Qt::AlignRight);
        lbdf_l->setMaximumWidth(QWIDGETSIZE_MAX);
        lbdf_r->setMaximumWidth(QWIDGETSIZE_MAX);
        QWidget *wdfl = new QWidget();
        wdfl->setLayout(new QHBoxLayout);
        wdfl->layout()->setMargin(0);
        wdfl->layout()->addWidget(lbdf_l);
        wdfl->layout()->addWidget(lbdf_r);

        m_wdf = new QFrame(this);
        m_wdf->setLayout(new QVBoxLayout);
        m_wdf->layout()->setMargin(0);
        m_wdf->layout()->setContentsMargins(12, 8, 12, 8);
        m_wdf->layout()->addWidget(wdfl);
        m_wdf->layout()->addWidget(progbdf);
        new DFMRoundBackground(m_wdf, 8);
        qobject_cast<QVBoxLayout *>(m_scrollArea->widget()->layout())->insertWidget(0, m_wdf);

    } else {
        // tagged file basicinfo not complete??
        DUrl realUrl = m_url.isTaggedFile() ? DUrl::fromLocalFile(m_url.fragment(QUrl::FullyDecoded)) : m_url;

        //! bug#40608 解决通过标记访问保险箱，属性菜单显示不正确的问题.
        bool isVaultFile = VaultController::isRootDirectory(realUrl.toLocalFile());
        if (isVaultFile) {
            //! set scheme to get vault file info.
            realUrl.setScheme(DFMVAULT_SCHEME);
        }
        //dgvfsinfo有缓存的才去刷新缓存的信息
        DAbstractFileInfoPointer fileInfo = DAbstractFileInfo::getFileInfo(m_url);
        if (fileInfo) {
            fileInfo->refresh(fileInfo->isGvfsMountFile());
        } else {
            fileInfo = DFileService::instance()->createFileInfo(this, realUrl);
        }
        if (!fileInfo) {
            close();
            return;
        }

        m_icon->setPixmap(fileInfo->fileIcon().pixmap(128, 128));
        m_edit->setPlainText(fileInfo->fileDisplayName());
        m_edit->setAlignment(Qt::AlignHCenter);

        if (!fileInfo->canRename()) {
            m_editDisbaled = true;
        }

        m_basicInfoFrame = createBasicInfoWidget(fileInfo);

        QStringList titleList;
        if (fileInfo->isFile()) {
            titleList << basicInfo;
            //在回收站搜索文件，需要使搜索结果的文件属性面板与回收站文件属性面板保持一致
            if (!m_url.isTrashFile() && !(m_url.isSearchFile() && fileInfo->redirectedFileUrl().isTrashFile())) {
                titleList << openWith;
                if (fileInfo->canManageAuth())
                    titleList << authManager;
            }
        } else {
            titleList << basicInfo;
            //! 选中的文件是保险箱的，则屏蔽掉共享选项
            if (fileInfo->canShare() && !VaultController::isVaultFile(fileInfo->toQFileInfo().canonicalFilePath())) {
                titleList << shareManager;
            }
            if (!fileInfo->isVirtualEntry() && !m_url.isTrashFile() && fileInfo->canManageAuth() &&
                    !VaultController::ins()->isRootDirectory(m_url.toLocalFile())) {
                //在回收站搜索文件，需要使搜索结果的文件属性面板与回收站文件属性面板保持一致
                if (m_url.isSearchFile()) {
                    if (!fileInfo->redirectedFileUrl().isTrashFile()) {
                        titleList << authManager;
                    }
                } else {
                    titleList << authManager;
                }
            }
        }
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup.at(0)->setContent(m_basicInfoFrame);

        if (fileInfo->isDir()) {
            if (fileInfo->canShare()) {
                m_shareinfoFrame = createShareInfoFrame(fileInfo);
                m_expandGroup.at(1)->setContent(m_shareinfoFrame);
                m_expandGroup.at(1)->setExpand(false);
            }

            if (fileInfo->toLocalFile().isEmpty()) {
                startComputerFolderSize(m_url);
            } else if (fileInfo->isSymLink()) {
                startComputerFolderSize(fileInfo->redirectedFileUrl());
            } else {
                startComputerFolderSize(DUrl::fromLocalFile(fileInfo->toLocalFile()));
            }

            m_fileCount = fileInfo->filesCount();
        } else {
            m_fileCount = 1;
            m_size = fileInfo->size();

            int openWithIndex = titleList.indexOf(openWith);
            if (openWithIndex != -1) {
                m_OpenWithListWidget = createOpenWithListWidget(fileInfo);
                m_expandGroup.at(openWithIndex)->setContent(m_OpenWithListWidget);
                m_expandGroup.at(openWithIndex)->setExpand(false);
            }
        }

        int authMgrIndex = titleList.indexOf(authManager);
        if (authMgrIndex != -1) {
            m_authorityManagementFrame = createAuthorityManagementWidget(fileInfo);
            m_expandGroup.at(authMgrIndex)->setContent(m_authorityManagementFrame);
            m_expandGroup.at(authMgrIndex)->setExpand(false);
        }
    }
    initTextShowFrame(m_edit->toPlainText());
    if (m_editDisbaled) {
        m_editButton->hide();
    }

    loadPluginExpandWidgets();

    m_expandGroup.first()->setExpand(true);

    initConnect();
}

void PropertyDialog::initUI()
{
    m_icon->setFixedHeight(128);
    m_icon->setParent(this);
    QFrame *m_editFrame = new QFrame;

    QHBoxLayout *editLayout = new QHBoxLayout;
    editLayout->addStretch();
    editLayout->addWidget(m_edit);
    editLayout->addStretch();
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(0, 0, 0, 0);
    m_editFrame->setLayout(editLayout);
    m_edit->setParent(m_editFrame);

    m_editStackWidget = new QStackedWidget(this);
    m_editStackWidget->addWidget(m_editFrame);

    m_mainLayout = new QVBoxLayout;

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_icon, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(m_editStackWidget, 0, Qt::AlignHCenter | Qt::AlignTop);

    QFrame *frame = new QFrame(this);
    frame->setLayout(m_mainLayout);
    addContent(frame);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setObjectName("PropertyDialog-QScrollArea");
    QPalette palette = m_scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    m_scrollArea->viewport()->setPalette(palette);
    m_scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    QFrame *infoframe = new QFrame;
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    // 修复BUG-47113 UI显示不对
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 20);
    scrollWidgetLayout->setSpacing(ArrowLineExpand_SPACING);
    infoframe->setLayout(scrollWidgetLayout);
    m_scrollArea->setWidget(infoframe);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    QVBoxLayout *scrolllayout = new QVBoxLayout;
    scrolllayout->addWidget(m_scrollArea);
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    layout->addLayout(scrolllayout, 1);

    QFrame *tagFrame = initTagFrame(m_url);
    if (tagFrame != nullptr) {
        scrollWidgetLayout->addWidget(tagFrame);
    }

    setFixedWidth(350);
}

QFrame *PropertyDialog::initTagFrame(const DUrl &url)
{
    if (m_tagInfoFrame != nullptr) {
        ((DFMTagWidget *)m_tagInfoFrame)->loadTags(url);
        return m_tagInfoFrame;
    }

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, url);
    DUrl t_url = url;
    //! 保险箱中属性窗口中要获取标记需要真实路径，需要将虚拟路径转换为真实路径
    if (url.isVaultFile()) {
        t_url = VaultController::vaultToLocalUrl(url);
    }
    if (fileInfo && fileInfo->canTag()) {
        DFMTagWidget *tagInfoFrame = new DFMTagWidget(t_url, this);
        new DFMRoundBackground(tagInfoFrame, 8);
        m_tagInfoFrame = tagInfoFrame;

        QFont font = tagInfoFrame->tagTitle()->font();
        font.setBold(true);
        font.setPixelSize(17);
        tagInfoFrame->tagTitle()->setFont(font);
        m_tagInfoFrame->setMaximumHeight(150);

        qDebug() << "tag frame is created for: " << t_url;

        return m_tagInfoFrame;
    }

    return nullptr;
}

void PropertyDialog::updateInfo()
{
    initTagFrame(m_url);
}

const DUrl PropertyDialog::getRealUrl()
{
    if (m_url.isRecentFile()) {
        return DUrl::fromLocalFile(m_url.path());
    }

    return m_url;
}

bool PropertyDialog::canChmod(const DAbstractFileInfoPointer &info)
{
    bool ret = true;

    if (info->scheme() == BURN_SCHEME || info->isGvfsMountFile())
        ret = false;

    if (!info->canRename() || !info->canManageAuth())
        ret = false;

    QString path = info->filePath();
    static QRegularExpression regExp("^/run/user/\\d+/gvfs/.+$",
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);
    if (regExp.match(path, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch())
        ret = false;

    return ret;
}

void PropertyDialog::initConnect()
{
    connect(m_edit, &NameTextEdit::editFinished, this, &PropertyDialog::showTextShowFrame);
    connect(m_edit, &NameTextEdit::textChanged, this, [this]() {
        QString text = m_edit->toPlainText();
        const QString old_text = text;

        text = DFMGlobal::preprocessingFileName(text);
        if (text.count() != old_text.count()) {
            m_edit->setPlainText(text);
        }
    });

    DAbstractFileWatcher *fileWatcher = DFileService::instance()->createFileWatcher(this, m_url, this);

    connect(fileWatcher, &DAbstractFileWatcher::fileDeleted, this, &PropertyDialog::onChildrenRemoved);
    connect(fileWatcher, &DAbstractFileWatcher::fileMoved, this, [this](const DUrl & from, const DUrl & to) {
        Q_UNUSED(to)

        onChildrenRemoved(from);
    });
}


void PropertyDialog::updateFolderSize(qint64 size)
{
    m_size = size;
    if(m_sizeWorker)
        m_fileCount = m_sizeWorker->filesCount() + m_sizeWorker->directorysCount(false);
    m_folderSizeLabel->setText(FileUtils::formatSize(size));
    m_containSizeLabel->setText(QString::number(m_fileCount));
}

void PropertyDialog::renameFile()
{
    bool donotShowSuffix{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() };

    const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, m_url);

    QString fileName;
    if (donotShowSuffix &&
            fileInfo->isFile() &&
            !fileInfo->suffix().isEmpty() && !fileInfo->isDesktopFile()) {
        fileName = fileInfo->baseNameOfRename();
    } else {
        fileName = fileInfo->fileNameOfRename();
    }

    m_edit->setPlainText(fileName);
    m_editStackWidget->setCurrentIndex(0);
    m_edit->setFixedHeight(m_textShowFrame->height());
    m_edit->setFocus();

    int endPos = -1;
    if (fileInfo->isFile()) {

        QString suffixStr{ fileInfo->suffix() };
        if (suffixStr.isEmpty() || donotShowSuffix || fileInfo->isDesktopFile()) {
            endPos = m_edit->toPlainText().length();
        } else if (m_edit->toPlainText().endsWith(suffixStr)) {
            endPos = m_edit->toPlainText().length() - fileInfo->suffix().length() - 1;
        }
    }
    if (endPos == -1) {
        m_edit->selectAll();
        endPos = m_edit->toPlainText().length();
    }
    QTextCursor cursor = m_edit->textCursor();
    cursor.setPosition(0);
    cursor.setPosition(endPos, QTextCursor::KeepAnchor);
    m_edit->setTextCursor(cursor);
}

void PropertyDialog::showTextShowFrame()
{
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, m_url);

    bool donotShowSuffix{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() };

    QString newName = m_edit->toPlainText();

    if (newName.trimmed().isEmpty()) {
        m_edit->setIsCanceled(true);
    }

    if (donotShowSuffix && fileInfo->isFile() &&
            !fileInfo->suffix().isEmpty() && !fileInfo->isDesktopFile()) {
        newName += "." + fileInfo->suffix();
    }

    if (m_edit->isCanceled()) {
        initTextShowFrame(fileInfo->fileDisplayName());
    } else {
        DUrl oldUrl = m_url;
        DUrl newUrl = fileInfo->getUrlByNewFileName(newName);

        if (oldUrl == newUrl) {
            m_editStackWidget->setCurrentIndex(1);
            return;
        }

        if (fileService->renameFile(this, oldUrl, newUrl)) {
            if (!fileInfo->isDesktopFile()) { // this is a dirty fix.
                m_url = newUrl;
                updateInfo();//bug 25419
                onHideFileCheckboxChecked(false);    //bug 29958
                if (m_basicInfoFrame) { //bug 29961
                    QCheckBox *hideThisFile = m_basicInfoFrame->findChild<QCheckBox *>(QString("hideThisFileCheckBox"));
                    if (hideThisFile) {
                        hideThisFile->setChecked(false);
                    }
                }

                dialogManager->refreshPropertyDialogs(oldUrl, newUrl);
            }
            const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, m_url);

            initTextShowFrame(info->fileDisplayName());

            if (m_shareinfoFrame) {
                m_shareinfoFrame->setFileinfo(info);
            }
        } else {
            m_editStackWidget->setCurrentIndex(1);
        }
    }
}

void PropertyDialog::onChildrenRemoved(const DUrl &fileUrl)
{
    if (m_url.isUserShareFile()) {
        return;
    }
    if (fileUrl == m_url) {
        QTimer::singleShot(100, this, [ = ] {
            this->close();
        });
        //        close();
    }
}

void PropertyDialog::flickFolderToSidebar(const DUrl &fileUrl)
{
    //! 只处理当前触发的对话框
    if (fileUrl.toLocalFile() != this->getUrl().toLocalFile()) {
        return;
    }

    DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(WindowManager::getWindowById(m_fmevent.windowId()));
    if (!window) {
        return;
    }

    //when current window is minimized,cancle animation
    if (window->windowState() == Qt::WindowMinimized) {
        return;
    }

    // we are actually using network group's center position as target position
    // since it's used as a animation target position, a fuzzy result is okay.
    DFMSideBar *sideBar = window->getLeftSideBar();
    QString groupName = DFMSideBar::groupName(DFMSideBar::GroupName::Network);
    sideBar->scrollToGroup(groupName);
    QPoint targetPos = sideBar->groupGeometry(groupName).center();

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, m_url);

    //QLabel *aniLabel = new QLabel(window);
    m_aniLabel = new QLabel(window);
    m_aniLabel->raise();
    m_aniLabel->setFixedSize(m_icon->size());
    m_aniLabel->setAttribute(Qt::WA_TranslucentBackground);
    m_aniLabel->setPixmap(fileInfo->fileIcon().pixmap(160, 160));
    m_aniLabel->move(window->mapFromGlobal(m_icon->mapToGlobal(m_icon->pos())));

    int angle;
    if (targetPos.x() > m_aniLabel->x()) {
        angle = 45;
    } else {
        angle = -45;
    }

    // QVariantAnimation *xani = new QVariantAnimation(this);
    m_xani = new QVariantAnimation(this);
    m_xani->setStartValue(m_aniLabel->pos());
    m_xani->setEndValue(QPoint(targetPos.x(), angle));
    if (DFMGlobal::isWayLand()) {
        m_xani->setDuration(700);
    } else {
        m_xani->setDuration(440);
    }

    //QVariantAnimation *gani = new QVariantAnimation(this);
    m_gani = new QVariantAnimation(this);
    m_gani->setStartValue(m_aniLabel->geometry());
    m_gani->setEndValue(QRect(targetPos.x(), targetPos.y(), 20, 20));
    m_gani->setEasingCurve(QEasingCurve::InBack);
    if (DFMGlobal::isWayLand()) {
        m_gani->setDuration(700);
    } else {
        m_gani->setDuration(440);
    }

    connect(m_xani, &QVariantAnimation::valueChanged, [ = ](const QVariant & val) {
        if (m_aniLabel) {
            m_aniLabel->move(QPoint(val.toPoint().x() - m_aniLabel->width() / 2, m_aniLabel->y()));
            QImage img = fileInfo->fileIcon().pixmap(m_aniLabel->size()).toImage();
            QMatrix ma;
            ma.rotate(val.toPoint().y());
            img = img.transformed(ma, Qt::SmoothTransformation);
            img = img.scaled(m_aniLabel->width() / 2, m_aniLabel->height() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_aniLabel->setPixmap(QPixmap::fromImage(img));
            if (m_aniLabel->isHidden()) {
                m_aniLabel->show();
            }
        }
    });
    connect(m_xani, &QVariantAnimation::finished, [ = ] {
        m_xani->deleteLater();
    });

    connect(m_gani, &QVariantAnimation::valueChanged, [ = ](const QVariant & val) {
        m_aniLabel->move(QPoint(m_aniLabel->x(),
                                val.toRect().y() - val.toRect().width() / 2));
        m_aniLabel->setFixedSize(val.toRect().size() * 2);
    });
    connect(m_gani, &QVariantAnimation::finished, [ = ] {
        m_gani->deleteLater();
        m_aniLabel->deleteLater();
    });
    m_xani->start();
    m_gani->start();
}
void PropertyDialog::onOpenWithBntsChecked(QAbstractButton *w)
{
    if (w) {
        MimesAppsManager::setDefautlAppForTypeByGio(w->property("mimeTypeName").toString(),
                                                    w->property("appPath").toString());
    }
}

void PropertyDialog::onHideFileCheckboxChecked(bool checked)
{
    DUrl url = getRealUrl();
    QFileInfo info(url.toLocalFile());
    if (!info.exists()) return;

    DFMFileListFile flf(info.absolutePath());
    const QString fileName = info.fileName();

    //隐藏属性无变动，则不保存文件。task#40201
    bool save = false;
    qDebug() << info.absolutePath();
    if (checked) {
        if (!flf.contains(fileName)) {
            flf.insert(fileName);
            save = true;
        }
    } else {
        if (flf.contains(fileName)) {
            flf.remove(info.fileName());
            save = true;
        }
    }

    if (save)
        flf.save();
}

void PropertyDialog::onCancelShare()
{
    if (m_xani.data()) {
        m_xani->setDuration(0);
    }

    if (m_gani.data()) {
        m_gani->setDuration(0);
    }
}

void PropertyDialog::mousePressEvent(QMouseEvent *event)
{
    if (m_edit->isVisible()) {
        if (event->button() != Qt::RightButton) {
            m_edit->setIsCanceled(m_edit->toPlainText().isEmpty());
            emit m_edit->editFinished();
        }
    }
    DDialog::mousePressEvent(event);
}

void PropertyDialog::startComputerFolderSize(const DUrl &url)
{
    DUrl validUrl = url;
    if (url.isUserShareFile()) {
        validUrl.setScheme(FILE_SCHEME);
    }
    DUrlList urls;
    urls << validUrl;

    if (!m_sizeWorker)
        m_sizeWorker = new DFileStatisticsJob(this);

    connect(m_sizeWorker, &DFileStatisticsJob::dataNotify, this, &PropertyDialog::updateFolderSize);
    m_sizeWorker->start(urls);
}

void PropertyDialog::toggleFileExecutable(bool isChecked)
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(this, m_url);
    if (isChecked) {
        DFileService::instance()->setPermissions(this, getRealUrl(), info->permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
    } else {
        DFileService::instance()->setPermissions(this, getRealUrl(), info->permissions() & ~(QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther));
    }
}

DUrl PropertyDialog::getUrl()
{
    return m_url;
}

int PropertyDialog::getFileCount()
{
    return m_fileCount;
}

qint64 PropertyDialog::getFileSize()
{
    return m_size;
}

void PropertyDialog::raise()
{
    DDialog::raise();
    emit raised();
}

void PropertyDialog::hideEvent(QHideEvent *event)
{
    if (m_xani) {
        m_xani->stop();
        delete m_xani;
    }
    if (m_gani) {
        m_gani->stop();
        delete m_gani;
    }
    if (m_aniLabel)
        delete  m_aniLabel;
    emit aboutToClosed(m_url);
    DDialog::hideEvent(event);
    emit closed(m_url);
    if (m_sizeWorker) {
        m_sizeWorker->stop();
    }
}

void PropertyDialog::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);
}

const QList<DDrawer *> &PropertyDialog::expandGroup() const
{
    return m_expandGroup;
}

int PropertyDialog::contentHeight() const
{
    int expandsHeight = ArrowLineExpand_SPACING;
    for (const DDrawer *expand : m_expandGroup) {
        if (m_shareinfoFrame && m_shareinfoFrame->isHidden()) {
            m_shareinfoFrame->show();
        }
        expandsHeight += expand->height();
    }
#define DIALOG_TITLEBAR_HEIGHT 50
    return (DIALOG_TITLEBAR_HEIGHT +
            m_icon->height() +
            m_editStackWidget->height() +
            expandsHeight +
            contentsMargins().top() +
            contentsMargins().bottom() +
            (m_wdf ? m_wdf->height() : 0) +
            (m_tagInfoFrame ? m_tagInfoFrame->height() : 0) +
            40);
}

int PropertyDialog::getDialogHeight() const
{
    int totalHeight = this->size().height() + contentHeight() ;
    totalHeight += std::accumulate(m_expandGroup.begin(), m_expandGroup.end(), 0, [](int total, const DDrawer * expand) {
        if (expand->expand())
            return total += expand->window()->height();
        return total;
    });

    return totalHeight;
}

void PropertyDialog::loadPluginExpandWidgets()
{
    //QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_scrollArea->widget()->layout());
    QList<PropertyDialogExpandInfoInterface *> plugins = PluginManager::instance()->getExpandInfoInterfaces();
    foreach (PropertyDialogExpandInfoInterface *plugin, plugins) {
        DArrowLineDrawer *expand = new DArrowLineDrawer;//DArrowLineExpand;
        QWidget *frame = plugin->expandWidget(m_url.toString());
        if (!frame) {
            expand->deleteLater();
            continue;
        }
        frame->setMaximumHeight(EXTEND_FRAME_MAXHEIGHT);
        frame->setParent(this);
        expand->setTitle(plugin->expandWidgetTitle(m_url.toString()));
        expand->setExpand(false);
        expand->setContent(frame);

        initExpand(layout, expand);
        m_expandGroup.push_back(expand);
    }
    layout->addStretch();
}

void PropertyDialog::initExpand(QVBoxLayout *layout, DDrawer *expand)
{
    expand->setFixedHeight(ArrowLineExpand_HIGHT);
    QMargins cm = layout->contentsMargins();
    QRect rc = contentsRect();
    expand->setFixedWidth(rc.width() - cm.left() - cm.right());
    expand->setExpandedSeparatorVisible(false);
    expand->setSeparatorVisible(false);
    layout->addWidget(expand, 0, Qt::AlignTop);

    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(expand, expand);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, hanceedWidget, [ = ]() {
        QRect rect = geometry();
        rect.setHeight(contentHeight() + ArrowLineExpand_SPACING * 2);
        setGeometry(rect);
    });
}

QList<DDrawer *> PropertyDialog::addExpandWidget(const QStringList &titleList)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_scrollArea->widget()->layout());
    QList<DDrawer *> group;

    for (const QString &title : titleList) {
        DArrowLineDrawer *expand = new DArrowLineDrawer;//DArrowLineExpand;
        expand->setTitle(title);
        initExpand(layout, expand);
        group.push_back(expand);
    }

    return group;
}

void PropertyDialog::initTextShowFrame(const QString &text)
{
    m_textShowFrame = new QFrame(this);

    m_editButton = new DIconButton(m_textShowFrame);
    m_editButton->setObjectName("EditButton");
    m_editButton->setIcon(QIcon::fromTheme("edit-rename"));
    m_editButton->setIconSize({24, 24});
    m_editButton->setFixedSize(24, 24);
    m_editButton->setFlat(true);
    connect(m_editButton, &QPushButton::clicked, this, &PropertyDialog::renameFile);

    QString t = DFMGlobal::elideText(text, m_edit->size(), QTextOption::WrapAtWordBoundaryOrAnywhere, m_edit->font(), Qt::ElideMiddle, 0);
    QStringList labelTexts = t.split("\n");
    const int maxLineCount = 3;

    int textHeight = 0;
    QVBoxLayout *textShowLayout = new QVBoxLayout;

    for (int i = 0; i < labelTexts.length(); i++) {
        if (i > (maxLineCount - 1)) {
            break;
        }
        QString labelText = labelTexts.at(i);
        QLabel *label = new QLabel(labelText, m_textShowFrame);
        label->setAlignment(Qt::AlignHCenter);
        QHBoxLayout *hLayout = new QHBoxLayout;

        textHeight += label->fontInfo().pixelSize() + 10;

        hLayout->addStretch(1);
        hLayout->addWidget(label);
        if (i < (labelTexts.length() - 1) && i != (maxLineCount - 1)) {
            if (label->fontMetrics().width(labelText) > (m_edit->width() - 10)) {
                label->setFixedWidth(m_edit->width());
            }
        } else {
            // the final line of file name label, with a edit btn.
            if (labelTexts.length() >= maxLineCount) {
                for (int idx = i + 1; idx < labelTexts.length(); idx++) {
                    labelText += labelTexts.at(idx);
                }
            }

            if (label->fontMetrics().width(labelText) > (m_edit->width() - 2 * m_editButton->width()) && labelTexts.length() >= maxLineCount) {
                labelText = label->fontMetrics().elidedText(labelText, Qt::ElideMiddle, m_edit->width() - m_editButton->width());
            }
            label->setText(labelText);
            hLayout->addSpacing(2);
            hLayout->addWidget(m_editButton);
        }
        textShowLayout->addLayout(hLayout);
        hLayout->addStretch(1);
    }

    textShowLayout->setContentsMargins(0, 0, 0, 0);
    textShowLayout->setSpacing(0);
    m_textShowFrame->setLayout(textShowLayout);
    textShowLayout->addStretch(1);

    m_textShowFrame->setFixedHeight(textHeight + 15);

    if (m_editStackWidget->count() == 1) {
        m_editStackWidget->addWidget(m_textShowFrame);
    } else {
        m_editStackWidget->insertWidget(1, m_textShowFrame);
    }
    m_editStackWidget->setCurrentIndex(1);
    m_editStackWidget->setFixedHeight(m_textShowFrame->height());
}

QFrame *PropertyDialog::createBasicInfoWidget(const DAbstractFileInfoPointer &info)
{
    QFrame *widget = new QFrame(this);
    SectionKeyLabel *sizeSectionLabel = new SectionKeyLabel(QObject::tr("Size"), widget);
    SectionKeyLabel *typeSectionLabel = new SectionKeyLabel(QObject::tr("Type"), widget);
    SectionKeyLabel *TimeCreatedSectionLabel = new SectionKeyLabel(QObject::tr("Time created"), widget);
    SectionKeyLabel *TimeReadSectionLabel = new SectionKeyLabel(QObject::tr("Time accessed"), widget);
    SectionKeyLabel *TimeModifiedSectionLabel = new SectionKeyLabel(QObject::tr("Time modified"), widget);
    SectionKeyLabel *sourcePathSectionLabel = new SectionKeyLabel(QObject::tr("Source path"), nullptr);

    m_containSizeLabel = new SectionValueLabel(info->sizeDisplayName());
    m_folderSizeLabel = new SectionValueLabel("", widget);
    SectionValueLabel *typeLabel = new SectionValueLabel(info->mimeTypeDisplayName());
    SectionValueLabel *timeCreatedLabel = nullptr;
    SectionValueLabel *timeReadLabel = nullptr;
    SectionValueLabel *timeModifiedLabel = nullptr;
    if(VaultController::isRootDirectory(info->fileUrl().toLocalFile())){
        TimeModifiedSectionLabel->setText(QObject::tr("Time locked"));
        //! 保险箱根目录创建、访问、修改时间的读取
        DFM_NAMESPACE::DFMSettings setting(VAULT_TIME_CONFIG_FILE);
        timeCreatedLabel = new SectionValueLabel(setting.value(QString("VaultTime"), QString("CreateTime")).toString());
        timeReadLabel = new SectionValueLabel(setting.value(QString("VaultTime"), QString("InterviewTime")).toString());
        if(setting.value(QString("VaultTime"), QString("LockTime")).toString().isEmpty())
            timeModifiedLabel = new SectionValueLabel(setting.value(QString("VaultTime"), QString("InterviewTime")).toString());
        else
            timeModifiedLabel = new SectionValueLabel(setting.value(QString("VaultTime"), QString("LockTime")).toString());
    }
    else{
        TimeModifiedSectionLabel->setText(QObject::tr("Time modified"));
        timeCreatedLabel = new SectionValueLabel(info->createdDisplayName());
        timeReadLabel = new SectionValueLabel(info->lastReadDisplayName());
        timeModifiedLabel = new SectionValueLabel(info->lastModifiedDisplayName());
    }

    DFMGlobal::setToolTip(TimeCreatedSectionLabel);
    DFMGlobal::setToolTip(TimeReadSectionLabel);
    DFMGlobal::setToolTip(TimeModifiedSectionLabel);
    DFMGlobal::setToolTip(sourcePathSectionLabel);

    QFormLayout *layout = new QFormLayout;
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(16);
    layout->setLabelAlignment(Qt::AlignRight);

    if (info->isDir()) {
        SectionKeyLabel *fileAmountSectionLabel = new SectionKeyLabel(QObject::tr("Contains"));
        layout->addRow(sizeSectionLabel, m_folderSizeLabel);
        layout->addRow(fileAmountSectionLabel, m_containSizeLabel);
    } else {
        layout->addRow(sizeSectionLabel, m_containSizeLabel);
    }

    if (!info->isVirtualEntry()) {
        layout->addRow(typeSectionLabel, typeLabel);
    }

    SectionKeyLabel *linkPathSectionLabel = new SectionKeyLabel(QObject::tr("Location"));
    QLabel *locationPathLabel = nullptr;
    if (info->isSymLink()) {
        LinkSectionValueLabel *linkPathLabel = new LinkSectionValueLabel(info->symlinkTargetPath());
        linkPathLabel->setToolTip(info->symlinkTargetPath());
        linkPathLabel->setLinkTargetUrl(info->redirectedFileUrl());
        linkPathLabel->setOpenExternalLinks(true);
        linkPathLabel->setWordWrap(false);
        QString t = linkPathLabel->fontMetrics().elidedText(info->symlinkTargetPath(), Qt::ElideMiddle, 150);
        linkPathLabel->setText(t);
        locationPathLabel = linkPathLabel;
    } else {
        locationPathLabel = new SectionValueLabel();
        QString absoluteFilePath = info->absoluteFilePath();
        //! 在属性窗口中不显示保险箱中的文件真实路径
        if (info->fileUrl().isVaultFile()) {
            absoluteFilePath = VaultController::pathToVirtualPath(absoluteFilePath);
        }
        locationPathLabel->setText(absoluteFilePath);
        locationPathLabel->setToolTip(absoluteFilePath);
        QString t = locationPathLabel->fontMetrics().elidedText(absoluteFilePath, Qt::ElideMiddle, 150);
        locationPathLabel->setWordWrap(false);
        locationPathLabel->setText(t);
    }

    layout->addRow(linkPathSectionLabel, locationPathLabel);
    if (!info->isVirtualEntry()) {
        layout->addRow(TimeCreatedSectionLabel, timeCreatedLabel);
        layout->addRow(TimeReadSectionLabel, timeReadLabel);
        layout->addRow(TimeModifiedSectionLabel, timeModifiedLabel);
    }

    //在回收站搜索文件，需要使搜索结果的文件属性面板与回收站文件属性面板保持一致
    QString trashSourcePath = "";
    if (info->fileUrl().isSearchFile() && info->redirectedFileUrl().isTrashFile()) {
        const DAbstractFileInfoPointer &trashfileInfo = DFileService::instance()->createFileInfo(this, info->redirectedFileUrl());
        trashSourcePath = static_cast<const TrashFileInfo *>(trashfileInfo.constData())->sourceFilePath();
    }

    if (info->fileUrl().isTrashFile()) {
        trashSourcePath = static_cast<const TrashFileInfo *>(info.constData())->sourceFilePath();
    }

    //添加回收站文件原始路径
    if (!trashSourcePath.isEmpty()) {
        SectionValueLabel *sourcePathLabel = new SectionValueLabel(trashSourcePath);
        QString elidedStr = sourcePathLabel->fontMetrics().elidedText(trashSourcePath, Qt::ElideMiddle, 150);
        sourcePathLabel->setToolTip(trashSourcePath);
        sourcePathLabel->setWordWrap(false);
        sourcePathLabel->setText(elidedStr);
        sourcePathSectionLabel->setParent(widget);
        layout->addRow(sourcePathSectionLabel, sourcePathLabel);
    }

    if (info->fileUrl().isRecentFile()) {
        QString pathStr = info->filePath();
        SectionValueLabel *sourcePathLabel = new SectionValueLabel(pathStr);
        QString elidedStr = sourcePathLabel->fontMetrics().elidedText(pathStr, Qt::ElideMiddle, 150);
        sourcePathLabel->setToolTip(pathStr);
        sourcePathLabel->setWordWrap(false);
        sourcePathLabel->setText(elidedStr);
        sourcePathSectionLabel->setParent(widget);
        layout->addRow(sourcePathSectionLabel, sourcePathLabel);
    }

    if (!sourcePathSectionLabel->parent()) {
        delete sourcePathSectionLabel;
    }

    DGioSettings gsettings("com.deepin.dde.filemanager.general", "/com/deepin/dde/filemanager/general/");

    if (gsettings.value("property-dlg-hidefile-checkbox").toBool() && DFMFileListFile::supportHideByFile(info->filePath())
            && !VaultController::ins()->isRootDirectory(info->filePath())) {
        DFMFileListFile flf(QFileInfo(info->filePath()).absolutePath());
        QString fileName = info->fileName();
        QCheckBox *hideThisFile = new QCheckBox(info->isDir() ? tr("Hide this folder") : tr("Hide this file"));
        hideThisFile->setObjectName(QString("hideThisFileCheckBox"));
        //        hideThisFile->setToolTip("TODO: hint message?");

        //在回收站搜索文件，需要使搜索结果的文件属性面板与回收站文件属性面板保持一致
        bool canHide = false;
        if (DFMFileListFile::canHideByFile(info->filePath()) && !info->fileUrl().isTrashFile()) {   // fix bug#33763 回收站中不允许对文件属性进行编辑
            canHide = true;
            if (info->fileUrl().isSearchFile()) {
                DUrl url = info->redirectedFileUrl();
                if (url.isTrashFile())
                    canHide = false;
            }
        }

        hideThisFile->setEnabled(canHide);
        hideThisFile->setChecked(flf.contains(fileName));
        layout->addWidget(hideThisFile); // FIXME: do the UI thing later.
        connect(hideThisFile, &QCheckBox::clicked, this, &PropertyDialog::onHideFileCheckboxChecked);
    }

    layout->setContentsMargins(15, 15, 30, 15);
    widget->setLayout(layout);

    return widget;
}

ShareInfoFrame *PropertyDialog::createShareInfoFrame(const DAbstractFileInfoPointer &info)
{
    DAbstractFileInfoPointer infoPtr = info->canRedirectionFileUrl() ? DFileService::instance()->createFileInfo(nullptr, info->redirectedFileUrl())
                                       : info;
    ShareInfoFrame *frame = new ShareInfoFrame(infoPtr, this);
    //play animation after a folder is shared
    // 侧边栏创建完成共享标签后再执行动画
    DFileManagerWindow *window = qobject_cast<DFileManagerWindow *>(WindowManager::getWindowById(m_fmevent.windowId()));
    if (window) {
        DFMSideBar *sideBar = window->getLeftSideBar();
        connect(sideBar, &DFMSideBar::addUserShareItemFinished, this, &PropertyDialog::flickFolderToSidebar);
    }

    if (DFMGlobal::isWayLand()) {
        // 取消共享时停止动画效果
        connect(frame, &ShareInfoFrame::unfolderShared, this, &PropertyDialog::onCancelShare);
    }

    return frame;
}

QList<QPair<QString, QString> > PropertyDialog::createLocalDeviceInfoWidget(const DAbstractFileInfoPointer &info)
{
    QList<QPair<QString, QString> > results;

    if (!info) {
        qWarning("BUG: cannot create local device info because given device info pointer is not valid");
        return results;
    }

    QString fsType = info->extraProperties()["fsType"].toString();
    quint64 fsUsed = info->extraProperties()["fsUsed"].toULongLong();
    quint64 fsFreeSize = info->extraProperties()["fsFreeSize"].toULongLong();
    quint64 fsSize = info->extraProperties()["fsSize"].toULongLong();
    //fixed:CD display size error
    if (static_cast<DFMRootFileInfo::ItemType>(info->fileType()) == DFMRootFileInfo::ItemType::UDisksOptical) {
        DFMRootFileInfo *pFileInfo = dynamic_cast<DFMRootFileInfo *>(info.data());
        QString strVolTag;
        if (pFileInfo)
            strVolTag = pFileInfo->getVolTag();
        fsSize = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nTotal;
        fsFreeSize = fsSize - DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nUsage;
    }
    quint64 fileCount = 0;
    DUrl redirectedFileUrl = info->redirectedFileUrl();
    if (!redirectedFileUrl.isEmpty()) {
        if (redirectedFileUrl.burnIsOnDisc()) {
            if (!redirectedFileUrl.burnDestDevice().isEmpty()) {
                DUrl stagingUrl = DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                                      + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"
                                                      + redirectedFileUrl.burnDestDevice().replace('/', '_'));
                QString stagingFilePath = stagingUrl.toLocalFile();
                if (!stagingFilePath.isEmpty()) {
                    fileCount = FileUtils::filesCount(stagingFilePath);
                }
            }

            DAbstractFileInfoPointer fi = fileService->createFileInfo(this, redirectedFileUrl);
            DUrl url = DUrl::fromLocalFile(fi->extraProperties()["mm_backer"].toString());
            redirectedFileUrl = url;
        }

        QString localFilePath = redirectedFileUrl.toLocalFile();
        if (!localFilePath.isEmpty()) {
            fileCount += FileUtils::filesCount(localFilePath);
        }
    }

    static QHash<DFMRootFileInfo::ItemType, QString> devtypemap = {
        {DFMRootFileInfo::ItemType::UDisksRoot, QObject::tr("Local disk")},
        {DFMRootFileInfo::ItemType::UDisksData, QObject::tr("Local disk")},
        {DFMRootFileInfo::ItemType::UDisksFixed, QObject::tr("Local disk")},
        {DFMRootFileInfo::ItemType::UDisksRemovable, QObject::tr("Removable disk")},
        {DFMRootFileInfo::ItemType::UDisksOptical, QObject::tr("DVD")},
        {DFMRootFileInfo::ItemType::GvfsFTP, QObject::tr("Network shared directory")},
        {DFMRootFileInfo::ItemType::GvfsSMB, QObject::tr("Network shared directory")},
        {DFMRootFileInfo::ItemType::GvfsMTP, QObject::tr("Android mobile device")},
        {DFMRootFileInfo::ItemType::GvfsGPhoto2, QObject::tr("Apple mobile device")},
        {DFMRootFileInfo::ItemType::GvfsCamera, QObject::tr("Camera")},
        {DFMRootFileInfo::ItemType::NotConnRemote, QObject::tr("Unconnected network shared directory")}
    };

    //fix GvfsGPhoto2协议对Apple mobile device判断有问题，再增加一层判断
    {
        auto itemtype = static_cast<DFMRootFileInfo::ItemType>(info->fileType());
        if (itemtype == DFMRootFileInfo::ItemType::GvfsGPhoto2) {
            QString devicePath = info->redirectedFileUrl().path();

            qInfo() << "Deivce Type: DFMRootFileInfo::ItemType::GvfsGPhoto2 Device Path:" << devicePath;

            //判断host中是否有"Apple_Inc"，没有且不为空则改为安卓
            if (!devicePath.isEmpty() && !devicePath.contains("Apple_Inc")) {
                qWarning() << "Deivce Type is DFMRootFileInfo::ItemType::GvfsGPhoto2. Not find Apple_Inc in device path"
                           << devicePath << "Set Deivce Type [GvfsGPhoto2] to [GvfsMTP]";
                itemtype = DFMRootFileInfo::ItemType::GvfsMTP;
            }
        }
        results.append({QObject::tr("Device type"), devtypemap.value(itemtype, QObject::tr("Unknown device"))});
    }
    //old
    //results.append({QObject::tr("Device type"), devtypemap.value(static_cast<DFMRootFileInfo::ItemType>(info->fileType()), QObject::tr("Unknown device"))});
    //end fix

    //fixed:CD display size error
    if (static_cast<DFMRootFileInfo::ItemType>(info->fileType()) == DFMRootFileInfo::ItemType::UDisksOptical) {
        DFMRootFileInfo *pFileInfo = dynamic_cast<DFMRootFileInfo *>(info.data());
        QString strVolTag;
        if (pFileInfo)
            strVolTag = pFileInfo->getVolTag();
        fsSize = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nTotal;
        fsUsed = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nUsage;
    }

    results.append({QObject::tr("Total space"), FileUtils::formatSize(fsSize)});
    if (!fsType.isEmpty()) {
        results.append({QObject::tr("File system"), fsType});
    }
    results.append({QObject::tr("Contains"), (fileCount != 1 ? QObject::tr("%1 items") : QObject::tr("%1 item")).arg(fileCount)});

    quint64 fsFreeSizeSet = fsFreeSize > 0 ? fsFreeSize : (fsSize - fsUsed);
    results.append({QObject::tr("Free space"), FileUtils::formatSize(fsFreeSizeSet)});

    return results;
}

QFrame *PropertyDialog::createInfoFrame(const QList<QPair<QString, QString> > &properties)
{
    QFrame *widget = new QFrame(this);

    QFormLayout *layout = new QFormLayout;
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(16);
    layout->setLabelAlignment(Qt::AlignRight);

    for (const QPair<QString, QString> &kv : properties) {
        SectionKeyLabel *keyLabel = new SectionKeyLabel(kv.first, widget);
        SectionValueLabel *valLabel = new SectionValueLabel(kv.second, widget);
        layout->addRow(keyLabel, valLabel);
    }

    widget->setLayout(layout);
    //widget是最小高度时，藏语会发生截断，layout自适应，设置最小展示高度
    widget->setMinimumHeight(EXTEND_FRAME_MAXHEIGHT);
    return widget;
}

QListWidget *PropertyDialog::createOpenWithListWidget(const DAbstractFileInfoPointer &info)
{
    DUrl fileUrl = info->fileUrl();
    DAbstractFileInfoPointer infoPtr = info;
    while (infoPtr->canRedirectionFileUrl()) {
        if (fileUrl == infoPtr->redirectedFileUrl()) {
            break;
        }
        fileUrl = infoPtr->redirectedFileUrl();
        infoPtr = fileService->createFileInfo(nullptr, fileUrl);
    }
    QListWidget *listWidget = new QListWidget(this);
    listWidget->setSpacing(8);
    listWidget->setObjectName("OpenWithListWidget");
    // 修复BUG-47113 UI显示问题
    listWidget->setFrameShape(QFrame::HLine);
    m_OpenWithButtonGroup = new QButtonGroup(listWidget);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList recommendApps = mimeAppsManager->getRecommendedApps(fileUrl);

    QString mimeType = infoPtr->mimeType().name();
    QString defaultApp = mimeAppsManager->getDefaultAppDisplayNameByGio(mimeType);

    foreach (const QString &appFile, recommendApps) {
        if (!QFile::exists(appFile)) {
            continue;
        }
        DesktopFile df(appFile);

        QListWidgetItem *item = new QListWidgetItem;

        QCheckBox *itemBox = new QCheckBox(df.getDisplayName());
        itemBox->setObjectName("OpenWithItem");
        itemBox->setIcon(QIcon::fromTheme(df.getIcon()));
        itemBox->setIconSize(QSize(16, 16));
        itemBox->setProperty("appPath", appFile);
        itemBox->setProperty("mimeTypeName", mimeType);
        m_OpenWithButtonGroup->addButton(itemBox);
        item->setData(Qt::UserRole, df.getName());
        listWidget->addItem(item);
        listWidget->setItemWidget(item, itemBox);

        if (df.getLocalName() == defaultApp) {
            itemBox->setChecked(true);
        }

    }

    int listHeight = 0;
    int count = listWidget->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = listWidget->item(i);
        item->setFlags(Qt::NoItemFlags);
        int h = listWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        // 乘以2是因为item与item之间有两个spacing
        listHeight += h + listWidget->spacing() * 2;
    }
    // 加上最后一个spacing
    listHeight += listWidget->spacing();

    // 修复UI-BUG-48789 自动设置listwidget的高度，使得根据内容延展其面板的长度
    if (count < 1) {
        // 当没有打开方式时，设置一个固定大小
        listWidget->setFixedHeight(ArrowLineExpand_HIGHT);
    } else {
        listWidget->setFixedHeight(listHeight);
    }
    listWidget->setFixedWidth(300);
    // 隐藏垂直滚动条
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_OpenWithButtonGroup, SIGNAL(buttonClicked(QAbstractButton *)),
            this, SLOT(onOpenWithBntsChecked(QAbstractButton *)));

    return listWidget;
}

QFrame *PropertyDialog::createAuthorityManagementWidget(const DAbstractFileInfoPointer &info)
{
    QFrame *widget = new QFrame;
    QFormLayout *layout = new QFormLayout;

    QComboBox *ownerBox = new QComboBox;
    QComboBox *groupBox = new QComboBox;
    QComboBox *otherBox = new QComboBox;

    ownerBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    groupBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    otherBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);

    DUrl parentUrl = info->parentUrl();
    DStorageInfo storageInfo(parentUrl.toLocalFile());
    const QString &fsType = storageInfo.fileSystemType();
    // these are for file or folder, folder will with executable index.
    int readWriteIndex = 0, readOnlyIndex = 0;

    int readOnlyFlag = 4;
    int readOnlyWithXFlag = 5;

    int readWriteFlag = 6;
    int readWriteWithXFlag = 7;

    QStringList authorityList;

    authorityList << QObject::tr("Access denied") // 0
                  << QObject::tr("Executable") // 1
                  << QObject::tr("Write only") // 2
                  << QObject::tr("Write only") // 3 with x
                  << QObject::tr("Read only")  // 4
                  << QObject::tr("Read only")  // 5 with x
                  << QObject::tr("Read-write") // 6
                  << QObject::tr("Read-write"); // 7 with x

    static QStringList canChmodFileType = {"vfat", "fuseblk"};

    if (info->isFile()) {
        // append `Executable` string
        QString append = QStringLiteral(" , ") + QObject::tr("Executable");
        authorityList[3] += append;
        authorityList[5] += append;
        authorityList[7] += append;

        // file: read is read, read-write is read-write
        readOnlyIndex = readOnlyFlag;
        readWriteIndex = readWriteFlag;
    }

    if (info->isDir()) {
        // folder: read is read and executable, read-write is read-write and executable
        readOnlyIndex = readOnlyWithXFlag;
        readWriteIndex = readWriteWithXFlag;
    }

    // enumFlag should be 0~7, this is just a check to avoid runtime error
    auto getPermissionString = [ &authorityList ](int enumFlag) {
        enumFlag &= 0x0007;
        if (enumFlag < 0 || enumFlag > 7) {
            return QStringLiteral("..what?");
        } else {
            return authorityList[enumFlag];
        }
    };

    // set QComboBox, notice this permission number is not just 0~7
    auto setComboBoxByPermission = [ = ](QComboBox * cb, int permission, int offset) {
        int index = permission >> offset;
        if (index == readWriteFlag || index == readWriteWithXFlag) {
            cb->setCurrentIndex(0);
        } else if (index == readOnlyFlag || index == readOnlyWithXFlag) {
            cb->setCurrentIndex(1);
        } else {
            cb->addItem(getPermissionString(index), QVariant(permission));
            cb->setCurrentIndex(2);
        }
    };

    // when change the index...
    auto onComboBoxChanged = [ = ]() {

        // 保险箱文件权限特殊处理,不要修改文件权限，保证文件出保险箱和进保险箱权限一直
        if(VaultController::isVaultFile(info->fileUrl().toLocalFile()))
            return;

        struct stat fileStat;
        QByteArray infoBytes(info->toLocalFile().toUtf8());
        stat(infoBytes.data(), &fileStat);
        auto preMode = fileStat.st_mode;

        int ownerFlags = ownerBox->currentData().toInt();
        int groupFlags = groupBox->currentData().toInt();
        int otherFlags = otherBox->currentData().toInt();

        QFile::Permissions permissions = info->permissions();
        //点击combobox都需要保持执行权限，否则将失去相关权限位
        ownerFlags |= (permissions & QFile::ExeOwner);
        groupFlags |= (permissions & QFile::ExeGroup);
        otherFlags |= (permissions & QFile::ExeOther);

        DFileService::instance()->setPermissions(this, getRealUrl(),
                                                 QFileDevice::Permissions(ownerFlags) |
                                                 /*(info->permissions() & 0x0700) |*/
                                                 QFileDevice::Permissions(groupFlags) |
                                                 QFileDevice::Permissions(otherFlags));
        infoBytes = info->toLocalFile().toUtf8();
        stat(infoBytes.data(), &fileStat);
        auto afterMode = fileStat.st_mode;
        // 修改权限失败
        // todo 回滚权限
        if (preMode == afterMode) {
            qDebug() << "chmod failed";
        }
    };

    ownerBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteOwner | QFile::ReadOwner));
    ownerBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadOwner));
    groupBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteGroup | QFile::ReadGroup));
    groupBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadGroup));
    otherBox->addItem(authorityList[readWriteIndex], QVariant(QFile::WriteOther | QFile::ReadOther));
    otherBox->addItem(authorityList[readOnlyIndex], QVariant(QFile::ReadOther));

    setComboBoxByPermission(ownerBox, info->permissions() & 0x7000, 12);
    setComboBoxByPermission(groupBox, info->permissions() & 0x0070, 4);
    setComboBoxByPermission(otherBox, info->permissions() & 0x0007, 0);

    layout->setLabelAlignment(Qt::AlignLeft);
    layout->addRow(QObject::tr("Owner"), ownerBox);
    layout->addRow(QObject::tr("Group"), groupBox);
    layout->addRow(QObject::tr("Others"), otherBox);

    if (info->isFile()) {
        m_executableCheckBox = new QCheckBox;
        m_executableCheckBox->setText(tr("Allow to execute as program"));
        connect(m_executableCheckBox, &QCheckBox::toggled, this, &PropertyDialog::toggleFileExecutable);
        if (info->ownerId() != getuid()) {
            m_executableCheckBox->setDisabled(true);
        }

        QString filePath = info->path();
        if (VaultController::ins()->isVaultFile(info->path())) { // Vault file need to use stat function to read file permission.
            QString localFile = info->toLocalFile();
            struct stat buf;
            std::string stdStr = localFile.toStdString();
            stat(stdStr.c_str(), &buf);
            if ((buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXGRP) || (buf.st_mode & S_IXOTH)) {
                m_executableCheckBox->setChecked(true);
            }
        } else {
            if (info->permission(QFile::ExeUser) || info->permission(QFile::ExeGroup) || info->permission(QFile::ExeOther)) {
                m_executableCheckBox->setChecked(true);
            }
        }
        // 一些文件系统不支持修改可执行权限
        if (!canChmod(info) || canChmodFileType.contains(fsType)) {
            m_executableCheckBox->setDisabled(true);
        }
        layout->addRow(m_executableCheckBox);
    }


    layout->setContentsMargins(15, 10, 30, 10);
    widget->setLayout(layout);

    connect(ownerBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), widget, onComboBoxChanged);
    connect(groupBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), widget, onComboBoxChanged);
    connect(otherBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), widget, onComboBoxChanged);

    // 置灰：
    // 1. 本身用户无权限
    // 2. 所属文件系统无权限机制
    if (info->ownerId() != getuid() ||
            !canChmod(info) ||
            fsType == "fuseblk") {
        ownerBox->setDisabled(true);
        groupBox->setDisabled(true);
        otherBox->setDisabled(true);
    }

    // tmp: 暂时的处理
    if (fsType == "vfat") {
        groupBox->setDisabled(true);
        otherBox->setDisabled(true);
        if (info->isDir()) {
            ownerBox->setDisabled(true);
        }
    }

    // 保险箱文件权限特殊处理
    if(VaultController::isVaultFile(info->fileUrl().toLocalFile())) {
        ownerBox->setCurrentIndex(0);
        ownerBox->setDisabled(true);
        groupBox->setDisabled(true);
        otherBox->setDisabled(true);
    }

    // smb、ftp、sftp不支持文件权限的修改
    if (info->isGvfsMountFile()) {
        ownerBox->setDisabled(true);
        groupBox->setDisabled(true);
        otherBox->setDisabled(true);        
    }

    return widget;
}
