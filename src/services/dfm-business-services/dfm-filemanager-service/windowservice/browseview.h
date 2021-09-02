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
#ifndef DFMBROWSEVIEW_H
#define DFMBROWSEVIEW_H

#include "displayviewlogic.h"
#include "dfm_filemanager_service_global.h"

#include "dfm-base/widgets/dfmfileview/fileview.h"

#include <QObject>
#include <QWidget>

DSB_FM_BEGIN_NAMESPACE

class DFMBrowseView : public DFMFileView, public DFMDisplayViewLogic
{
    Q_OBJECT

public:
    explicit DFMBrowseView(QWidget *parent = nullptr);

    virtual ~DFMBrowseView() override;

    virtual void setRootUrl(const QUrl &url) override;

    virtual QUrl rootUrl() override;
};


template <class T>
class DFMSchemeWidegtFactory
{
    Q_DISABLE_COPY(DFMSchemeWidegtFactory)

    //定义创建函数类型
    typedef std::function<T*()> CreateFunc;

    //构造函数列表
    QHash<QString, CreateFunc> _constructList{};

public:
    DFMSchemeWidegtFactory(){}

    template<class CT = T>
    bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        if (_constructList[scheme]) {
            if (errorString)
                *errorString = QObject::tr("The current scheme has registered "
                                           "the associated construction class");
            return false;
        }

        CreateFunc foo = [=](){
            return new CT();
        };
        _constructList.insert(scheme,foo);
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

        CreateFunc constantFunc = _constructList.value(scheme);
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

class DFMBrowseWidgetFactory : public DFMSchemeWidegtFactory<DFMDisplayViewLogic>
{
    Q_DISABLE_COPY(DFMBrowseWidgetFactory)

public:

    DFMBrowseWidgetFactory() {}

    //提供任意子类的转换方法模板，仅限DFMBrowseView树族
    //与qSharedPointerDynamicCast保持一致
    template<class T>
    T* create(const QUrl &url, QString *errorString = nullptr)
    {
        return dynamic_cast<T*>(DFMSchemeWidegtFactory<DFMDisplayViewLogic>::create(url, errorString));
    }

    static DFMBrowseWidgetFactory& instance();
};

DSB_FM_END_NAMESPACE

#endif // DFMBROWSEVIEW_H
