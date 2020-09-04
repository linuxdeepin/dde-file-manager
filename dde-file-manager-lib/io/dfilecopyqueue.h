#ifndef DFILECOPYQUEUE_H
#define DFILECOPYQUEUE_H

#include <QQueue>
#include <QMutex>

template <class T>
class DFileCopyQueue : public QQueue<T>
{
public:
    inline void enqueue(const T &t) {
        QMutexLocker lock(&m_mutex);
        QQueue<T>::enqueue(t);
    }
    inline T dequeue() {
        QMutexLocker lock(&m_mutex);
        return QQueue<T>::dequeue();
    }
    inline T &head() {
        QMutexLocker lock(&m_mutex);
        return QQueue<T>::head();
    }
    inline bool isEmpty() {
        QMutexLocker lock(&m_mutex);
        return QList<T>::isEmpty();
    }
private:
    QMutex m_mutex;
};

#endif // DFILECOPYQUEUE_H
