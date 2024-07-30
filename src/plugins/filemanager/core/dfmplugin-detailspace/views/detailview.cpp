// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailview.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>

#include <dfm-framework/dpf.h>

#include <QGridLayout>
#include <QScrollArea>
#include <QFileSystemModel>
#include <QTreeView>

#include <DPushButton>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

Q_DECLARE_METATYPE(QString *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_detailspace;

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE) };

DetailView::DetailView(QWidget *parent)
    : DFrame(parent)
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &DetailView::initUiForSizeMode);
#endif
    initInfoUI();
    initUiForSizeMode();
}

DetailView::~DetailView()
{
}

/*!
 * \brief               在最右文件信息窗口中追加新增控件
 * \param widget        新增控件对象
 * \return              是否成功
 */
bool DetailView::addCustomControl(QWidget *widget)
{
    if (widget) {
        QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
        insertCustomControl(vlayout->count() - 1, widget);
        return true;
    }
    return false;
}

/*!
 * \brief               在最右文件信息窗口中指定位置新增控件
 * \param widget        新增控件对象
 * \return              是否成功
 */
bool DetailView::insertCustomControl(int index, QWidget *widget)
{
    // final one is stretch
    index = index == -1 ? vLayout->count() - 1 : qMin(vLayout->count() - 1, index);

    if (widget) {
        widget->setParent(this);
        QFrame *frame = new QFrame(this);
        DPushButton *btn = new DPushButton(frame);
        btn->setEnabled(false);
        btn->setFixedHeight(1);
        QVBoxLayout *vlayout = new QVBoxLayout(frame);
        vlayout->setContentsMargins(0, 0, 0, 0);
        vlayout->setSpacing(10);
        vlayout->addWidget(btn);
        vlayout->addWidget(widget);
        frame->setLayout(vlayout);
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->insertWidget(index, frame, 0, Qt::AlignTop);

        expandList.append(frame);
        return true;
    }
    return false;
}

void DetailView::removeControl()
{
    for (QWidget *w : expandList) {
        expandList.removeOne(w);
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->removeWidget(w);
        delete w;
    }
}

void DetailView::setUrl(const QUrl &url, int widgetFilter)
{
    createHeadUI(url, widgetFilter);
    createBasicWidget(url, widgetFilter);
}

void DetailView::initUiForSizeMode()
{
    if (!scrollArea)
        return;
#ifdef DTKWIDGET_CLASS_DSizeMode
    scrollArea->setFixedWidth(DSizeModeHelper::element(254, 282));
    scrollArea->setContentsMargins(DSizeModeHelper::element(2, 0), 0, DSizeModeHelper::element(0, 6), 0);
#else
    scrollArea->setFixedWidth(282);
    scrollArea->setContentsMargins(0, 0, 6, 0);
#endif
}

void DetailView::initInfoUI()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setAlignment(Qt::AlignLeft);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    expandFrame = new QFrame(this);
    expandFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(expandFrame);

    vLayout = new QVBoxLayout(this);
    vLayout->addStretch();
    expandFrame->setLayout(vLayout);
    vLayout->setContentsMargins(0, 0, 8, 0);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea, Qt::AlignCenter);
    this->setFrameShape(QFrame::NoFrame);
    this->setLayout(mainLayout);
}

void DetailView::createHeadUI(const QUrl &url, int widgetFilter)
{
    if (widgetFilter == DetailFilterType::kIconView) {
        return;
    } else {
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);
        if (info.isNull())
            return;

        if (iconLabel) {
            vLayout->removeWidget(iconLabel);
            delete iconLabel;
            iconLabel = nullptr;
        }

        iconLabel = new DLabel(this);
        iconLabel->setFixedSize(200, 200);
        //iconLabel->setStyleSheet("border: 1px solid red;");
        QSize targetSize(200, 160);
        auto findPluginIcon = [](const QUrl &url) -> QString {
            QString iconName;
            bool ok = dpfHookSequence->run(kCurrentEventSpace, "hook_Icon_Fetch", url, &iconName);
            if (ok && !iconName.isEmpty())
                return iconName;

            return QString();
        };

        // get icon from plugin
        QIcon icon;
        ThumbnailHelper helper;
        const QString &iconName = findPluginIcon(info->urlOf(UrlInfoType::kUrl));
        if (!iconName.isEmpty()) {
            icon = QIcon::fromTheme(iconName);
        } else if (helper.checkThumbEnable(url)) {
            icon = info->extendAttributes(ExtInfoType::kFileThumbnail).value<QIcon>();
            if (icon.isNull()) {
                const auto &img = helper.thumbnailImage(url, Global::kLarge);
                icon = QPixmap::fromImage(img);
            }
        }
        if (icon.isNull())
            icon = info->fileIcon();

        QPixmap px = icon.pixmap(targetSize);
        px.setDevicePixelRatio(qApp->devicePixelRatio());
        iconLabel->setPixmap(px);
        iconLabel->setAlignment(Qt::AlignCenter);
        vLayout->insertWidget(0, iconLabel, 0, Qt::AlignHCenter);
    }
}

void DetailView::createBasicWidget(const QUrl &url, int widgetFilter)
{
    if (widgetFilter == DetailFilterType::kBasicView) {
        return;
    } else {
        fileBaseInfoView = new FileBaseInfoView(this);
        fileBaseInfoView->setFileUrl(url);
        addCustomControl(fileBaseInfoView);
    }
}

void DetailView::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);
}
