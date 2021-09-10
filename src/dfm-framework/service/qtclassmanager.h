#ifndef QTCLASSMANAGER_H
#define QTCLASSMANAGER_H

#include <QObject>

template<class CT = QObject>
class QtClassManager
{
public:

    virtual ~QtClassManager()
    {
        qDeleteAll(classList);
    }

    virtual bool append(const QString &name, CT *obj, QString *errorString = nullptr)
    {
        if (!obj){
            if (errorString)
                *errorString = QObject::tr("Failed, Can't append the empty class pointer");
            return false;
        }

        auto castPointer = qobject_cast<QObject*>(obj);
        if (castPointer)
            castPointer->setParent(nullptr);

        if (classList[name]) {
             if (errorString)
                 *errorString = QObject::tr("Failed, Objects cannot be added repeatedly");
             return false;
        }
        classList.insert(name,obj);
        return true;
    }

    virtual CT *value(const QString &name) const
    {
        auto res = classList[name];
        return res;
    }

    virtual const QStringList keys() const
    {
        return classList.keys();
    }

    virtual bool remove(const QString &name)
    {
        auto pointer = classList.take(name);
        if (pointer) delete pointer;
        classList.remove(name);
        return true;
    }

protected:
    QHash<QString, CT*> classList;
};

#endif // QTCLASSMANAGER_H
