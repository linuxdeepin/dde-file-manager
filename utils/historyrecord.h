#ifndef HISTORYRECORD_H
#define HISTORYRECORD_H

#include <QObject>
#include <QList>
#include <QString>

#include "debugobejct.h"

class HistoryRecord : public DebugObejct
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(QStringList records READ records WRITE setRecords)
public:
    explicit HistoryRecord(QObject *parent = 0);
    ~HistoryRecord();

    QStringList& records();
    void setRecords(const QStringList& records);
    int currentIndex();
    void setCurrentIndex(int index);

    QString currentUrl();


    int count();
    int length();


    const QString &at(int i) const;
    QString &operator[](int i);
    const QString &operator[](int i) const;

    const QString & first() const;
    const QString & end() const;

    void clear();
    void append(const QString& url);

    bool isEmpty();
    void removeAt(int i);
    void removeFirst();
    void removeLast();
    QString takeAt(int i);
    QString takeFirst();
    QString takeLast();
    int indexOf(const QString &url, int from = 0) const;
    bool contains(const QString &url) const;

signals:
    void currentIndexChanged(int index);
    void currentUrlChanged(const QString& url);
    void frontReached();
    void endReached();

public slots:
    void back();
    void forward();
    void push(const QString& url);

private:
    QStringList m_records{};
    int m_currentIndex = 0;
};



#endif // HISTORYRECORD_H
