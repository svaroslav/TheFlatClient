#include "graphictile.h"

GraphicTile::GraphicTile(QString filePath, int width)
{
    this->filePath = filePath;
    this->width = width;
}

QRectF GraphicTile::boundingRect() const
{
    return QRect(0 - (width / 2), 0 - (width / 2), width, width);
}

QPainterPath GraphicTile::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void GraphicTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPixmap image(filePath);
    painter->drawPixmap(0 - (width / 2), 0 - (width / 2), width, width, image);
}
