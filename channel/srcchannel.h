#ifndef SRCCHANNEL_H
#define SRCCHANNEL_H

#include <QWidget>
#include "plot/srcplot.h"
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonArray>

class SrcChannel : public QWidget
{
    Q_OBJECT
public:
    SrcChannel(Plot *p, QWidget *parent = 0);
    ~SrcChannel();

    // Инициализация из JSON сервера. Возвращает максимальное X для оси.
    double initFromJson(const QJsonObject &obj);

    void addData(double ust, double val, double t);
    void clear();

private:
    SrcPlot *srcUst = nullptr;
    SrcPlot *srcVal = nullptr;
    SrcPlot *srcPwr = nullptr;
    Plot *plot;
    QGroupBox *groupBoxCh;

    void mkWidget(QString name, QColor cv, QColor cu);
};

#endif // SRCCHANNEL_H
