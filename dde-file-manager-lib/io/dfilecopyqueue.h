#ifndef DFILECOPYQUEUE_H
#define DFILECOPYQUEUE_H

#include <QQueue>
#include <QMutex>

template <class T>
class DFileCopyQueue : public QQueue<T>
{
public:
    // compiler-generated special member functions are fine!
    inline void swap(QQueue<T> &other) Q_DECL_NOTHROW {
        QMutexLocker lock(&m_mutex);
        QQueue<T>::swap(other);
    } // prevent QList<->QQueue swaps
#ifndef Q_QDOC
    // bring in QList::swap(int, int). We cannot say using QList<T>::swap,
    // because we don't want to make swap(QList&) available.
    inline void swap(int i, int j) {
        QMutexLocker lock(&m_mutex);
        QQueue<T>::swap(i, j);
    }
#endif
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
    inline const T &head() const {
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
