#include "srcchannel.h"

SrcChannel::SrcChannel(Plot *p, QWidget *parent) :
    QWidget(parent), plot(p)
{
    this->setLayout(new QVBoxLayout(this));
}

SrcChannel::~SrcChannel()
{
}

void SrcChannel::addData(double ust, double val, double t)
{
    if (srcUst) srcUst->addData(t,ust);
    if (srcVal) srcVal->addData(t,val);
}

void SrcChannel::clear()
{
    if (srcUst) srcUst->clear();
    if (srcVal) srcVal->clear();
    if (srcPwr) srcPwr->clear();
}

void SrcChannel::mkWidget(QString name, QColor cv, QColor cu)
{
    groupBoxCh = new QGroupBox(name,this);
    this->layout()->addWidget(groupBoxCh);
    srcVal = new SrcPlot(plot,tr("Температура"),cv,true,this);
    srcUst = new SrcPlot(plot,tr("Уставка"),cu,true,this);
    srcPwr = new SrcPlot(plot,tr("Мощность"),Qt::black,false,this);
    groupBoxCh->setLayout(new QVBoxLayout);
    groupBoxCh->layout()->addWidget(srcVal);
    groupBoxCh->layout()->addWidget(srcUst);
    groupBoxCh->layout()->addWidget(srcPwr);
}

double SrcChannel::initFromJson(const QJsonObject &obj)
{
    QString nam = obj.value("nam").toString();
    QStringList c_ust = obj.value("color_ust").toString().split(":");
    QStringList c_val = obj.value("color_val").toString().split(":");
    bool is_main = obj.value("is_main").toBool();

    QColor cu(0, 0, 0);
    QColor cv(0, 0, 0);

    if (c_ust.size() == 3) {
        cu = QColor(c_ust.at(0).toInt(), c_ust.at(1).toInt(), c_ust.at(2).toInt());
    }
    if (c_val.size() == 3) {
        cv = QColor(c_val.at(0).toInt(), c_val.at(1).toInt(), c_val.at(2).toInt());
    }

    if (is_main) nam += tr(" <главный>");
    mkWidget(nam, cv, cu);

    QJsonArray dataArr = obj.value("data").toArray();
    int interval = plot->getInterval();
    double x = 0.0;
    QDateTime tm = plot->getBaseTime();
    int delta;

    for (const QJsonValue &val : dataArr) {
        QJsonObject row = val.toObject();

        // Конвертируем UTC-время из БД в локальное для правильного отображения
        QDateTime dt = QDateTime::fromString(row.value("dat_time").toString(), Qt::ISODate);
        dt = dt.toLocalTime();

        delta = tm.secsTo(dt);
        tm = dt;
        x += (double)delta / interval;

        double ust = row.value("ust").toDouble();
        double v   = row.value("val").toDouble();
        double pwr = row.value("pwr").toDouble() * 100.0;

        if (srcUst) srcUst->addData(x, ust);
        if (srcVal) srcVal->addData(x, v);
        if (srcPwr) srcPwr->addData(x, pwr);
    }

    if (srcUst) srcUst->plotData();
    if (srcVal) srcVal->plotData();
    if (srcPwr) srcPwr->plotData();

    return x;
}
