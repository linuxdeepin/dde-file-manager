/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "windowservice/browseview.h"

#include "dfm-framework/log/frameworklog.h"

DSB_FM_BEGIN_NAMESPACE

DFMAddressBar *DFMBrowseWindowPrivate::addressBar() const
{
    return m_addressBar;
}

void DFMBrowseWindowPrivate::setAddressBar(DFMAddressBar *addressBar)
{
    if (!m_titleBarLayout) return;

    if (!m_titleBarLayout->replaceWidget(addressBar,m_addressBar)->isEmpty())
    {
        if (m_addressBar) {
            delete m_addressBar;
            m_addressBar = nullptr;
        }

        if (m_addressBar)
        {
            m_addressBar = addressBar;
        }
    }
}

void DFMBrowseWindowPrivate::doSearchButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    showAddrsssBar();
    m_searchButton->hide();
}

void DFMBrowseWindowPrivate::doViewModeButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    DFMBrowseView* view = qobject_cast<DFMBrowseView*>(currentDisplayView());

    if (!view || !sender() || !optionButtonBox()
            || !listViewButton() || !iconViewButton()) return ;

    if (sender() == listViewButton()) {
        iconViewButton()->setChecked(false);
        view->setViewMode(DFMBrowseView::ViewMode::ListMode);
    }

    if (sender() == iconViewButton()) {
        listViewButton()->setChecked(false);
        view->setViewMode(DFMBrowseView::ViewMode::IconMode);
    }
}

QWidget *DFMBrowseWindowPrivate::currentDisplayView()
{
    if (!m_splitter) return nullptr;
    return m_splitter->widget(1);
}

DFMBrowseWindowPrivate::DFMBrowseWindowPrivate(DFMBrowseWindow *qq)
    : QObject(qq),
      q_ptr(qq)
{
    initDefaultLayout();
    qq->installEventFilter(this);
}

void DFMBrowseWindowPrivate::setDefaultViewMode(QListView::ViewMode mode)
{
    m_mode = mode;
}

void DFMBrowseWindowPrivate::setRootUrl(const QUrl &url)
{
    //添加url到历史导航
    if (m_navWidget && sender() != m_navWidget) {
        m_navWidget->appendUrl(url);
        QObject::connect(m_navWidget, &DFMNavWidget::releaseUrl,
                         this, &DFMBrowseWindowPrivate::setRootUrl,
                         Qt::UniqueConnection);
    }

    //切换麵包屑
    if (m_crumbBar)
        m_crumbBar->setRootUrl(url);

    //切换sidebar
    if (m_sidebar)
        m_sidebar->setCurrentUrl(url);

    //切換view
    if (!m_displayCheckView.checkViewUrl(url)) {

        qWarning() << Q_FUNC_INFO << "Failed default URL check";
        //显示 checkView(Label)
        return m_displayCheckView.showBeginLogic();

    } else {

        QString errorString;
        DFMDisplayViewLogic* viewLogic = m_views.value(url.scheme());
        QWidget* viewWidget = dynamic_cast<QWidget*>(viewLogic);
        if (!viewLogic) { // scheme 关联的view url检查失败
            m_displayCheckView.setText(QObject::tr("You can't use view to display. "
                                                   "You need to add view before that"));

            //取消功能按钮的显示
            m_optionButtonBox->hide();
            //显示 checkView(Label)
            if (&m_displayCheckView != m_splitter->widget(1))
                m_splitter->replaceWidget(1, &m_displayCheckView);

            return m_displayCheckView.showBeginLogic();
        }

        if (viewLogic && !viewLogic->checkViewUrl(url, &errorString)) {
            //设置当前错误信息
            m_displayCheckView.setText(errorString);
            //取消功能按钮的显示
            m_optionButtonBox->hide();
            //依然显示 checkView(Label)
            return m_displayCheckView.showBeginLogic();
        } else { //正常show logic

            if (m_splitter && viewWidget != m_splitter->widget(1))
                m_splitter->replaceWidget(1, viewWidget);

            QObject::connect(iconViewButton(), &QToolButton::clicked,
                             this, &DFMBrowseWindowPrivate::doViewModeButtonClicked,
                             Qt::UniqueConnection);

            QObject::connect(listViewButton(), &QToolButton::clicked,
                             this, &DFMBrowseWindowPrivate::doViewModeButtonClicked,
                             Qt::UniqueConnection);

            if (m_mode == DFMBrowseView::IconMode) {
                if (listViewButton())
                    listViewButton()->setChecked(false);
                if (iconViewButton())
                    iconViewButton()->setChecked(true);
            }

            if (m_mode == DFMBrowseView::ListMode) {
                if (listViewButton())
                    listViewButton()->setChecked(true);
                if (iconViewButton())
                    iconViewButton()->setChecked(false);
            }

            //设置当前url
            viewLogic->setRootUrl(url);
            //取消默认的CheckView显示
            m_displayCheckView.showEndLogic();
            //展示当前view
            viewLogic->showBeginLogic();
        }

    }
}

DButtonBoxButton *DFMBrowseWindowPrivate::navBackButton() const
{
    return m_navWidget->navBackButton();
}

void DFMBrowseWindowPrivate::setNavBackButton(DButtonBoxButton *navBackButton)
{
    return m_navWidget->setNavBackButton(navBackButton);
}

DButtonBoxButton *DFMBrowseWindowPrivate::navForwardButton() const
{
    return m_navWidget->navForwardButton();
}

void DFMBrowseWindowPrivate::setNavForwardButton(DButtonBoxButton *navForwardButton)
{
    return m_navWidget->setNavForwardButton(navForwardButton);
}

void DFMBrowseWindowPrivate::setSearchButton(QToolButton *searchButton)
{

    if (!m_titleBarLayout->replaceWidget(searchButton,m_searchButton)->isEmpty()) {

        if (m_searchButton) {
            delete m_searchButton;
            m_searchButton = nullptr;
        }

        if (!m_searchButton)
            m_searchButton = searchButton;

        if (m_searchButton->icon().isNull())
            m_searchButton->setIcon(QIcon::fromTheme("search"));

        m_searchButton->setFixedSize({36,36});
        m_searchButton->setFocusPolicy(Qt::NoFocus);
        m_searchButton->setIconSize({16,16});

        QObject::connect(m_searchButton, &QToolButton::clicked,
                         this, &DFMBrowseWindowPrivate::doSearchButtonClicked,
                         Qt::UniqueConnection);
    }
}

QToolButton *DFMBrowseWindowPrivate::searchButton() const
{
    return m_searchButton;
}

QToolButton *DFMBrowseWindowPrivate::searchFilterButton() const
{
    return m_searchFilterButton;
}

void DFMBrowseWindowPrivate::setSearchFilterButton(QToolButton *searchFilterButton)
{
    m_searchFilterButton = searchFilterButton;
}

DFMOptionButtonBox *DFMBrowseWindowPrivate::optionButtonBox() const
{
    return m_optionButtonBox;
}

void DFMBrowseWindowPrivate::setOptionButtonBox(DFMOptionButtonBox *optionButtonBox)
{
    m_optionButtonBox = optionButtonBox;
}

QToolButton *DFMBrowseWindowPrivate::listViewButton() const
{
    if (optionButtonBox())
        return optionButtonBox()->listViewButton();
    return nullptr;
}

void DFMBrowseWindowPrivate::setListViewButton(QToolButton *button)
{
    if (optionButtonBox())
        optionButtonBox()->setListViewButton(button);
}

QToolButton *DFMBrowseWindowPrivate::iconViewButton() const
{
    if (optionButtonBox())
        return optionButtonBox()->iconViewButton();
    return nullptr;
}

void DFMBrowseWindowPrivate::setIconViewButton(QToolButton *button){
    if (optionButtonBox())
        optionButtonBox()->setIconViewButton(button);
}

DFMCrumbBar *DFMBrowseWindowPrivate::crumbBar() const
{
    return m_crumbBar;
}

void DFMBrowseWindowPrivate::setCrumbBar(DFMCrumbBar *crumbBar)
{
    m_crumbBar = crumbBar;
}

void DFMBrowseWindowPrivate::addview(const QString &scheme, DFMDisplayViewLogic* logic)
{
    auto viewWidget = dynamic_cast<QWidget*>(logic);
    if (!viewWidget) {
        qWarning() << Q_FUNC_INFO
                   << "Error, can't to add logic view, "
                      "cause not inherit QWidget";
    } else {
        m_views.insert(scheme,logic);
    }
}

bool DFMBrowseWindowPrivate::viewIsAdded(const QString &scheme)
{
    return m_views.value(scheme);
}

QWidget *DFMBrowseWindowPrivate::propertyView() const
{
    return m_propertyView;
}

void DFMBrowseWindowPrivate::setPropertyView(QWidget *propertyView)
{
    m_propertyView = propertyView;
}

DFMSideBar *DFMBrowseWindowPrivate::sidebar() const
{
    return m_sidebar;
}

void DFMBrowseWindowPrivate::setSidebar(DFMSideBar *sidebar)
{
    m_sidebar = sidebar;
}

void DFMBrowseWindowPrivate::initDefaultLayout()
{
    q_ptr->titlebar()->setIcon(QIcon::fromTheme("dde-file-manager",
                                                QIcon::fromTheme("system-file-manager")));

    if (!m_titleBar) {
        m_titleBar = new QFrame;
    }

    if (!m_titleBarLayout) {
        m_titleBarLayout = new QHBoxLayout(m_titleBar);
        m_titleBarLayout->setMargin(0);
        m_titleBarLayout->setSpacing(0);
    }

    if (!m_navWidget) {
        m_navWidget = new DFMNavWidget;
        m_titleBarLayout->addSpacing(14); //导航栏左侧间隔14
        m_titleBarLayout->addWidget(m_navWidget,0,Qt::AlignLeft);
    }

    if (!m_addressBar) {
        m_addressBar = new DFMAddressBar;
        m_addressBar->setFixedHeight(36);
        m_addressBar->installEventFilter(this);
        m_titleBarLayout->addSpacing(4);
        m_titleBarLayout->addWidget(m_addressBar);
    }

    if (!m_crumbBar) {
        m_crumbBar = new DFMCrumbBar;
        m_titleBarLayout->addWidget(m_crumbBar);
    }

    if (!m_searchButton) {
        m_searchButton = new QToolButton;
        m_searchButton->setFixedSize({36,36});
        m_searchButton->setFocusPolicy(Qt::NoFocus);
        m_searchButton->setIcon(QIcon::fromTheme("search"));
        m_searchButton->setIconSize({16,16});
        m_titleBarLayout->addWidget(m_searchButton);
        QObject::connect(m_searchButton, &QToolButton::clicked,
                         this, &DFMBrowseWindowPrivate::doSearchButtonClicked,
                         Qt::UniqueConnection);
    }

    if (!m_searchFilterButton) {
        m_searchFilterButton = new QToolButton;
        m_searchFilterButton->setFixedSize({36,36});
        m_searchFilterButton->setFocusPolicy(Qt::NoFocus);
        m_searchFilterButton->setIcon(QIcon::fromTheme("dfm_view_filter"));
        m_searchFilterButton->setIconSize({16,16});
        m_titleBarLayout->addWidget(m_searchFilterButton);
    }

    if (!m_optionButtonBox) {
        m_optionButtonBox = new DFMOptionButtonBox;
        m_titleBarLayout->addSpacing(4);
        m_titleBarLayout->addWidget(m_optionButtonBox,0,Qt::AlignRight);
    }

    if (m_titleBar) {
        m_titleBar->setLayout(m_titleBarLayout);
        m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        q_ptr->titlebar()->setCustomWidget(m_titleBar);
    }

    if (!m_splitter) {
        m_splitter = new DFMSplitter(Qt::Orientation::Horizontal);
        m_splitter->setChildrenCollapsible(false);
        m_splitter->setHandleWidth(0);

        if (!q_ptr->centralWidget())
            q_ptr->setCentralWidget(m_splitter);
    }

    if (!m_sidebar)
        m_sidebar = new DFMSideBar;

    m_splitter->addWidget(m_sidebar);

    if (m_displayCheckView.text().isEmpty())
        m_displayCheckView.setText(m_displayCheckView.metaObject()->className());

    m_splitter->addWidget(&m_displayCheckView);

    if (!m_displayWidget)
        m_displayWidget = new QWidget;

    if (!m_displayViewLayout)
        m_displayViewLayout = new QVBoxLayout;

    m_displayWidget->setLayout(m_displayViewLayout);
    m_splitter->addWidget(m_displayWidget);

    showCrumbBar();
}

void DFMBrowseWindowPrivate::showAddrsssBar()
{
    m_crumbBar->hide();
    m_addressBar->show();
    m_addressBar->setFocus();
}

void DFMBrowseWindowPrivate::showCrumbBar()
{
    showSearchButton();

    if (m_crumbBar) m_crumbBar->show();

    if (m_addressBar) {
        m_addressBar->clear();
        m_addressBar->hide();
    }

    if (q_ptr) q_ptr->setFocus();
}

void DFMBrowseWindowPrivate::showSearchButton()
{
    if (m_searchButton) m_searchButton->show();
    if (m_searchFilterButton) m_searchFilterButton->hide();
}

void DFMBrowseWindowPrivate::showSearchFilterButton()
{
    if (m_searchButton) m_searchButton->hide();
    if (m_searchFilterButton) m_searchFilterButton->show();
}

bool DFMBrowseWindowPrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == q_ptr && event->type() == QEvent::Show) {
        q_ptr->activateWindow();
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
