#ifndef PLOTDATA_H
#define PLOTDATA_H

#include <QWidget>
#include <QQueue>
#include <QUrl>
#include <QPointer>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include "srcchannel.h"
#include "plot/cfgplot.h"
#include "rest/restconnection.h"

class PlotData : public QWidget
{
    Q_OBJECT
public:
    explicit PlotData(Plot *p, QWidget *parent = 0);
    void clearData();

private:
    QVector<SrcChannel*> srcData;
    Plot *plot;

    // Очередь актуальных запросов (храним только последний URL)
    QQueue<QUrl> queue;
    bool isProcessing = false;
    QPointer<QNetworkReply> currentReply;

private slots:
    void processNextRequest();
    void onDataReceived();

public slots:
    void refresh(int id_owrab, bool new_format, bool ost=false);
    void refresh(QDateTime dBeg, QDateTime dEnd, int id_oven);
};

#endif // PLOTDATA_H