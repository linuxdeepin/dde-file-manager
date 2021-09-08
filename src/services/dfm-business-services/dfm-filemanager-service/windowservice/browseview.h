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
#ifndef BrowseView_H
#define BrowseView_H

#include "displayviewlogic.h"
#include "dfm_filemanager_service_global.h"

#include "dfm-base/widgets/dfmfileview/fileview.h"

#include <QObject>
#include <QWidget>

DFMBASE_USE_NAMESPACE
DSB_FM_BEGIN_NAMESPACE

class BrowseView : public FileView, public DisplayViewLogic
{
    Q_OBJECT
public:
    explicit BrowseView(QWidget *parent = nullptr);
    virtual ~BrowseView() override;
    virtual void setRootUrl(const QUrl &url) override;
    virtual QUrl rootUrl() override;
};

template <class T>
class SchemeWidegtFactory
{
    Q_DISABLE_COPY(SchemeWidegtFactory)
    typedef std::function<T*()> CreateFunc; //定义创建函数类型
    QHash<QString, CreateFunc> constructList{}; //构造函数列表
public:
    explicit SchemeWidegtFactory(){}
    virtual ~SchemeWidegtFactory(){}

    template<class CT = T>
    bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        if (constructList[scheme]) {
            if (errorString)
                *errorString = QObject::tr("The current scheme has registered "
                                           "the associated construction class");
            return false;
        }

        CreateFunc foo = [=](){
            return new CT();
        };
        constructList.insert(scheme,foo);
        return true;
    }

    T* create(const QUrl &url, QString *errorString = nullptr)
    {
        QString scheme = url.scheme();
        if(!UrlRoute::hasScheme(scheme)) {
            if (errorString)
                *errorString = QObject::tr("No scheme found for "
                                           "URL registration");
            return nullptr;
        }

        CreateFunc constantFunc = constructList.value(scheme);
        if (constantFunc) {
            return constantFunc();
        } else {
            if (errorString)
                *errorString = QObject::tr("Scheme should be call registered 'regClass()' function "
                                           "before create function");
            return nullptr;
        }
    }
};

class BrowseWidgetFactory : public SchemeWidegtFactory<DisplayViewLogic>
{
    Q_DISABLE_COPY(BrowseWidgetFactory)
public:
    explicit BrowseWidgetFactory();
    virtual ~BrowseWidgetFactory();
    //提供任意子类的转换方法模板，仅限BrowseView树族
    //与qSharedPointerDynamicCast保持一致
    template<class T>
    T* create(const QUrl &url, QString *errorString = nullptr)
    {
        return dynamic_cast<T*>(SchemeWidegtFactory<DisplayViewLogic>::create(url, errorString));
    }

    static BrowseWidgetFactory& instance();
};

DSB_FM_END_NAMESPACE

#endif // BrowseView_H
