/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "debugobejct.h"
#include <QMetaProperty>


/*!
 * \brief DObejct::DObejct base class of your obj which you want to support qdebug
 * \param parent
 */
DebugObejct::DebugObejct(QObject *parent) : QObject(parent)
{

}


/*!
 * \brief operator << friend overload qdebug << to debug QObject instance
 * \param dbg debug instance which you use to debug object
 * \param obj instance obj which you want to debug
 * \return
 */
QDebug operator<<(QDebug dbg, const QObject &obj)
{
    QStringList ignoredProperties;
    ignoredProperties << "objectName";
    const QMetaObject *metaobject = obj.metaObject();
    dbg.nospace() << metaobject->className() << "({";
    int count = metaobject->propertyCount();
    for (int i=0; i<count; ++i) {
      QMetaProperty metaproperty = metaobject->property(i);
      const char *name = metaproperty.name();
      if (ignoredProperties.contains(QLatin1String(name)) || (!metaproperty.isReadable()))
        continue;
      QVariant value = obj.property(name);
      if( i <= (count - 1)){
          if (value.toString().count() == 0){
             dbg.nospace() << "\"" << name << "\": [" << value << "],";
          }else{
             dbg.nospace() << "\"" << name << "\":" << value.toString() << ",";
          }
      }
    }
    dbg.nospace() << "}) ";
    return dbg;
}
