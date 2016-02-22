#include "historyrecord.h"

HistoryRecord::HistoryRecord(QObject *parent) : DebugObejct(parent)
{

}

HistoryRecord::~HistoryRecord()
{

}

/*!
 * \brief HistoryRecord::setCurrentIndex
 * get records
 * \param index
 */
QStringList &HistoryRecord::records()
{
    return m_records;
}

/*!
 * \brief HistoryRecord::setCurrentIndex
 * set reocords
 * \param index
 */
void HistoryRecord::setRecords(const QStringList &records)
{
    m_records = records;
}

/*!
 * \brief HistoryRecord::setCurrentIndex
 * get current index
 * \param index
 */
int HistoryRecord::currentIndex()
{
    return m_currentIndex;
}

/*!
 * \brief HistoryRecord::setCurrentIndex
 * set current index
 * \param index
 */
void HistoryRecord::setCurrentIndex(int index)
{
    m_currentIndex = index;
    emit currentIndexChanged(m_currentIndex);
}


/*!
 * \brief HistoryRecord::currentUrl
 * return current url of history records
 * \return
 */
QString HistoryRecord::currentUrl()
{
    return m_records.at(m_currentIndex);
}

/*!
 * \brief HistoryRecord::count
 * return number of records
 * \return
 */
int HistoryRecord::count()
{
    return m_records.count();
}

/*!
 * \brief HistoryRecord::length
 * same as \b count
 * \return
 */
int HistoryRecord::length()
{
    return m_records.count();
}

/*!
 * \brief HistoryRecord::at
 * Returns the item at index position \a i in the list.\a i must be a valid index position in the list (i.e., 0 <= \a i < size()).
 * \param i
 * \return
 */
const QString &HistoryRecord::at(int i) const
{
    return m_records.at(i);
}


/*!
 * \brief HistoryRecord::operator []
 * Returns the item at index position i as a modifiable reference. \a i must be a valid index position in the list (i.e., 0 <= i < size()).
If this function is called on a list that is currently being shared, it will trigger a copy of all elements. Otherwise, this function runs in constant time.
If you do not want to modify the list you should use QList::at().
 * \param i
 * \return
 */
QString &HistoryRecord::operator[](int i)
{
    return m_records[i];
}

/*!
 * \brief HistoryRecord::operator []
 * This is an overloaded function.
 * \param i
 * \return
 */
const QString &HistoryRecord::operator[](int i) const
{
    return m_records[i];
}

/*!
 * \brief HistoryRecord::end
 * Returns a reference to the first item in the list. The list must not be empty. If the list can be empty, call isEmpty() before calling this function.
 * \return
 */
const QString &HistoryRecord::first() const
{
    return m_records.first();
}

/*!
 * \brief HistoryRecord::end
 * Returns a reference to the last item in the list. The list must not be empty. If the list can be empty, call isEmpty() before calling this function.
 * \return
 */
const QString &HistoryRecord::end() const
{
    return m_records.at(m_records.count() - 1);
}

/*!
 * \brief HistoryRecord::clear
 * Removes all records from the list.
 */
void HistoryRecord::clear()
{
    m_records.clear();
    m_currentIndex = 0;
}

/*!
 * \brief HistoryRecord::append
 * inserts value at the end of the list.
 * \param url
 */
void HistoryRecord::append(const QString &url)
{
    m_records.append(url);
    setCurrentIndex(count() - 1);
}


/*!
 * \brief HistoryRecord::isEmpty
 * Returns true if the list contains no items; otherwise returns false.
 * \return
 */
bool HistoryRecord::isEmpty()
{
    return m_records.isEmpty();
}

/*!
 * \brief HistoryRecord::removeAt
 * Removes the item at index position \a i. \a i must be a valid index position in the list (i.e., 0 <= \a i < size()).
 * \param i
 */
void HistoryRecord::removeAt(int i)
{
    QString url = currentUrl();
    m_records.removeAt(i);
    m_currentIndex = m_records.indexOf(url, 0);
}

/*!
 * \brief HistoryRecord::removeFirst
 * Removes the first item in the list. Calling this function is equivalent to calling removeAt(0). The list must not be empty. If the list can be empty, call isEmpty() before calling this function.
 */
void HistoryRecord::removeFirst()
{
    int oldCount = count();
    m_records.removeFirst();
    if (m_currentIndex >= 1 && m_currentIndex < oldCount){
        m_currentIndex -= 1;
    }
}


/*!
 * \brief HistoryRecord::removeLast
 * Removes the last item in the list. Calling this function is equivalent to calling removeAt(size() - 1). The list must not be empty. If the list can be empty, call isEmpty() before calling this function.
 */
void HistoryRecord::removeLast()
{
    int oldCount = count();
    m_records.removeLast();
    if (m_currentIndex >= 1 && m_currentIndex < oldCount){
        m_currentIndex -= 1;
    }
}

/*!
 * \brief HistoryRecord::takeAt
 * Removes the item at index position \a i and returns it. \a i must be a valid index position in the list (i.e., 0 <= i < size()).

If you don't use the return value, removeAt() is more efficient.
 * \param i
 * \return
 */
QString HistoryRecord::takeAt(int i)
{
    int oldCount = count();
    QString removeUrl = m_records.takeAt(i);
    if (m_currentIndex >= 1 && m_currentIndex < oldCount){
        m_currentIndex -= 1;
    }
    return removeUrl;
}

/*!
 * \brief HistoryRecord::takeFirst
 * Removes the first item in the list and returns it. This is the same as takeAt(0). This function assumes the list is not empty. To avoid failure, call isEmpty() before calling this function.

If this list is not shared, this operation takes constant time.

If you don't use the return value, removeFirst() is more efficient.
 * \return
 */
QString HistoryRecord::takeFirst()
{
    QString removeUrl = m_records.takeFirst();
    int oldCount = count();
    if (m_currentIndex >= 1 && m_currentIndex < oldCount){
        m_currentIndex -= 1;
    }
    return removeUrl;
}

/*!
 * \brief HistoryRecord::takeLast Removes the last item in the list and returns it. This is the same as takeAt(size() - 1). This function assumes the list is not empty. To avoid failure, call isEmpty() before calling this function.

If this list is not shared, this operation takes constant time.

If you don't use the return value, removeLast() is more efficient.
 * \return
 */
QString HistoryRecord::takeLast()
{
    QString removeUrl = m_records.takeLast();
    int oldCount = count();
    if (m_currentIndex >= 1 && m_currentIndex < oldCount){
        m_currentIndex -= 1;
    }
    return removeUrl;
}

/*!
 * \brief HistoryRecord::indexOf Returns the index position of the first occurrence of value in the list, searching forward from index position from. Returns -1 if no item matched.
 * \param url
 * \param from
 * \return
 */
int HistoryRecord::indexOf(const QString &url, int from) const
{
    return m_records.indexOf(url, from);
}

/*!
 * \brief HistoryRecord::contains Returns true if the list contains an occurrence of value; otherwise returns false.
 * \param url
 * \return
 */
bool HistoryRecord::contains(const QString &url) const
{
    return m_records.contains(url);
}

/*!
 * \brief HistoryRecord::back last record action
 */
void HistoryRecord::back()
{
    if (0 < m_currentIndex && m_currentIndex <= count()){
        qDebug() << m_currentIndex;
        setCurrentIndex(m_currentIndex - 1);
        emit currentUrlChanged(currentUrl());
        qDebug() << m_currentIndex;
    }else if (m_currentIndex == 0){
        emit frontReached();
    }
}

/*!
 * \brief HistoryRecord::forward next record action
 */
void HistoryRecord::forward()
{
    int endIndex = count() - 1;
    qDebug() << m_currentIndex << endIndex;
    if (0 <= m_currentIndex && m_currentIndex < (endIndex - 1)){
        setCurrentIndex(m_currentIndex + 1);
        emit currentUrlChanged(currentUrl());
    }else if (m_currentIndex == endIndex){
        emit endReached();
    }
}

/*!
 * \brief HistoryRecord::push same as \b append
 * \param url
 */
void HistoryRecord::push(const QString &url)
{
    append(url);
}
