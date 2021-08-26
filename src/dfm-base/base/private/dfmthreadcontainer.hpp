/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#ifndef DTHREADCONTAINER_H
#define DTHREADCONTAINER_H

#include <QList>
#include <QMap>
#include <QMutex>

template<class T>

class DThreadList {
public:
    DThreadList<T>():m_list(new QList<T>){}
    ~DThreadList() {
        m_list->clear();
        delete m_list;
        m_list = nullptr;
    }
    /**
     * @brief push_back 从链表的最后插入模板类型
     * @param T 模板类引用
     * @return
     */
    inline void push_back(const T &t){
        QMutexLocker lk(&m_mutex);
        m_list->push_back(t);
    }
    /**
     * @brief insert 插入一个模板类型到链表
     * @param T 模板类引用
     * @return
     */
    inline void insert(const T &t){
        QMutexLocker lk(&m_mutex);
        m_list->insert(t);
    }
    /**
     * @brief setList 设置一个链表
     * @param QList<T> 模板类链表
     * @return
     */
    inline void setList(const QList<T> &t){
        QMutexLocker lk(&m_mutex);
        *m_list = t;
    }
    /**
     * @brief setList 设置一个链表
     * @param QList<T> 模板类链表
     * @return const QList<T> &获取链表
     */
    inline const QList<T> &list(){
        QMutexLocker lk(&m_mutex);
        return *m_list;
    }
    /**
     * @brief removeAll 从链表中移除所有的模板类型
     * @param T 模板类引用
     * @return
     */
    inline void removeAll(const T &t)
    {
        QMutexLocker lk(&m_mutex);
        m_list->removeAll(t);
    }
    /**
     * @brief removeOne 从链表中移除一个的模板类型
     * @param T 模板类引用
     * @return
     */
    inline void removeOne(const T &t)
    {
        QMutexLocker lk(&m_mutex);
        m_list->removeOne(t);
    }

    /**
     * @brief contains 链表中是否包含模板
     * @param T 模板类引用
     * @return bool 是否包含模板
     */
    inline bool contains(const T &t)
    {
        QMutexLocker lk(&m_mutex);
        return m_list->contains(t);
    }
    /**
     * @brief clear 链表中是否包含模板
     * @param T 模板类引用
     * @return bool 是否包含模板
     */
    inline void clear()
    {
        QMutexLocker lk(&m_mutex);
        return m_list->clear();
    }
    /**
     * @brief append 增加一个元素
     * @param T 模板类引用
     * @return bool 是否包含模板
     */
    inline void append(const T &t)
    {
        QMutexLocker lk(&m_mutex);
        return m_list->append(t);
    }
//    /**
//     * @brief push_back 增加一个元素到最后
//     * @param T 模板类引用
//     * @return
//     */
//    inline void push_back(const T &t)
//    {
//        return append(t);
//    }
    /**
     * @brief size 链表的总长度
     * @return int 链表的总长度
     */
    inline int size() const
    {
        return m_list->size();
    }
    /**
     * @brief at 链表的总长度
     * @param int 当前第i个位置
     * @return T& 链表的总长度
     */
    inline const T& at(int i) const
    {
        return m_list->at(i);
    }
    /**
     * @brief begin 当前链表的开始迭代器
     * @param null
     * @return QList<T>::iterator 当前链表的开始迭代器
     */
    inline typename QList<T>::iterator begin() {
        QMutexLocker lk(&m_mutex);
        return m_list->begin();
    }
    /**
     * @brief begin 当前链表的结束迭代器
     * @param null
     * @return QList<T>::iterator 当前链表的结束迭代器
     */
    inline typename QList<T>::iterator end() {
        QMutexLocker lk(&m_mutex);
        return m_list->end();
    }
    /**
     * @brief erase 去掉链表中当前的迭代器
     * @param QList<T>::iterator 链表的迭代器
     * @return QList<T>::iterator 当前迭代器的下一个迭代器
     */
    inline typename QList<T>::iterator erase(typename QList<T>::iterator it) {
        QMutexLocker lk(&m_mutex);
        return m_list->erase(it);
    }
    /**
     * @brief first 如果链表为空，返回一个nullptr，否则返回第一个的地址
     * @param
     * @return T * 返回第一个地址
     */
    inline T *first() {
        QMutexLocker lk(&m_mutex);
        if (m_list->isEmpty())
            return nullptr;
        return &(m_list->first());
    }
    /**
     * @brief count 链表的总个数
     * @return int 链表的总个数
     */
    inline int count() {
        QMutexLocker lk(&m_mutex);
        return m_list->count();
    }

    DThreadList<T> &operator=(const DThreadList<T> &l) {
        QMutexLocker lk(&m_mutex);
        *m_list = *l.m_list;
    }

private:
    QList<T> *m_list;
    QMutex m_mutex;
};

template<class DKey, class DValue>

class DThreadMap {
public:
    DThreadMap<DKey, DValue>():m_map(new QMap<DKey,DValue>)
    {
    }
    ~DThreadMap() {
        if (m_map) {
            {
                QMutexLocker lk(&m_mutex);
                m_map->clear();
            }
            delete m_map;
            m_map = nullptr;
        }
    }
    /**
     * @brief insert 插入一个模板类型到map
     * @param Key 模板类引用key
     * @param Value 模板类引用value
     * @return
     */
    inline void insert(const DKey &key, const DValue &value){
        QMutexLocker lk(&m_mutex);
        m_map->insert(key, value);
    }
    /**
     * @brief remove 从map中移除所有的模板类型
     * @param Key 模板类引用key
     * @return
     */
    inline void remove(const DKey &key){
        QMutexLocker lk(&m_mutex);
        m_map->remove(key);
    }
    /**
     * @brief contains map中是否包含key对应的键值对
     * Key 模板类引用key
     * @return bool 是否包含key对应的键值对
     */
    inline DValue value(const DKey &key){
        QMutexLocker lk(&m_mutex);
        return m_map->value(key);
    }
    /**
     * @brief contains map中是否包含key对应的键值对
     * Key 模板类引用key
     * @return bool 是否包含key对应的键值对
     */
    inline bool contains(const DKey &key){
        QMutexLocker lk(&m_mutex);
        return m_map->contains(key);
    }
    /**
     * @brief begin 当前map的开始迭代器
     * @param null
     * @return QMap<Key, Value>::iterator 当前map的开始迭代器
     */
    inline typename QMap<DKey, DValue>::iterator begin() {
        QMutexLocker lk(&m_mutex);
        return m_map->begin();
    }
    /**
     * @brief begin 当前map的结束迭代器
     * @param null
     * @return QMap<Key, Value>::iterator 当前map的结束迭代器
     */
    inline typename QMap<DKey, DValue>::iterator end() {
        QMutexLocker lk(&m_mutex);
        return m_map->end();
    }
    /**
     * @brief erase 去掉map中当前的迭代器
     * @param QMap<Key, Value>::iterator map的迭代器
     * @return QMap<Key, Value>::iterator 当前迭代器的下一个迭代器
     */
    inline typename QMap<DKey, DValue>::iterator erase(typename QMap<DKey, DValue>::iterator it) {
        QMutexLocker lk(&m_mutex);
        return m_map->erase(it);
    }
    /**
     * @brief count map的总个数
     * @return int map的总个数
     */
    inline int count() {
        QMutexLocker lk(&m_mutex);
        return m_map->count();
    }

private:
    QMap<DKey, DValue> *m_map;
    QMutex m_mutex;
};

#endif // DTHREADCONTAINER_H
