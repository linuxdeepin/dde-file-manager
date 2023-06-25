// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THREADCONTAINER_H
#define THREADCONTAINER_H

#include <dfm-base/dfm_base_global.h>

#include <QList>
#include <QMap>
#include <QMutex>
#include <QSharedData>

namespace dfmbase {
/*!
 * \class DThreadList 线程安全的List
 *
 * \brief 对QList进行了封装，加锁对每一个list的操作
 */
template<class T>
class DThreadList : public QSharedData
{
public:
    DThreadList<T>()
        : myList(new QList<T>) {}
    ~DThreadList()
    {
        QMutexLocker lk(&mutex);
        myList->clear();
        delete myList;
        myList = nullptr;
    }
    /*!
     * \brief push_back 从链表的最后插入模板类型
     *
     * \param T 模板类引用
     *
     * \return
     */
    inline void push_back(const T &t)
    {
        myList->push_back(t);
    }
    inline void push_backByLock(const T &t)
    {
        QMutexLocker lk(&mutex);
        myList->push_back(t);
    }
    /*!
     * \brief insert 插入一个模板类型到链表
     *
     * \param T 模板类引用
     *
     * \return
     */
    inline void insertByLock(const T &t)
    {
        QMutexLocker lk(&mutex);
        myList->insert(0, t);
    }
    /*!
     * \brief setList 设置一个链表
     *
     * \param QList<T> 模板类链表
     *
     * \return
     */
    inline void setList(const QList<T> &t)
    {
        QMutexLocker lk(&mutex);
        *myList = t;
    }
    /*!
     * \brief setList 设置一个链表
     *
     * \param QList<T> 模板类链表
     *
     * \return const QList<T> &获取链表
     */
    inline const QList<T> &list()
    {
        return *myList;
    }
    inline const QList<T> &listByLock()
    {
        QMutexLocker lk(&mutex);
        return *myList;
    }
    /*!
     * \brief removeAll 从链表中移除所有的模板类型
     *
     * \param T 模板类引用
     *
     * \return
     */
    inline void removeAllByLock(const T &t)
    {
        QMutexLocker lk(&mutex);
        myList->removeAll(t);
    }
    inline void removeAll(const T &t)
    {
        myList->removeAll(t);
    }
    /*!
     * \brief removeOne 从链表中移除一个的模板类型
     *
     * \param T 模板类引用
     *
     * \return
     */
    inline void removeOneByLock(const T &t)
    {
        QMutexLocker lk(&mutex);
        myList->removeOne(t);
    }

    /*!
     * \brief contains 链表中是否包含模板
     *
     * \param T 模板类引用
     *
     * \return bool 是否包含模板
     */
    inline bool contains(const T &t)
    {
        return myList->contains(t);
    }
    inline bool containsByLock(const T &t)
    {
        QMutexLocker lk(&mutex);
        return myList->contains(t);
    }
    /*!
     * \brief clear 链表中是否包含模板
     *
     * \param T 模板类引用
     *
     * \return bool 是否包含模板
     */
    inline void clearByLock()
    {
        QMutexLocker lk(&mutex);
        return myList->clear();
    }
    /*!
     * \brief append 增加一个元素
     *
     * \param T 模板类引用
     *
     * \return bool 是否包含模板
     */
    inline void appendByLock(const T &t)
    {
        QMutexLocker lk(&mutex);
        return myList->append(t);
    }
    /*!
     * \brief size 链表的总长度
     *
     * \return int 链表的总长度
     */
    inline int size() const
    {
        return myList->size();
    }
    /*!
     * \brief at 链表的总长度
     *
     * \param int 当前第i个位置
     *
     * \return T& 链表的总长度
     */
    inline const T &at(int i) const
    {
        return myList->at(i);
    }
    /*!
     * \brief first 如果链表为空，返回一个nullptr，否则返回第一个的地址
     *
     * \param
     *
     * \return T * 返回第一个地址
     */
    inline const T &first()
    {
        QMutexLocker lk(&mutex);
        if (myList->isEmpty())
            return nullptr;
        return myList->first();
    }
    inline T takeByLock()
    {
        QMutexLocker lk(&mutex);
        if (myList->isEmpty())
            return nullptr;
        return myList->takeFirst();
    }
    /*!
     * \brief count 链表的总个数
     *
     * \return int 链表的总个数
     */
    inline int count()
    {
        QMutexLocker lk(&mutex);
        return myList->count();
    }

    DThreadList<T> &operator=(const DThreadList<T> &l)
    {
        QMutexLocker lk(&mutex);
        *myList = *l.myList;
        return *this;
    }
    /*!
     * \brief 获取实例所在位置
     *
     * \return int 返回实例所在位置
     */
    int indexOf(const T &t, int from = 0)
    {
        QMutexLocker lk(&mutex);
        return myList->indexOf(t, from);
    }
    void lock()
    {
        mutex.lock();
    }
    void unlock()
    {
        mutex.unlock();
    }

private:
    QList<T> *myList;   // 当前的QList
    QMutex mutex;   // 当前的锁
};
/*!
 * \class DThreadMap 线程安全的Map
 *
 * \brief 对QMap进行了封装，加锁对每一个QMap的操作
 */
template<class DKey, class DValue>

class DThreadMap
{
public:
    DThreadMap<DKey, DValue>()
        : myMap()
    {
    }
    /*!
     * \brief insert 插入一个模板类型到map
     *
     * \param Key 模板类引用key
     *
     * \param Value 模板类引用value
     *
     * \return
     */
    inline void insert(const DKey &key, const DValue &value)
    {
        QMutexLocker lk(&mutex);
        myMap.insert(key, value);
    }
    /*!
     * \brief remove 从map中移除所有的模板类型
     *
     * \param Key 模板类引用key
     *
     * \return
     */
    inline void remove(const DKey &key)
    {
        QMutexLocker lk(&mutex);
        myMap.remove(key);
    }
    /*!
     * \brief contains map中是否包含key对应的键值对
     *
     * Key 模板类引用key
     *
     * \return bool 是否包含key对应的键值对
     */
    inline DValue value(const DKey &key)
    {
        QMutexLocker lk(&mutex);
        return myMap.value(key);
    }
    /*!
     * \brief contains map中是否包含key对应的键值对
     *
     * Key 模板类引用key
     *
     * \return bool 是否包含key对应的键值对
     */
    inline bool contains(const DKey &key)
    {
        QMutexLocker lk(&mutex);
        return myMap.contains(key);
    }
    /*!
     * \brief begin 当前map的开始迭代器
     *
     * \param null
     *
     * \return QMap<Key, Value>::iterator 当前map的开始迭代器
     */
    inline typename QMap<DKey, DValue>::iterator begin()
    {
        QMutexLocker lk(&mutex);
        return myMap.begin();
    }
    /*!
     * \brief begin 当前map的结束迭代器
     *
     * \param null
     *
     * \return QMap<Key, Value>::iterator 当前map的结束迭代器
     */
    inline typename QMap<DKey, DValue>::iterator end()
    {
        QMutexLocker lk(&mutex);
        return myMap.end();
    }
    /*!
     * \brief erase 去掉map中当前的迭代器
     *
     * \param QMap<Key, Value>::iterator map的迭代器
     *
     * \return QMap<Key, Value>::iterator 当前迭代器的下一个迭代器
     */
    inline typename QMap<DKey, DValue>::iterator erase(typename QMap<DKey, DValue>::iterator it)
    {
        QMutexLocker lk(&mutex);
        return myMap.erase(it);
    }
    /*!
     * \brief count map的总个数
     *
     * \return int map的总个数
     */
    inline int count()
    {
        QMutexLocker lk(&mutex);
        return myMap.count();
    }
    /*!
     * \brief clear 清理整个map
     *
     * \return
     */
    inline void clear()
    {
        QMutexLocker lk(&mutex);
        return myMap.clear();
    }

    inline QList<DKey> keys()
    {
        QMutexLocker lk(&mutex);
        return myMap.keys();
    }

    inline QMap<DKey, DValue> map() const
    {
        QMutexLocker lk(&mutex);
        return myMap;
    }

private:
    QMap<DKey, DValue> myMap;   // 当前的QMap
    QMutex mutable mutex;   // 当前的锁
};
}
#endif   // THREADCONTAINER_H
