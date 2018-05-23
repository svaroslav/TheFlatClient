#include "graphicbutton.h"

GraphicButton::GraphicButton(QString text, QString backgroundFile, int width, int height, QColor textColor, QString fontFamily)
{
    this->setGeometry(0, 0, width, height);
    this->setText(text);
    this->setStyleSheet("background-image:url(" + backgroundFile + ");"
                        "font-weight: bold;"
                        "font-size: " + QString::number(height / 2) + "px;"
                        "font-family: " + fontFamily + ";"
                        "color: rgb(" + QString::number(textColor.red()) + ","
                        "" + QString::number(textColor.green()) + ","
                        "" + QString::number(textColor.blue()) + ");");
}
