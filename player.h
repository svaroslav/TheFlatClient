#ifndef PLAYER_H
#define PLAYER_H

#include <QDebug>

class Player
{

public:
    Player();

    QString getUsername() const;
    void setUsername(const QString &value);

private:
    QString username;

};

#endif // PLAYER_H
