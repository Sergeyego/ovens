#include "formgraph.h"
#include "ui_formgraph.h"

FormGraph::FormGraph(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormGraph)
{
    ui->setupUi(this);

    plot = new Plot(this);
    ui->verticalLayoutPlot->addWidget(plot);

    cfgPlot = new CfgPlot(plot,this);
    ui->verticalLayoutCfg->addWidget(cfgPlot);

    plotData = new PlotData(plot,this);
    ui->verticalLayoutChn->insertWidget(0,plotData);

    ui->dateEditBeg->setDate(QDate::currentDate().addDays(-QDate::currentDate().day()+1));
    ui->dateEditEnd->setDate(QDate::currentDate().addDays(1));

    ui->dateTimeEditBeg->setDateTime(QDateTime::currentDateTime().addDays(-3));
    ui->dateTimeEditEnd->setDateTime(QDateTime::currentDateTime().addDays(1));

    modelDry = new RestRoTableModel(this);

    ui->tableViewDry->setModel(modelDry);

    modelOven = new RestRoTableModel(this);
    ui->tableViewOven->setModel(modelOven);
    updOven();

    connect(modelDry,SIGNAL(sigRefresh()),this,SLOT(updDryFinished()));
    connect(ui->tableViewDry->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(setCurrentProc(QModelIndex)));
    connect(ui->pushButtonUpdDry,SIGNAL(clicked(bool)),this,SLOT(updDry()));
    connect(ui->checkBoxMonly,SIGNAL(clicked(bool)),this,SLOT(updDry()));
    connect(ui->radioButtonEl,SIGNAL(clicked(bool)),this,SLOT(updDry()));
    connect(ui->radioButtonWire,SIGNAL(clicked(bool)),this,SLOT(updDry()));
    //connect(ui->tableViewDry->horizontalHeader(),SIGNAL(sectionClicked(int)),modelDry,SLOT(srt(int)));
    connect(ui->checkBoxOst,SIGNAL(clicked(bool)),this,SLOT(updGraph()));
    connect(ui->tableViewOven->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(setCurrentOven(QModelIndex)));
    connect(ui->pushButtonOven,SIGNAL(clicked(bool)),this,SLOT(updOven()));
    connect(ui->pushButtonPrint,SIGNAL(clicked(bool)),this,SLOT(printGraph()));
    connect(ui->pushButtonPdf,SIGNAL(clicked(bool)),this,SLOT(saveGraph()));

    loadSettings();
    updDry();
}

FormGraph::~FormGraph()
{
    saveSettings();
    delete ui;
}

void FormGraph::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("graph_splitter_width").toByteArray());
}

void FormGraph::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("graph_splitter_width",ui->splitter->saveState());
}

QString FormGraph::secToStr(qint64 sec)
{
    if (sec==0) return QString();
    QTime t(0,0,0);
    int d = sec / 86400;
    QString suf = (d>0)? QString::number(d)+QString::fromUtf8(" д ") : QString();
    t=t.addSecs(sec);
    return suf+t.toString("hh:mm:ss");
}

QString FormGraph::getSensorInfo(int id_owen, QDate date)
{
    /*QString info;
    QSqlQuery query;
    query.prepare("select otc.nam, otcs.nam, otcs.num, otcs.dat_pov, otcs.dat_end from owens_trm_channel_sensor otcs "
                  "inner join "
                  "(select otcs2.id_channel, max(otcs2.dat) as dat from owens_trm_channel_sensor otcs2 "
                  "where otcs2.dat <= :dat "
                  "group by otcs2.id_channel) as z on z.dat=otcs.dat and z.id_channel=otcs.id_channel "
                  "inner join owens_trm_channel otc on otc.id=otcs.id_channel "
                  "inner join owens o on o.id = otc.id_owen "
                  "where otc.is_enabled=true and o.id = :id_ow and otcs.dat_end >= :dat2 ");
    query.bindValue(":dat",date);
    query.bindValue(":id_ow",id_owen);
    query.bindValue(":dat2",date);
    if (query.exec()){
        while (query.next()){
            info+="<br>";
            info+=query.value(0).toString()+tr(": датчик ");
            info+=query.value(1).toString()+tr(" №");
            info+=query.value(2).toString()+tr(" поверка до ");
            info+=query.value(4).toDate().toString("dd.MM.yyyy")+tr(";");
        }
    } else {
        QMessageBox::critical(this,"Error",query.lastError().text(),QMessageBox::Ok);
    }
    return info;*/
}

void FormGraph::print(QPrinter *p)
{
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
    renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
    QPainter painter(p);
    QRectF page = p->pageRect(QPrinter::DevicePixel);
    int w = page.width()-page.x();
    QRect rect(page.x(), page.y(), w*0.95, w*1.3);
    renderer.render(plot,&painter,rect);
}

void FormGraph::updDry()
{
    bool b=ui->checkBoxMonly->isChecked();
    ui->dateEditBeg->setEnabled(!b);
    ui->dateEditEnd->setEnabled(!b);
    QDate beg = b? QDate::currentDate().addDays(-30) : ui->dateEditBeg->date();
    QDate end = b? QDate::currentDate().addYears(100) : ui->dateEditEnd->date();
    QString mode = ui->radioButtonEl->isChecked()? "e" : "w";
    QUrlQuery query;
    query.addQueryItem("mode",mode);
    modelDry->setPath("api/proc/ovens/rab/"+beg.toString("yyyy-MM-dd")+"/"+end.toString("yyyy-MM-dd")+"?"+query.toString());
    modelDry->select();
}

void FormGraph::updDryFinished()
{
    if (ui->tableViewDry->model()->rowCount()){
        ui->tableViewDry->selectRow(ui->tableViewDry->model()->rowCount()-1);
        ui->tableViewDry->scrollToBottom();
    }
    double tmax = ui->radioButtonEl->isChecked()? 450 : 900;
    cfgPlot->setYmax(tmax);
}

void FormGraph::updGraph()
{
    setCurrentProc(ui->tableViewDry->currentIndex());
}

void FormGraph::setCurrentProc(QModelIndex index)
{
    int id_owrab=modelDry->getModelData(index.row(),"id").toInt();
    int id_own=modelDry->getModelData(index.row(),"id_o").toInt();
    QDateTime dt_beg = modelDry->getModelData(index.row(),"dt_beg").toDateTime();
    QVariant dt_end = modelDry->getModelData(index.row(),"dt_end");
    QString strEnd;
    if (!dt_end.isNull()){
        strEnd=dt_end.toDateTime().toString("dd.MM.yyyy, HH:mm:ss");
    }

    qint64 tpi=dt_beg.secsTo(dt_end.toDateTime());
    QString tp=secToStr(tpi);

    plot->setbaseTime(dt_beg);

    ui->lineEditT->setText(modelDry->getModelData(index.row(),"t0").toString());
    ui->lineEditW->setText(modelDry->getModelData(index.row(),"w0").toString());
    ui->lineEditZms->setText(modelDry->getModelData(index.row(),"zms").toString());
    ui->lineEditKvo->setText(modelDry->getModelData(index.row(),"kvo").toString());
    ui->lineEditProcBeg->setText(dt_beg.toString("dd.MM.yyyy, HH:mm:ss"));
    ui->lineEditProcEnd->setText(strEnd);
    ui->lineEditS->setText(tp);






    /*QDateTime begTime=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,2),Qt::EditRole).toDateTime();
    QDateTime endTime=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,11),Qt::EditRole).toDateTime();
    bool new_format=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,12),Qt::EditRole).toBool();

    QString own=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,1),Qt::DisplayRole).toString();
    QString mark=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,4),Qt::DisplayRole).toString();
    QString part=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,3),Qt::DisplayRole).toString();

    QString zms=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,7),Qt::DisplayRole).toString();
    QString kvo=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,8),Qt::DisplayRole).toString();
    QString wl=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,6),Qt::DisplayRole).toString();
    QString tl=ui->tableViewDry->model()->data(ui->tableViewDry->model()->index(index,5),Qt::DisplayRole).toString();

    qint64 tpi=begTime.secsTo(endTime);
    QString tp=secToStr(tpi);
    ui->lineEditS->setText(tp);

    plot->setbaseTime(begTime);
    plotData->refresh(id_owrab, new_format,ui->checkBoxOst->isChecked());
    double sec=plot->canvasMap(QwtPlot::xBottom).s2();

    QString vr=(ui->checkBoxOst->isChecked()) ? secToStr(sec) : tp;
    QString title="<FONT SIZE=2>"+own+tr(", ")+mark+tr(", п.")+part+tr(", замес ")+zms+tr(", ")+"<br>"+
                    kvo+tr("кг, w0=")+wl+tr(", t0=")+tl+",<br>"+
                    begTime.toString("dd.MM.yy hh:mm:ss")+" - "+endTime.toString("dd.MM.yy hh:mm:ss")+" ("+vr+")"+"</FONT>";
    title+="<FONT SIZE=1>"+getSensorInfo(id_own,begTime.date())+"</FONT>";;
    plot->setTitle(title);*/
}

void FormGraph::setCurrentOven(QModelIndex index)
{
    /*int id_oven=ui->tableViewOven->model()->data(ui->tableViewOven->model()->index(index.row(),0),Qt::EditRole).toInt();
    QString nam=ui->tableViewOven->model()->data(ui->tableViewOven->model()->index(index.row(),1),Qt::DisplayRole).toString();
    plot->setbaseTime(ui->dateTimeEditBeg->dateTime());
    plotData->refresh(ui->dateTimeEditBeg->dateTime(),ui->dateTimeEditEnd->dateTime(), id_oven);
    plot->setTitle("<FONT SIZE=2>"+nam+"<br>"+ui->dateTimeEditBeg->dateTime().toString("dd.MM.yy hh:mm:ss")+
                   " - "+ui->dateTimeEditEnd->dateTime().toString("dd.MM.yy hh:mm:ss")+"</FONT><FONT SIZE=1>"+getSensorInfo(id_oven,ui->dateTimeEditBeg->dateTime().date())+"</FONT>");*/
}

void FormGraph::updOven()
{
    modelOven->setPath("api/proc/ovens/list");
    modelOven->select();
}

void FormGraph::printGraph()
{
    QPrinter printer;
    printer.setColorMode(QPrinter::Color);
    QPrintDialog dialog(&printer,this);
    if (dialog.exec()){
        print(&printer);
    }
}

void FormGraph::saveGraph()
{
    QSettings settings("szsm", QApplication::applicationName());
    QDir dir(settings.value("savePath",QDir::homePath()).toString());
    QString exportname = QFileDialog::getSaveFileName(this,tr("Сохранить PDF"),dir.path()+"/"+tr("график.pdf"), "*.pdf");
    if (!exportname.isEmpty()) {
        QPrinter p;
        p.setOutputFormat(QPrinter::PdfFormat);
        p.setOutputFileName(exportname);
        p.setColorMode(QPrinter::Color);
        print(&p);
        QFile file(exportname);
        QFileInfo info(file);
        settings.setValue("savePath",info.path());
    }
}