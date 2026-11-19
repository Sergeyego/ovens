#include "cfgplot.h"
#include "ui_cfgplot.h"

CfgPlot::CfgPlot(QwtPlot *plot, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CfgPlot),
    qwtPlot(plot)
{
    ui->setupUi(this);
    qwtPlot->setAxisScale(QwtPlot::yLeft,ui->spinBox_Ymin->value(),ui->spinBox_Ymax->value());
    qwtPlot->setAxisMaxMajor(QwtPlot::xBottom,ui->spinBox_Nx->value());
    qwtPlot->setAxisMaxMajor(QwtPlot::yLeft,ui->spinBox_Ny->value());

    connect(ui->spinBox_Nx,SIGNAL(valueChanged(int)),this,SLOT(setNx(int)));
    connect(ui->spinBox_Ny,SIGNAL(valueChanged(int)),this,SLOT(setNy(int)));
    connect(ui->spinBox_Ymax,SIGNAL(valueChanged(int)),this,SLOT(setYmax(int)));
    connect(ui->spinBox_Ymin,SIGNAL(valueChanged(int)),this,SLOT(setYmin(int)));
}

CfgPlot::~CfgPlot()
{
    delete ui;
}

void CfgPlot::setNx(int value)
{
    qwtPlot->setAxisMaxMajor(QwtPlot::xBottom,value);
    qwtPlot->replot();
}

void CfgPlot::setNy(int value)
{
    qwtPlot->setAxisMaxMajor(QwtPlot::yLeft,value);
    qwtPlot->replot();
}

void CfgPlot::setYmax(int value)
{
    qwtPlot->setAxisScale(QwtPlot::yLeft,ui->spinBox_Ymin->value(),value);
    qwtPlot->replot();
}

void CfgPlot::setYmin(int value)
{
    qwtPlot->setAxisScale(QwtPlot::yLeft,value,ui->spinBox_Ymax->value());
    qwtPlot->replot();
}

TimeScaleEngine::TimeScaleEngine(int interval, const QDateTime &baseTime)
    : m_interval(std::max(1, interval))
    , m_baseTime(baseTime)
{
}

void TimeScaleEngine::setInterval(int sec)        { m_interval = std::max(1, sec); }
void TimeScaleEngine::setBaseTime(const QDateTime &t) { m_baseTime = t; }
int  TimeScaleEngine::interval() const             { return m_interval; }
QDateTime TimeScaleEngine::baseTime() const        { return m_baseTime; }

QwtScaleDiv TimeScaleEngine::divideScale(double x1, double x2,
                                         int maxMajorSteps, int maxMinorSteps,
                                         double stepSize) const
{
    if (stepSize > 0.0) {
        return QwtLinearScaleEngine::divideScale(x1, x2, maxMajorSteps, maxMinorSteps, stepSize);
    }

    const double range = x2 - x1;
    if (range <= 0.0) {
        return QwtScaleDiv(x1, x2);
    }

    // «Красивые» шаги в секундах
    static const double candidates[] = {
        1, 2, 5, 10, 15, 30,
        60, 120, 300, 600, 900, 1800,
        3600, 7200, 10800, 21600, 43200, 86400, 172800
    };
    const int n = sizeof(candidates) / sizeof(candidates[0]);

    const double ideal_sec = (range * m_interval) / std::max(1, maxMajorSteps);
    double step_sec = candidates[n - 1];
    for (int i = 0; i < n; ++i) {
        if (candidates[i] >= ideal_sec) { step_sec = candidates[i]; break; }
    }
    // Кратность interval, чтобы v был целым
    step_sec = std::ceil(step_sec / m_interval) * m_interval;

    // Переход в epoch-секунды: теперь x=0 — это m_baseTime, а тики
    // выровнены по кратности step_sec в абсолютном времени
    //const double base_epoch = static_cast<double>(m_baseTime.toMSecsSinceEpoch()) / 1000.0;
    const double base_epoch = static_cast<double>(m_baseTime.toSecsSinceEpoch());
    const double t1 = base_epoch + x1 * m_interval;
    const double t2 = base_epoch + x2 * m_interval;

    // Основные тики — кратные step_sec в epoch-времени
    QList<double> majorTicks;
    for (double t = std::ceil(t1 / step_sec) * step_sec;
         t <= t2 + step_sec * 0.5;
         t += step_sec)
    {
        majorTicks << (t - base_epoch) / m_interval;
    }

    // Минорные тики — тоже выровнены по epoch-времени
    QList<double> minorTicks;
    if (maxMinorSteps > 0 && !majorTicks.isEmpty()) {
        const double minor_sec = step_sec / (maxMinorSteps + 1);
        for (double t = std::ceil(t1 / minor_sec) * minor_sec;
             t <= t2 + minor_sec * 0.5;
             t += minor_sec)
        {
            const double v = (t - base_epoch) / m_interval;
            bool isMajor = false;
            for (double m : majorTicks) {
                if (qFuzzyCompare(v + 1.0, m + 1.0)) { isMajor = true; break; }
            }
            if (!isMajor) minorTicks << v;
        }
    }

    return QwtScaleDiv(x1, x2, minorTicks, QList<double>(), majorTicks);
}


TimeScaleDraw::TimeScaleDraw()
{
    baseTime=QDateTime::currentDateTime();
    interval=1;
}

QwtText TimeScaleDraw::label(double v) const
{
    QDateTime t = baseTime.addSecs(qRound64(v * interval));

    // Шаг между основными метками (в секундах)
    double step_sec = 0;
    const QList<double> majors = scaleDiv().ticks(QwtScaleDiv::MajorTick);
    if (majors.size() >= 2) {
        step_sec = qAbs(majors[1] - majors[0]) * interval;
    }

    if (step_sec >= 60) {
        // Метки не чаще раза в минуту — округляем к ближайшей минуте,
        // чтобы не появлялось hh:mm:59
        const int sec = t.time().second();
        t = t.addSecs(sec >= 30 ? 60 - sec : -sec);

        return t.toString("hh:mm");
    }

    return t.toString("hh:mm:ss");
}

void TimeScaleDraw::setInterval(int sec)
{
    interval=sec;
}

void TimeScaleDraw::setBaseTime(QDateTime time)
{
    baseTime=time;
}

int TimeScaleDraw::getInterval()
{
    return interval;
}

QDateTime TimeScaleDraw::getBaseTime()
{
    return baseTime;
}


TimePicker::TimePicker(int xAxis, int yAxis, QwtPicker::RubberBand rubberBand, QwtPicker::DisplayMode trackerMode, QwtPlotCanvas *canvas):
    QwtPlotPicker(xAxis,yAxis,rubberBand,trackerMode,canvas)
{
    baseTime=QDateTime::currentDateTime();
    interval=1;
}

QwtText TimePicker::trackerTextF(const QPointF &pos) const
{
    QDateTime t = baseTime.addSecs(qRound64(pos.x() * interval));

    if (interval >= 60) {
        const int sec = t.time().second();
        t = t.addSecs(sec >= 30 ? 60 - sec : -sec);
    }

    QString text = t.toString("dd.MM.yy hh:mm:ss");
    text += ", " + QString::number(pos.y(), 'f', 0);
    return QwtText(text);
}

void TimePicker::setInterval(int sec)
{
    interval=sec;
}

void TimePicker::setBaseTime(QDateTime time)
{
    baseTime=time;
}

Plot::Plot(QWidget *parent) : QwtPlot(parent)
{
    setAutoReplot(false);
    setPalette(Qt::gray);
    setAxisTitle(QwtPlot::xBottom,QString::fromLocal8Bit("t, мин."));
    setAxisTitle(QwtPlot::yLeft,QString::fromLocal8Bit("T, град."));
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajorPen(QPen(Qt::black,0,Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray,0,Qt::DotLine));
    grid->attach(this);
    setAxisScale(QwtPlot::xBottom,0,0);

    timeScaleDraw = new TimeScaleDraw();
    setAxisScaleDraw(QwtPlot::xBottom, timeScaleDraw);

    timeScaleEngine = new TimeScaleEngine(1, QDateTime::currentDateTime());
    setAxisScaleEngine(QwtPlot::xBottom, timeScaleEngine);

    QwtPlotCanvas *canvas = new QwtPlotCanvas(this);

    timePicker = new TimePicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, canvas);
    timePicker->setTrackerPen(QColor(Qt::black));

    setCanvas(canvas);

}

void Plot::setInterval(int sec)
{
    timePicker->setInterval(sec);
    timeScaleDraw->setInterval(sec);
    timeScaleEngine->setInterval(sec);
}

void Plot::setbaseTime(QDateTime time)
{
    // Убираем миллисекунды, иначе на оси появляется :59:59
    QTime t = time.time();
    time.setTime(QTime(t.hour(), t.minute(), t.second(), 0));

    timePicker->setBaseTime(time);
    timeScaleDraw->setBaseTime(time);
    timeScaleEngine->setBaseTime(time);
}

int Plot::getInterval()
{
    return timeScaleDraw->getInterval();
}

QDateTime Plot::getBaseTime()
{
    return timeScaleDraw->getBaseTime();
}

void Plot::setXmax(int value)
{
    setAxisScale(QwtPlot::xBottom,0,value);
    replot();
}
