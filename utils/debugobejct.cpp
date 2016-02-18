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
