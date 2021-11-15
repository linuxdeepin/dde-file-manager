/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#include "dfmtagwidget.h"
#include "dfileservices.h"
#include "app/define.h"
#include "controllers/pathmanager.h"
#include "controllers/vaultcontroller.h"
#include "accessibility/ac-lib-file-manager.h"
#include "singleton.h"
#include <dfilemenumanager.h>
#include <shutil/desktopfile.h>
#include <tag/tagmanager.h>
#include <QLabel>
#include <QVBoxLayout>
#include <dabstractfilewatcher.h>
#include <QScrollBar>
#include <dstorageinfo.h>
DFM_BEGIN_NAMESPACE

class DFMCrumbEdit : public DCrumbEdit
{
public:
    explicit DFMCrumbEdit(QWidget *parent = nullptr): DCrumbEdit(parent)
    {
        auto doc = QTextEdit::document();
        doc->setDocumentMargin(doc->documentMargin() + 5);
    }

    bool isEditing()
    {
        return m_isEditByDoubleClick;
    }
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        m_isEditByDoubleClick = true;
        DCrumbEdit::mouseDoubleClickEvent(event);
        m_isEditByDoubleClick = false;
    }

private:
    bool m_isEditByDoubleClick{false};
};

class DFMTagWidgetPrivate : public QSharedData
{
public:
    explicit DFMTagWidgetPrivate(DFMTagWidget *qq, const DUrl &url);
    virtual ~DFMTagWidgetPrivate();

protected:
    DUrl redirectUrl(const DUrl &url);

private:
    DUrl    m_url;
    QLabel      *m_tagLable{nullptr};
    QLabel      *m_tagLeftLable{nullptr};
    QVBoxLayout *m_mainLayout{nullptr};
    DFMCrumbEdit  *m_tagCrumbEdit{ nullptr };
    DTagActionWidget *m_tagActionWidget{ nullptr };
    DAbstractFileWatcher *m_devicesWatcher{ nullptr };

    QMap<QString, QString> currentTagWithColorMap;

    DFMTagWidget *q_ptr{ nullptr };
    Q_DECLARE_PUBLIC(DFMTagWidget)
};


DUrl DFMTagWidgetPrivate::redirectUrl(const DUrl &url)
{
    DUrl durl = url;
    if (url.isTaggedFile()) {
        durl = DUrl::fromLocalFile(url.fragment(QUrl::FullyDecoded));
    } else if (url.isSearchFile()) {
        durl = url.searchedFileUrl();
    } else if (url.isVaultFile()) {
        durl = VaultController::vaultToLocalUrl(url);
    } else { /* if (url.isRecentFile()) */
        durl = DUrl::fromLocalFile(url.path());
    }

    return durl;
}

DFMTagWidgetPrivate::DFMTagWidgetPrivate(DFMTagWidget *qq, const DUrl &url)
    : m_url(redirectUrl(url))
    , q_ptr(qq)
{

}

DFMTagWidgetPrivate::~DFMTagWidgetPrivate()
{

}

DFMTagWidget::DFMTagWidget(DUrl url, QWidget *parent/*=nullptr*/)
    : QFrame(parent)
    , d_private(new DFMTagWidgetPrivate(this, url))
{
    AC_SET_OBJECT_NAME(this, AC_TAG_WIDGET);
    AC_SET_ACCESSIBLE_NAME(this, AC_TAG_WIDGET);
    initUi();
    initConnection();
}

DFMTagWidget::~DFMTagWidget()
{

}

void DFMTagWidget::initUi()
{
    Q_D(DFMTagWidget);
    d->m_mainLayout = new QVBoxLayout;
    setLayout(d->m_mainLayout);

    //tr("Tag"); // dde_file_manager::DFMTagWidget not found in .ts
    QString name = qApp->translate("DFMTagWidget", "Tag");
    d->m_tagLable = new QLabel(name, this);
    d->m_tagLable->setObjectName(name);
    AC_SET_ACCESSIBLE_NAME(d->m_tagLable, AC_TAG_LABLE);

    d->m_mainLayout->addWidget(d->m_tagLable);
    d->m_tagLeftLable = new QLabel(name, this);
    d->m_tagLeftLable->setObjectName(name);
    AC_SET_ACCESSIBLE_NAME(d->m_tagLeftLable, AC_TAG_LEFT_LABLE);

    d->m_tagActionWidget =  new DTagActionWidget(this);
    d->m_tagActionWidget->setMaximumHeight(20);
    d->m_tagActionWidget->setObjectName("tagActionWidget");
    AC_SET_ACCESSIBLE_NAME(d->m_tagActionWidget, AC_TAG_ACTION_WIDGET);
    QHBoxLayout *tagActionLayout = new QHBoxLayout;
    tagActionLayout->addWidget(d->m_tagLeftLable);
    tagActionLayout->addWidget(d->m_tagActionWidget);
    d->m_mainLayout->addLayout(tagActionLayout);
    d->m_tagLeftLable->setHidden(true);

    d->m_tagCrumbEdit = new DFMCrumbEdit(this);
    d->m_tagCrumbEdit->setObjectName("tagCrumbEdit");
    AC_SET_ACCESSIBLE_NAME(d->m_tagCrumbEdit, AC_TAG_CRUMB_EDIT);
    d->m_tagCrumbEdit->setFrameShape(QFrame::Shape::NoFrame);
    d->m_tagCrumbEdit->viewport()->setBackgroundRole(QPalette::NoRole);
    d->m_mainLayout->addWidget(d->m_tagCrumbEdit);

    // 修复bug-47113 UI显示问题
//    d->m_mainLayout->addStretch();
    d->m_mainLayout->setContentsMargins(10, 10, 10, 10);

    loadTags(d->m_url);
}

void DFMTagWidget::initConnection()
{
    Q_D(DFMTagWidget);
    if (!d->m_tagCrumbEdit || !d->m_tagActionWidget)
        return;

    QObject::connect(d->m_tagCrumbEdit, &DCrumbEdit::crumbListChanged, d->m_tagCrumbEdit, [ = ]() {
        if (!d->m_tagCrumbEdit->isEditing() && !d->m_tagCrumbEdit->property("LoadFileTags").toBool()) {
            bool ret = DFileService::instance()->makeTagsOfFiles(nullptr, {d->m_url}, d->m_tagCrumbEdit->crumbList());

            if (!ret) {
                loadTags(d->m_url);
                return;
            }
        }
    });

    QObject::connect(d->m_tagActionWidget, &DTagActionWidget::checkedColorChanged, d->m_tagActionWidget, [ = ](const QColor & color) {
        Q_UNUSED(color)
        const QStringList tagNameList = TagManager::instance()->getTagsThroughFiles({d->m_url});
        QMap<QString, QColor> nameColors = TagManager::instance()->getTagColor({tagNameList});
        DUrlList urlList{d->m_url};
        QList<QColor> checkedColors{ d->m_tagActionWidget->checkedColorList() };
        QSet<QString> defaultNames = TagManager::instance()->allTagOfDefaultColors();

        QStringList newTagNames;
        for (const QColor &color : checkedColors) {
            QString tagName = TagManager::instance()->getTagNameThroughColor(color);
            if (tagName.isEmpty()) {
                continue;
            }
            newTagNames << tagName;
        }

        for (auto it = nameColors.begin(); it != nameColors.end(); ++it) {
            if (!defaultNames.contains(it.key())) {
                newTagNames << it.key();
            }
        }

        DFileService::instance()->makeTagsOfFiles(nullptr, urlList, newTagNames);
        loadTags(d->m_url);
    });
}

void DFMTagWidget::loadTags(const DUrl &durl)
{
    Q_D(DFMTagWidget);
    DUrl url = d->redirectUrl(durl);
    if (!d->m_tagCrumbEdit || !d->m_tagActionWidget || !shouldShow(url))
        return;
    const QStringList tag_name_list = TagManager::instance()->getTagsThroughFiles({url});
    QMap<QString, QColor> nameColors = TagManager::instance()->getTagColor({tag_name_list});
    QSet<QString> defaultColors = TagManager::instance()->allTagOfDefaultColors();
    QList<QColor>  selectColors;

    //避免重复刷新edit，防止出现因刷新导致输入文本丢失的情况
    if (d->m_url == url && tag_name_list.length() == d->currentTagWithColorMap.count()) {
        bool needRefreshEdit = false;
        for (const QString &tag : tag_name_list) {
            QString colorName = TagManager::instance()->getColorByDisplayName(tag);
            if (!d->currentTagWithColorMap.contains(tag) ||
                    d->currentTagWithColorMap.value(tag) != colorName) {
                needRefreshEdit = true;
                break;
            }
        }

        if (!needRefreshEdit)
            return;
    }

    d->m_tagCrumbEdit->setProperty("LoadFileTags", true);
    d->m_tagCrumbEdit->clear();
    d->currentTagWithColorMap.clear();
    for (auto it = nameColors.begin(); it != nameColors.end(); ++it) {
        DCrumbTextFormat format = d->m_tagCrumbEdit->makeTextFormat();
        format.setText(it.key());
        // 默认名字的颜色才勾选 checkbox
        QString colorName = TagManager::instance()->getColorByDisplayName(it.key());
        if (!colorName.isEmpty()) {
            QColor defaultColor = TagManager::instance()->getColorByColorName(colorName);
            if (defaultColors.contains(it.key()) && it.value() == defaultColor) {
                selectColors << it.value();
            }
        }
        format.setBackground(QBrush(it.value()));
        format.setBackgroundRadius(5);
        d->m_tagCrumbEdit->insertCrumb(format, 0);
        d->currentTagWithColorMap.insert(it.key(), colorName);
    }
    d->m_tagCrumbEdit->setProperty("LoadFileTags", false);

    d->m_tagActionWidget->setCheckedColorList(selectColors);
    if (!d->m_devicesWatcher || d->m_url != url) {
        d->m_url = url;

        if (d->m_devicesWatcher) {
            d->m_devicesWatcher->stopWatcher();
            d->m_devicesWatcher->deleteLater();
        }
        d->m_devicesWatcher = DFileService::instance()->createFileWatcher(this, d->m_url, this);
        if (d->m_devicesWatcher) {
            d->m_devicesWatcher->startWatcher();

            connect(d->m_devicesWatcher, &DAbstractFileWatcher::fileAttributeChanged, this, [ = ](const DUrl & url) {
                if (url == d->m_url) {
                    loadTags(d->m_url);
                }
            });
            //当文件被删除时需要在这里把watcher移除，否则可能导致再创建同名文件无法正确添加watcher
            connect(d->m_devicesWatcher, &DAbstractFileWatcher::fileDeleted, this, [ = ] {
                if (d->m_devicesWatcher)
                {
                    d->m_devicesWatcher->stopWatcher();
                    d->m_devicesWatcher->deleteLater();
                    d->m_devicesWatcher = nullptr;
                }
            });
        }
    }
}

QWidget *DFMTagWidget::tagTitle()
{
    Q_D(DFMTagWidget);
    return d->m_tagLable;
}

QWidget *DFMTagWidget::tagLeftTitle()
{
    Q_D(DFMTagWidget);
    return d->m_tagLeftLable;
}

DTagActionWidget *DFMTagWidget::tagActionWidget()
{
    Q_D(DFMTagWidget);
    return d->m_tagActionWidget;
}

DCrumbEdit *DFMTagWidget::tagCrumbEdit()
{
    Q_D(DFMTagWidget);
    return d->m_tagCrumbEdit;
}

bool DFMTagWidget::shouldShow(const DUrl &url)
{
    if (DFileService::instance()->checkGvfsMountfileBusy(url,false))
        return false;
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(nullptr, url);
    //如果是网络挂载（gvfs）文件就返回
    if (!fileInfo || fileInfo->isVirtualEntry() || fileInfo->isGvfsMountFile() || DStorageInfo::isLowSpeedDevice(url.toAbsolutePathUrl().path()))
        return false;
    bool isComputerOrTrash = false;
    DUrl realTargetUrl = fileInfo->fileUrl();
    if (fileInfo && fileInfo->isSymLink()) {
        realTargetUrl = fileInfo->rootSymLinkTarget();
    }

    if (realTargetUrl.toLocalFile().endsWith(QString(".") + "desktop")) {
        DesktopFile df(realTargetUrl.toLocalFile());
        isComputerOrTrash = (df.getDeepinId() == "dde-trash" || df.getDeepinId() == "dde-computer");
    }

    bool showTags = !systemPathManager->isSystemPath(url.path()) &&
                    !isComputerOrTrash && DFileMenuManager::whetherShowTagActions({url});
    return showTags;
}

DFM_END_NAMESPACE
