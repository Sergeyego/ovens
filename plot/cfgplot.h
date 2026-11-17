#ifndef CFGPLOT_H
#define CFGPLOT_H

#include <QWidget>
#include <QTime>
#include <QPen>
#include "qwt/qwt_plot.h"
#include "qwt/qwt_scale_draw.h"
#include "qwt/qwt_plot_grid.h"
#include "qwt/qwt_plot_picker.h"
#include "qwt/qwt_plot_canvas.h"
#include "qwt/qwt_plot_textlabel.h"
#include "qwt/qwt_text.h"
#include "qwt/qwt_scale_engine.h"
#include "qwt/qwt_scale_div.h"

namespace Ui {
class CfgPlot;
}

class TimeScaleEngine : public QwtLinearScaleEngine
{
public:
    explicit TimeScaleEngine(int interval = 1,
                             const QDateTime &baseTime = QDateTime::currentDateTime());

    void setInterval(int sec);
    void setBaseTime(const QDateTime &time);
    int  interval() const;
    QDateTime baseTime() const;

    QwtScaleDiv divideScale(double x1, double x2,
                            int maxMajorSteps, int maxMinorSteps,
                            double stepSize = 0.0) const override;

private:
    int m_interval;
    QDateTime m_baseTime;
};

class TimeScaleDraw: public QwtScaleDraw
{
public:
    TimeScaleDraw();
    virtual QwtText label(double v) const;
    void setInterval(int sec);
    void setBaseTime(QDateTime time);
    int getInterval();
    QDateTime getBaseTime();
private:
    QDateTime baseTime;
    int interval;
};

class TimePicker: public QwtPlotPicker
{
public:
   TimePicker(int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QwtPlotCanvas *canvas );
   QwtText trackerTextF(const QPointF &pos) const;
   void setInterval(int sec);
   void setBaseTime(QDateTime time);
private:
    QDateTime baseTime;
    int interval;

} ;

class Plot : public QwtPlot{
    Q_OBJECT
public:
    Plot(QWidget *parent = 0);
    void setInterval(int sec);
    void setbaseTime(QDateTime time);
    int getInterval();
    QDateTime getBaseTime();
private:
    TimePicker *timePicker;
    TimeScaleDraw *timeScaleDraw;
    TimeScaleEngine *timeScaleEngine;
public slots:
    void setXmax(int value);
};

class CfgPlot : public QWidget
{
    Q_OBJECT
public:
    explicit CfgPlot(QwtPlot *plot, QWidget *parent = 0);
    ~CfgPlot();
private:
    Ui::CfgPlot *ui;
    QwtPlot *qwtPlot;
public slots:
    void setNx(int value);
    void setNy(int value);
    void setYmax(int value);
    void setYmin(int value);
};

#endif // CFGPLOT_H
