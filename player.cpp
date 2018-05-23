#include "player.h"

Player::Player()
{

}

QString Player::getUsername() const
{
    return username;
}

void Player::setUsername(const QString &value)
{
    username = value;
}
