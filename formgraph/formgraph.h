#ifndef FORMGRAPH_H
#define FORMGRAPH_H

#include <QWidget>
#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QPainter>
#include <QModelIndex>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QShortcut>
#include <QPointer>
#include "plot/cfgplot.h"
#include "rest/restrotablemodel.h"
#include "rest/restconnection.h"
#include "rest/resterror.h"
#include "channel/plotdata.h"
#include "qwt/qwt_scale_map.h"
#include "qwt/qwt_plot_renderer.h"

namespace Ui {
class FormGraph;
}

class FormGraph : public QWidget
{
    Q_OBJECT

public:
    explicit FormGraph(QWidget *parent = nullptr);
    ~FormGraph();

private:
    Ui::FormGraph *ui;
    Plot *plot;
    CfgPlot *cfgPlot;
    PlotData *plotData;
    RestRoTableModel *modelDry;
    RestRoTableModel *modelOven;

    // Асинхронные датчики
    QPointer<QNetworkReply> m_sensorReply;
    QString m_pendingBaseTitle;
    int     m_pendingIdOwen = -1;
    QDate   m_pendingDate;
    bool    m_loadingData = false;
    int  m_savedOvenId = -1;
    bool m_restoreOvenRow = false;

    void loadSettings();
    void saveSettings();
    QString secToStr(qint64 sec);
    void requestSensorInfo(int id_owen, QDate date, const QString &baseTitle);
    void print(QPrinter *p);

private slots:
    void updDry();
    void updDryFinished();
    void updGraph();
    void setCurrentProc(QModelIndex index);
    void setCurrentOven(QModelIndex index);
    void updOven();
    void updOvenFinished();
    void printGraph();
    void saveGraph();
    void saveProc();
    void deleteProc();
    void onSensorInfoReceived();
};

#endif // FORMGRAPH_H
