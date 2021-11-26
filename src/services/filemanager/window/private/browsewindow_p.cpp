/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "browsewindow_p.h"
#include "window/browseview.h"
#include "dfm-base/widgets/dfmfileview/fileviewitem.h"
#include "dfm-base/widgets/dfmfileview/fileviewmodel.h"

#include <dfm-framework/framework.h>

DSB_FM_BEGIN_NAMESPACE

AddressBar *BrowseWindowPrivate::addressBar() const
{
    return addressBarIns;
}

void BrowseWindowPrivate::setAddressBar(AddressBar *addressBar)
{
    if (!titleBarLayoutIns) return;

    if (!addressBar) return;

    if (!titleBarLayoutIns->replaceWidget(addressBarIns, addressBar)) {
        if (addressBar) {
            delete addressBar;
            addressBar = nullptr;
        }
    } else {
        addressBarIns = addressBar;
    }
}

void BrowseWindowPrivate::doSearchButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    showAddrsssBar();
    searchButtonIns->hide();
}

void BrowseWindowPrivate::doViewModeButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    BrowseView *view = qobject_cast<BrowseView *>(currentDisplayView());

    if (!view || !sender() || !optionButtonBox()
        || !listViewButton() || !iconViewButton()) return;

    if (sender() == listViewButton()) {
        iconViewButton()->setChecked(false);
        view->setViewMode(BrowseView::ViewMode::ListMode);
        mode = BrowseView::ListMode;
    }

    if (sender() == iconViewButton()) {
        listViewButton()->setChecked(false);
        view->setViewMode(BrowseView::ViewMode::IconMode);
        mode = BrowseView::IconMode;
    }

    if (mode == BrowseView::IconMode) {
        if (listViewButton())
            listViewButton()->setChecked(false);
        if (iconViewButton())
            iconViewButton()->setChecked(true);
    } else if (mode == BrowseView::ListMode) {
        if (listViewButton())
            listViewButton()->setChecked(true);
        if (iconViewButton())
            iconViewButton()->setChecked(false);
    }
}

QWidget *BrowseWindowPrivate::currentDisplayView()
{
    if (!splitterIns) return nullptr;
    return splitterIns->widget(1);
}

BrowseWindowPrivate::BrowseWindowPrivate(BrowseWindow *qq)
    : QObject(qq),
      q(qq)
{
    initDefaultLayout();
    qq->installEventFilter(this);
}

BrowseWindowPrivate::~BrowseWindowPrivate()
{
}

void BrowseWindowPrivate::setDefaultViewMode(QListView::ViewMode mode)
{
    BrowseWindowPrivate::mode = mode;
}

void BrowseWindowPrivate::setRootUrl(const QUrl &url)
{
    if (!viewIsAdded(url.scheme())) {
        auto view = BrowseWidgetFactory::create<BrowseView>(url);
        //绑定view点击的触发逻辑
        QObject::connect(view, &FileView::dirClicked,
                         this, &BrowseWindowPrivate::setRootUrl,
                         Qt::UniqueConnection);
        addView(url.scheme(), view);
    }

    //添加url到历史导航
    if (navWidgetIns && sender() != navWidgetIns) {
        navWidgetIns->appendUrl(url);
        QObject::connect(navWidgetIns, &NavWidget::releaseUrl,
                         this, &BrowseWindowPrivate::setRootUrl,
                         Qt::UniqueConnection);
    }

    //切换麵包屑
    if (crumbBarIns) {
        showCrumbBar();
        crumbBarIns->setRootUrl(url);
    }

    //切换sidebar
    if (sidebarIns)
        sidebarIns->setCurrentUrl(url);

    if (addressBarIns) {
        QObject::connect(addressBarIns, &AddressBar::editingFinishedUrl,
                         this, &BrowseWindowPrivate::setRootUrl,
                         Qt::UniqueConnection);
        if (sender() == addressBarIns) {
            addressBarIns->stopSpinner();
        }
    }

    //切換view
    if (!displayCheckViewIns.checkViewUrl(url)) {

        qWarning() << Q_FUNC_INFO << "Failed default URL check";
        //显示 checkView(Label)
        return displayCheckViewIns.showBeginLogic();

    } else {

        QString errorString;
        DisplayViewLogic *viewLogic = views.value(url.scheme());
        QWidget *viewWidget = dynamic_cast<QWidget *>(viewLogic);
        if (!viewLogic) {   // scheme 关联的view url检查失败
            displayCheckViewIns.setText(QObject::tr("You can't use view to display. "
                                                    "You need to add view before that"));

            //取消功能按钮的显示
            optionButtonBoxIns->hide();
            //显示 checkView(Label)
            if (&displayCheckViewIns != splitterIns->widget(1))
                splitterIns->replaceWidget(1, &displayCheckViewIns);

            return displayCheckViewIns.showBeginLogic();
        }

        if (viewLogic && !viewLogic->checkViewUrl(url, &errorString)) {
            //设置当前错误信息
            displayCheckViewIns.setText(errorString);
            //取消功能按钮的显示
            optionButtonBoxIns->hide();
            //依然显示 checkView(Label)
            return displayCheckViewIns.showBeginLogic();
        } else {   //正常show logic

            if (splitterIns && viewWidget != splitterIns->widget(1))
                splitterIns->replaceWidget(1, viewWidget);

            QObject::connect(iconViewButton(), &QToolButton::clicked,
                             this, &BrowseWindowPrivate::doViewModeButtonClicked,
                             Qt::UniqueConnection);

            QObject::connect(listViewButton(), &QToolButton::clicked,
                             this, &BrowseWindowPrivate::doViewModeButtonClicked,
                             Qt::UniqueConnection);

            if (mode == BrowseView::IconMode) {
                if (listViewButton())
                    listViewButton()->setChecked(false);
                if (iconViewButton())
                    iconViewButton()->setChecked(true);
            }

            if (mode == BrowseView::ListMode) {
                if (listViewButton())
                    listViewButton()->setChecked(true);
                if (iconViewButton())
                    iconViewButton()->setChecked(false);
            }

            //设置当前url
            viewLogic->setRootUrl(url);
            //取消默认的CheckView显示
            displayCheckViewIns.showEndLogic();
            //展示当前view
            viewLogic->showBeginLogic();
        }
    }
}

DButtonBoxButton *BrowseWindowPrivate::navBackButton() const
{
    return navWidgetIns->navBackButton();
}

void BrowseWindowPrivate::setNavBackButton(DButtonBoxButton *navBackButton)
{
    return navWidgetIns->setNavBackButton(navBackButton);
}

DButtonBoxButton *BrowseWindowPrivate::navForwardButton() const
{
    return navWidgetIns->navForwardButton();
}

void BrowseWindowPrivate::setNavForwardButton(DButtonBoxButton *navForwardButton)
{
    return navWidgetIns->setNavForwardButton(navForwardButton);
}

void BrowseWindowPrivate::setSearchButton(QToolButton *searchButton)
{

    if (!titleBarLayoutIns->replaceWidget(searchButton, searchButton)->isEmpty()) {

        if (searchButton) {
            delete searchButton;
            searchButton = nullptr;
        }

        if (!searchButton)
            searchButtonIns = searchButton;

        if (searchButton->icon().isNull())
            searchButton->setIcon(QIcon::fromTheme("search"));

        searchButton->setFixedSize({ 36, 36 });
        searchButton->setFocusPolicy(Qt::NoFocus);
        searchButton->setIconSize({ 16, 16 });

        QObject::connect(searchButton, &QToolButton::clicked,
                         this, &BrowseWindowPrivate::doSearchButtonClicked,
                         Qt::UniqueConnection);
    }
}

QToolButton *BrowseWindowPrivate::searchButton() const
{
    return searchButtonIns;
}

QToolButton *BrowseWindowPrivate::searchFilterButton() const
{
    return searchFilterButtonIns;
}

void BrowseWindowPrivate::setSearchFilterButton(QToolButton *searchFilterButton)
{
    searchFilterButtonIns = searchFilterButton;
}

OptionButtonBox *BrowseWindowPrivate::optionButtonBox() const
{
    return optionButtonBoxIns;
}

void BrowseWindowPrivate::setOptionButtonBox(OptionButtonBox *optionButtonBox)
{
    optionButtonBoxIns = optionButtonBox;
}

QToolButton *BrowseWindowPrivate::listViewButton() const
{
    if (optionButtonBox())
        return optionButtonBox()->listViewButton();
    return nullptr;
}

void BrowseWindowPrivate::setListViewButton(QToolButton *button)
{
    if (optionButtonBox())
        optionButtonBox()->setListViewButton(button);
}

QToolButton *BrowseWindowPrivate::iconViewButton() const
{
    if (optionButtonBox())
        return optionButtonBox()->iconViewButton();
    return nullptr;
}

void BrowseWindowPrivate::setIconViewButton(QToolButton *button)
{
    if (optionButtonBox())
        optionButtonBox()->setIconViewButton(button);
}

CrumbBar *BrowseWindowPrivate::crumbBar() const
{
    return crumbBarIns;
}

void BrowseWindowPrivate::setCrumbBar(CrumbBar *crumbBar)
{
    crumbBarIns = crumbBar;
}

void BrowseWindowPrivate::addView(const QString &scheme, DisplayViewLogic *logic)
{
    auto viewWidget = dynamic_cast<QWidget *>(logic);
    if (!viewWidget) {
        qWarning() << Q_FUNC_INFO
                   << "Error, can't to add logic view, cause not inherit QWidget";
    } else {
        views.insert(scheme, logic);
    }
}

bool BrowseWindowPrivate::viewIsAdded(const QString &scheme)
{
    if (nullptr == views.value(scheme))
        return false;
    else
        return true;
}

QWidget *BrowseWindowPrivate::propertyView() const
{
    return propertyViewIns;
}

void BrowseWindowPrivate::setPropertyView(QWidget *propertyView)
{
    propertyViewIns = propertyView;
}

SideBar *BrowseWindowPrivate::sidebar() const
{
    return sidebarIns;
}

void BrowseWindowPrivate::setSidebar(SideBar *sidebar)
{
    sidebarIns = sidebar;
}

void BrowseWindowPrivate::initDefaultLayout()
{
    q->titlebar()->setIcon(QIcon::fromTheme("dde-file-manager",
                                            QIcon::fromTheme("system-file-manager")));

    if (!titleBarIns) {
        titleBarIns = new QFrame;
    }

    if (!titleBarLayoutIns) {
        titleBarLayoutIns = new QHBoxLayout(titleBarIns);
        titleBarLayoutIns->setMargin(0);
        titleBarLayoutIns->setSpacing(0);
    }

    if (!navWidgetIns) {
        navWidgetIns = new NavWidget;
        titleBarLayoutIns->addSpacing(14);   //导航栏左侧间隔14
        titleBarLayoutIns->addWidget(navWidgetIns, 0, Qt::AlignLeft);
    }

    if (!addressBarIns) {
        addressBarIns = new AddressBar;
        addressBarIns->setFixedHeight(36);
        addressBarIns->installEventFilter(this);
        titleBarLayoutIns->addSpacing(4);
        titleBarLayoutIns->addWidget(addressBarIns);
    }

    if (!crumbBarIns) {
        crumbBarIns = new CrumbBar;
        titleBarLayoutIns->addWidget(crumbBarIns);
    }

    if (!searchButtonIns) {
        searchButtonIns = new QToolButton;
        searchButtonIns->setFixedSize({ 36, 36 });
        searchButtonIns->setFocusPolicy(Qt::NoFocus);
        searchButtonIns->setIcon(QIcon::fromTheme("search"));
        searchButtonIns->setIconSize({ 16, 16 });
        titleBarLayoutIns->addWidget(searchButtonIns);
        QObject::connect(searchButtonIns, &QToolButton::clicked,
                         this, &BrowseWindowPrivate::doSearchButtonClicked,
                         Qt::UniqueConnection);
    }

    if (!searchFilterButtonIns) {
        searchFilterButtonIns = new QToolButton;
        searchFilterButtonIns->setFixedSize({ 36, 36 });
        searchFilterButtonIns->setFocusPolicy(Qt::NoFocus);
        searchFilterButtonIns->setIcon(QIcon::fromTheme("dfview_filter"));
        searchFilterButtonIns->setIconSize({ 16, 16 });
        titleBarLayoutIns->addWidget(searchFilterButtonIns);
    }

    if (!optionButtonBoxIns) {
        optionButtonBoxIns = new OptionButtonBox;
        titleBarLayoutIns->addSpacing(4);
        titleBarLayoutIns->addWidget(optionButtonBoxIns, 0, Qt::AlignRight);
    }

    if (titleBarIns) {
        titleBarIns->setLayout(titleBarLayoutIns);
        titleBarIns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        q->titlebar()->setCustomWidget(titleBarIns);
    }

    if (!splitterIns) {
        splitterIns = new Splitter(Qt::Orientation::Horizontal);
        splitterIns->setChildrenCollapsible(false);
        splitterIns->setHandleWidth(0);

        if (!q->centralWidget())
            q->setCentralWidget(splitterIns);
    }

    if (!sidebarIns)
        sidebarIns = new SideBar;

    splitterIns->addWidget(sidebarIns);

    if (displayCheckViewIns.text().isEmpty())
        displayCheckViewIns.setText(displayCheckViewIns.metaObject()->className());

    splitterIns->addWidget(&displayCheckViewIns);

    if (!displayWidgetIns)
        displayWidgetIns = new QWidget;

    if (!displayViewLayoutIns)
        displayViewLayoutIns = new QVBoxLayout;

    displayWidgetIns->setLayout(displayViewLayoutIns);
    splitterIns->addWidget(displayWidgetIns);

    showCrumbBar();
}

void BrowseWindowPrivate::showAddrsssBar()
{
    crumbBarIns->hide();
    addressBarIns->show();
    addressBarIns->setFocus();
}

void BrowseWindowPrivate::showCrumbBar()
{
    showSearchButton();

    if (crumbBarIns) crumbBarIns->show();

    if (addressBarIns) {
        addressBarIns->clear();
        addressBarIns->hide();
    }

    if (q) q->setFocus();
}

void BrowseWindowPrivate::showSearchButton()
{
    if (searchButtonIns) searchButtonIns->show();
    if (searchFilterButtonIns) searchFilterButtonIns->hide();
}

void BrowseWindowPrivate::showSearchFilterButton()
{
    if (searchButtonIns) searchButtonIns->hide();
    if (searchFilterButtonIns) searchFilterButtonIns->show();
}

bool BrowseWindowPrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == q && event->type() == QEvent::Show) {
        q->activateWindow();
        return false;
    }

    //    if (watched == addressBar() && event->type() == QEvent::FocusOut) {
    //        showCrumbBar();
    //        return false;
    //    }

    if (watched == addressBar() && event->type() == QEvent::Hide) {
        showCrumbBar();
        return true;
    }

    return false;
}

DSB_FM_END_NAMESPACE
