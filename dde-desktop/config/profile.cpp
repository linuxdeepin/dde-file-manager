#include "profile.h"

class ProfilePrivate
{
public:
    ProfilePrivate(Profile *parent) : q_ptr(parent){}

    Profile *q_ptr;
    Q_DECLARE_PUBLIC(Profile)
};

Profile::Profile(QObject *parent) :
    QObject(parent), d_ptr(new ProfilePrivate(this))
{

}

Profile::~Profile()
{

}
