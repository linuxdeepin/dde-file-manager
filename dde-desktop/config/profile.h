#pragma once

#include <QObject>
#include <QScopedPointer>

class ProfilePrivate;
class Profile : public QObject
{
    Q_OBJECT
public:
    explicit Profile(QObject *parent = 0);
    ~Profile();

Q_SIGNALS:

public Q_SLOTS:

private:
    QScopedPointer<ProfilePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Profile)
};

