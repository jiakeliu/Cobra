#ifndef UPERMS_H
#define UPERMS_H

#include <QString>

class UserPermissions {
public:
    UserPermissions(QString user = ""): m_sUser(user), m_uiPerms(0) {}
    ~UserPermissions() {}

    bool operator<(const UserPermissions& rhs) const
        { return (m_sUser < rhs.m_sUser); }
    bool operator==(const UserPermissions& rhs) const
        { return (rhs.m_sUser == m_sUser); }
    bool operator==(const QString rhs) const
        { return (rhs == m_sUser); }
    bool operator==(const QString& rhs) const
        { return (rhs == m_sUser); }

    void setFlag(unsigned int flag) {
        m_uiPerms |= flag;
    }

    void toggleFlag(unsigned int flag) {
        m_uiPerms ^= flag;
    }

    void unsetFlag(unsigned int flag) {
        m_uiPerms &= ~flag;
    }

protected:
    QString m_sUser;
    unsigned int m_uiPerms;
};

#endif // UPERMS_H
