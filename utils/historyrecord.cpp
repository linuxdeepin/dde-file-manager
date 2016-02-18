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
    m_currentIndex = -1;
}

/*!
 * \brief HistoryRecord::append
 * inserts value at the end of the list.
 * \param url
 */
void HistoryRecord::append(const QString &url)
{
    if(!m_records.contains(url)){
        m_records.append(url);
        m_currentIndex = m_records.indexOf(url, 0);
    }
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
    QString url = currentUrl();
    m_records.removeFirst();
    m_currentIndex = m_records.indexOf(url, 0);
}


/*!
 * \brief HistoryRecord::removeLast
 * Removes the last item in the list. Calling this function is equivalent to calling removeAt(size() - 1). The list must not be empty. If the list can be empty, call isEmpty() before calling this function.
 */
void HistoryRecord::removeLast()
{
    QString url = currentUrl();
    m_records.removeFirst();
    m_currentIndex = m_records.indexOf(url, 0);
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
    QString url = currentUrl();
    QString removeUrl = m_records.takeAt(i);
    m_currentIndex = m_records.indexOf(url, 0);
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
    QString url = currentUrl();
    QString removeUrl = m_records.takeFirst();
    m_currentIndex = m_records.indexOf(url, 0);
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
    QString url = currentUrl();
    QString removeUrl = m_records.takeLast();
    m_currentIndex = m_records.indexOf(url, 0);
    return removeUrl;
}

/*!
 * \brief HistoryRecord::move Moves the item at index position \a from to index position \a to.
 * \param from
 * \param to
 */
void HistoryRecord::move(int from, int to)
{
    QString url = currentUrl();
    m_records.move(from, to);
    m_currentIndex = m_records.indexOf(url, 0);
}

/*!
 * \brief HistoryRecord::swap  Exchange the item at index position \a i with the item at index position \a j. This function assumes that both \a i and \j are at least 0 but less than size(). To avoid failure, test that both \a i and \a j are at least 0 and less than size().
 * \param i
 * \param j
 */
void HistoryRecord::swap(int i, int j)
{
    QString url = currentUrl();
    m_records.swap(i, j);
    m_currentIndex = m_records.indexOf(url, 0);
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
    if (0 < m_currentIndex && m_currentIndex < count()){
        setCurrentIndex(m_currentIndex - 1);
        emit currentUrlChanged(currentUrl());
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
    if (0 <= m_currentIndex && m_currentIndex < endIndex){
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
