/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef ACCESSIBLE_FUNCTIONS_H
#define ACCESSIBLE_FUNCTIONS_H

// 为了方便使用,把相关定义独立出来,如有需要,直接包含这个头文件,然后使用SET_*的宏去设置,USE_*宏开启即可
/* 宏参数说明
* classname:类名,例如DLineEdit
* accessiblename:accessible唯一标识,需保证唯一性[getAccessibleName函数处理],优先使用QObject::setAccessibleName值
* accessibletype:即QAccessible::Role,表示标识控件的类型
* classobj:QObject指针
* accessdescription:accessible描述内容,考虑到暂时用不到,目前都默认为空,有需要可自行设计接口
*
* 部分创建宏说明
* FUNC_CREATE:创建构造函数
* FUNC_PRESS:创建Press接口
* FUNC_SHOWMENU:创建右键菜单接口
* FUNC_PRESS_SHOWMENU:上两者的综合
* FUNC_RECT:实现rect接口
* FUNC_TEXT:实现text接口
* USE_ACCESSIBLE:对传入的类型设置其accessible功能
* USE_ACCESSIBLE_BY_OBJECTNAME:同上,[指定objectname]---适用同一个类，但objectname不同的情况
*
* 设置为指定类型的Accessible控件宏
* SET_BUTTON_ACCESSIBLE_PRESS_SHOWMENU:button类型,添加press和showmenu功能
* SET_BUTTON_ACCESSIBLE_SHOWMENU:button类型,添加showmenu功能
* SET_BUTTON_ACCESSIBLE:button类型,添加press功能
* SET_LABEL_ACCESSIBLE:label类型,用于标签控件
* SET_FORM_ACCESSIBLE:form类型,用于widget控件
* SET_SLIDER_ACCESSIBLE:slider类型,用于滑块控件
* SET_SEPARATOR_ACCESSIBLE:separator类型,用于分隔符控件
*/
#include <QAccessible>
#include <QAccessibleWidget>
#include <QEvent>
#include <QMap>
#include <QString>
#include <QWidget>
#include <QObject>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QApplication>

#define SEPARATOR "_"

inline QString getObjPrefix(QAccessible::Role r)
{
    // 按照类型添加固定前缀
    QMetaEnum metaEnum = QMetaEnum::fromType<QAccessible::Role>();
    QByteArray prefix = metaEnum.valueToKeys(r);
    switch (r) {
    case QAccessible::Button:       prefix = "Btn";         break;
    case QAccessible::StaticText:   prefix = "Label";       break;
    default:                        break;
    }
    return QString::fromLatin1(prefix);
}

inline QString getIntelAccessibleName(QWidget *w, QAccessible::Role r, QString fallback)
{
    // 避免重复生成
    static QMap< QObject *, QString > objnameMap;
    if (!objnameMap[w].isEmpty())
        return objnameMap[w];

    static QMap< QAccessible::Role, QList< QString > > accessibleMap;
    QString oldAccessName = w->accessibleName();
    oldAccessName.replace(SEPARATOR, "");

    // 再加上标识
    QString accessibleName = "";//getObjPrefix(r) + SEPARATOR;
    accessibleName += oldAccessName.isEmpty() ? fallback : oldAccessName;
    // 检查名称是否唯一
    if (accessibleMap[r].contains(accessibleName)) {
        if (objnameMap.key(accessibleName)) {
            objnameMap.remove(objnameMap.key(accessibleName));
            objnameMap.insert(w, accessibleName);
            return accessibleName;
        }
        // 获取编号，然后+1
        int pos = accessibleName.indexOf(SEPARATOR);
        int id = accessibleName.mid(pos + 1).toInt();

        QString newAccessibleName;
        do {
            // 一直找到一个不重复的名字
            newAccessibleName = accessibleName + SEPARATOR + QString::number(++id);
        } while (accessibleMap[r].contains(newAccessibleName));

        accessibleMap[r].append(newAccessibleName);
        objnameMap.insert(w, newAccessibleName);

        return newAccessibleName;
    } else {
        accessibleMap[r].append(accessibleName);
        objnameMap.insert(w, accessibleName);

        return accessibleName;
    }
}

inline QString getAccessibleName(QWidget *w, QAccessible::Role r, QString fallback)
{
    QString accessibleName = getIntelAccessibleName(w, r, fallback);
    if(accessibleName.isEmpty())
    {
        return getObjPrefix(r) + SEPARATOR;
    }

    return accessibleName;
}

#define FUNC_CREATE(classname,accessibletype,accessdescription)    Accessible##classname(classname *w) \
    : QAccessibleWidget(w,accessibletype,#classname)\
    , m_w(w)\
    , m_description(accessdescription)\
{}\
    private:\
    classname *m_w;\
    QString m_description;\

#define FUNC_PRESS(classobj)     QStringList actionNames() const override{\
    if(!classobj->isEnabled())\
    return QStringList();\
    return QStringList() << pressAction();}\
    void doAction(const QString &actionName) override{\
    if(actionName == pressAction())\
{\
    QPointF localPos = classobj->geometry().center();\
    QMouseEvent event(QEvent::MouseButtonPress,localPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);\
    QMouseEvent event2(QEvent::MouseButtonRelease,localPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);\
    qApp->sendEvent(classobj,&event);\
    qApp->sendEvent(classobj,&event2);\
    }\
    }\

#define FUNC_SHOWMENU(classobj)     QStringList actionNames() const override{\
    if(!classobj->isEnabled())\
    return QStringList();\
    return QStringList() << showMenuAction();}\
    void doAction(const QString &actionName) override{\
    if(actionName == showMenuAction())\
{\
    QPointF localPos = classobj->geometry().center();\
    QMouseEvent event(QEvent::MouseButtonPress,localPos,Qt::RightButton,Qt::RightButton,Qt::NoModifier);\
    QMouseEvent event2(QEvent::MouseButtonRelease,localPos,Qt::RightButton,Qt::RightButton,Qt::NoModifier);\
    qApp->sendEvent(classobj,&event);\
    qApp->sendEvent(classobj,&event2);\
    }\
    }\

#define FUNC_PRESS_SHOWMENU(classobj)     QStringList actionNames() const override{\
    if(!classobj->isEnabled())\
    return QStringList();\
    return QStringList() << pressAction() << showMenuAction();}\
    void doAction(const QString &actionName) override{\
    if(actionName == pressAction())\
{\
    QPointF localPos = classobj->geometry().center();\
    QMouseEvent event(QEvent::MouseButtonPress,localPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);\
    QMouseEvent event2(QEvent::MouseButtonRelease,localPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);\
    qApp->sendEvent(classobj,&event);\
    qApp->sendEvent(classobj,&event2);\
    }\
    else if(actionName == showMenuAction())\
{\
    QPointF localPos = classobj->geometry().center();\
    QMouseEvent event(QEvent::MouseButtonPress,localPos,Qt::RightButton,Qt::RightButton,Qt::NoModifier);\
    QMouseEvent event2(QEvent::MouseButtonRelease,localPos,Qt::RightButton,Qt::RightButton,Qt::NoModifier);\
    qApp->sendEvent(classobj,&event);\
    qApp->sendEvent(classobj,&event2);\
    }\
    }\

#define FUNC_RECT(classobj) QRect rect() const override{\
    if (!classobj->isVisible())\
    return QRect();\
    return classobj->geometry();\
    }\

#define FUNC_TEXT(classname,accessiblename) QString Accessible##classname::text(QAccessible::Text t) const{\
    switch (t) {\
    case QAccessible::Name:\
    return getAccessibleName(m_w, this->role(), accessiblename);\
    case QAccessible::Description:\
    return m_description;\
    default:\
    return QString();\
    }\
    }\

#define USE_ACCESSIBLE(classnamestring,classname)    if (classnamestring == QLatin1String(#classname) && object && object->isWidgetType())\
{\
    interface = new Accessible##classname(static_cast<classname *>(object));\
    }\

// [指定objectname]---适用同一个类，但objectname不同的情况
#define USE_ACCESSIBLE_BY_OBJECTNAME(classnamestring,classname,objectname)    if (classnamestring == QLatin1String(#classname) && object && (object->objectName() == objectname) && object->isWidgetType())\
{\
    interface = new Accessible##classname(static_cast<classname *>(object));\
    }\

#define SET_BUTTON_ACCESSIBLE_PRESS_DESCRIPTION(classname,accessiblename,accessdescription)  class Accessible##classname : public QAccessibleWidget\
{\
    public:\
    FUNC_CREATE(classname,QAccessible::Button,accessdescription)\
    QString text(QAccessible::Text t) const override;\
    FUNC_PRESS(m_w)\
    };\

#define SET_BUTTON_ACCESSIBLE_SHOWMENU_DESCRIPTION(classname,accessiblename,accessdescription)  class Accessible##classname : public QAccessibleWidget\
{\
    public:\
    FUNC_CREATE(classname,QAccessible::Button,accessdescription)\
    QString text(QAccessible::Text t) const override;\
    FUNC_SHOWMENU(m_w)\
    };\

#define SET_BUTTON_ACCESSIBLE_PRESS_SHOEMENU_DESCRIPTION(classname,accessiblename,accessdescription)  class Accessible##classname : public QAccessibleWidget\
{\
    public:\
    FUNC_CREATE(classname,QAccessible::Button,accessdescription)\
    QString text(QAccessible::Text t) const override;\
    FUNC_PRESS_SHOWMENU(m_w)\
    };\

#define SET_LABEL_ACCESSIBLE_WITH_DESCRIPTION(classname,aaccessibletype,accessiblename,accessdescription)  class Accessible##classname : public QAccessibleWidget\
{\
    public:\
    FUNC_CREATE(classname,aaccessibletype,accessdescription)\
    QString text(QAccessible::Text t) const override;\
    FUNC_RECT(m_w)\
    };\

// /*******************************************简化使用*******************************************/
#define SET_BUTTON_ACCESSIBLE_PRESS_SHOWMENU(classname,accessiblename)         SET_BUTTON_ACCESSIBLE_PRESS_SHOEMENU_DESCRIPTION(classname,accessiblename,"")\
    FUNC_TEXT(classname,accessiblename)

#define SET_BUTTON_ACCESSIBLE_SHOWMENU(classname,accessiblename)               SET_BUTTON_ACCESSIBLE_SHOWMENU_DESCRIPTION(classname,accessiblename,"")\
    FUNC_TEXT(classname,accessiblename)

#define SET_BUTTON_ACCESSIBLE(classname,accessiblename)                        SET_BUTTON_ACCESSIBLE_PRESS_DESCRIPTION(classname,accessiblename,"")\
    FUNC_TEXT(classname,accessiblename)

#define SET_LABEL_ACCESSIBLE(classname,accessiblename)                         SET_LABEL_ACCESSIBLE_WITH_DESCRIPTION(classname,QAccessible::StaticText,accessiblename,"")\
    FUNC_TEXT(classname,accessiblename)

#define SET_FORM_ACCESSIBLE(classname,accessiblename)                          SET_LABEL_ACCESSIBLE_WITH_DESCRIPTION(classname,QAccessible::Form,accessiblename,"");\
    FUNC_TEXT(classname,accessiblename)

#define SET_SLIDER_ACCESSIBLE(classname,accessiblename)                        SET_LABEL_ACCESSIBLE_WITH_DESCRIPTION(classname,QAccessible::Slider,accessiblename,"")\
    FUNC_TEXT(classname,accessiblename)

#define SET_SEPARATOR_ACCESSIBLE(classname,accessiblename)                     SET_LABEL_ACCESSIBLE_WITH_DESCRIPTION(classname,QAccessible::Separator,accessiblename,"")\
    FUNC_TEXT(classname,accessiblename)
/************************************************************************************************/

#endif // ACCESSIBLE_FUNCTIONS_H
