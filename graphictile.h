#ifndef GRAPHICTILE_H
#define GRAPHICTILE_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPixmap>

class GraphicTile : public QGraphicsItem
{
public:
    GraphicTile(QString filePath, int width);
    QRectF boundingRect() const;

protected:
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString filePath;
    int width;
};

#endif // GRAPHICTILE_H
