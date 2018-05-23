#ifndef GRAPHICBUTTON_H
#define GRAPHICBUTTON_H

#include <QPushButton>
#include <QPixmap>
#include <QIcon>

class GraphicButton : public QPushButton
{
public:
    GraphicButton(QString text, QString backgroundFile, int width, int height, QColor textColor, QString fontFamily);
};

#endif // GRAPHICBUTTON_H
