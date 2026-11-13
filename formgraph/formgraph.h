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
#include "plot/cfgplot.h"
#include "rest/restrotablemodel.h"
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
    void loadSettings();
    void saveSettings();
    QString secToStr(qint64 sec);
    QString getSensorInfo(int id_owen, QDate date);
    RestRoTableModel *modelOven;
    void print(QPrinter *p);

private slots:
    void updDry();
    void updGraph();
    void setCurrentProc(int index);
    void setCurrentOven(QModelIndex index);
    void updOven();
    void printGraph();
    void saveGraph();
};

#endif // FORMGRAPH_H
