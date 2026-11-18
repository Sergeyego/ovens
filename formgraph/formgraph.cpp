#include "formgraph.h"
#include "ui_formgraph.h"

FormGraph::FormGraph(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormGraph)
{
    ui->setupUi(this);

    plot = new Plot(this);
    ui->verticalLayoutPlot->addWidget(plot);

    cfgPlot = new CfgPlot(plot, this);
    ui->verticalLayoutCfg->addWidget(cfgPlot);

    plotData = new PlotData(plot, this);
    ui->verticalLayoutChn->insertWidget(0, plotData);

    ui->dateEditBeg->setDate(QDate::currentDate().addDays(-QDate::currentDate().day() + 1));
    ui->dateEditEnd->setDate(QDate::currentDate().addDays(1));

    ui->dateTimeEditBeg->setDateTime(QDateTime::currentDateTime().addDays(-3));
    ui->dateTimeEditEnd->setDateTime(QDateTime::currentDateTime().addDays(1));

    modelDry = new RestRoTableModel(this);
    ui->tableViewDry->setModel(modelDry);

    modelOven = new RestRoTableModel(this);
    ui->tableViewOven->setModel(modelOven);

    connect(modelOven, &RestRoTableModel::sigRefresh,
            this, &FormGraph::updOvenFinished);

    connect(modelDry, &QAbstractItemModel::modelReset,
            this, &FormGraph::updDryFinished);
    connect(ui->tableViewDry->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &FormGraph::setCurrentProc);
    connect(ui->pushButtonUpdDry, &QPushButton::clicked, this, &FormGraph::updDry);
    connect(ui->checkBoxMonly, &QCheckBox::clicked, this, &FormGraph::updDry);
    connect(ui->radioButtonEl, &QRadioButton::clicked, this, &FormGraph::updDry);
    connect(ui->radioButtonWire, &QRadioButton::clicked, this, &FormGraph::updDry);
    connect(ui->checkBoxOst, &QCheckBox::clicked, this, &FormGraph::updGraph);
    connect(ui->tableViewOven->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &FormGraph::setCurrentOven);
    connect(ui->pushButtonOven, &QPushButton::clicked, this, &FormGraph::updOven);
    connect(ui->pushButtonPrint, &QPushButton::clicked, this, &FormGraph::printGraph);
    connect(ui->pushButtonPdf, &QPushButton::clicked, this, &FormGraph::saveGraph);

    connect(ui->pushButtonSave, &QPushButton::clicked, this, &FormGraph::saveProc);

    auto onFieldChanged = [this]() {
        if (!m_loadingData) ui->pushButtonSave->setEnabled(true);
    };
    connect(ui->lineEditT,   &QLineEdit::textChanged, this, onFieldChanged);
    connect(ui->lineEditW,   &QLineEdit::textChanged, this, onFieldChanged);
    connect(ui->lineEditZms, &QLineEdit::textChanged, this, onFieldChanged);
    connect(ui->lineEditKvo, &QLineEdit::textChanged, this, onFieldChanged);

    // Горячие клавиши удаления: Ctrl+Delete и Ctrl+D
    auto *scDel = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete), ui->tableViewDry);
    scDel->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scDel, &QShortcut::activated, this, &FormGraph::deleteProc);

    auto *scD = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), ui->tableViewDry);
    scD->setContext(Qt::WidgetWithChildrenShortcut);
    connect(scD, &QShortcut::activated, this, &FormGraph::deleteProc);

    loadSettings();

    updDry();
    updOven();
}

FormGraph::~FormGraph()
{
    saveSettings();
    delete ui;
}

void FormGraph::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    ui->splitter->restoreState(settings.value("graph_splitter_width").toByteArray());
    if (settings.value("graph_radio_state").toBool()){
        ui->radioButtonWire->setChecked(true);
    } else {
        ui->radioButtonEl->setChecked(true);
    }
}

void FormGraph::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("graph_splitter_width", ui->splitter->saveState());
    settings.setValue("graph_radio_state",ui->radioButtonWire->isChecked());
}

QString FormGraph::secToStr(qint64 sec)
{
    if (sec <= 0) return QString();
    QTime t(0, 0, 0);
    int d = sec / 86400;
    QString suf = (d > 0) ? QString::number(d) + QString::fromUtf8(" д ") : QString();
    t = t.addSecs(sec);
    return suf + t.toString("hh:mm:ss");
}

void FormGraph::print(QPrinter *p)
{
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground);
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame);
    renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
    QPainter painter(p);
    QRectF page = p->pageRect(QPrinter::DevicePixel);
    QRectF rect(page.x(), page.y(), page.width() * 0.95, page.width() * 1.3);
    renderer.render(plot, &painter, rect);
}

void FormGraph::updDry()
{
    bool b = ui->checkBoxMonly->isChecked();
    ui->dateEditBeg->setEnabled(!b);
    ui->dateEditEnd->setEnabled(!b);
    QDate beg = b ? QDate::currentDate().addDays(-30) : ui->dateEditBeg->date();
    QDate end = b ? QDate::currentDate().addYears(100) : ui->dateEditEnd->date();
    QString mode = ui->radioButtonEl->isChecked() ? "e" : "w";
    QUrlQuery query;
    query.addQueryItem("mode", mode);
    modelDry->setPath("api/proc/ovens/rab/" + beg.toString("yyyy-MM-dd") + "/" + end.toString("yyyy-MM-dd") + "?" + query.toString());
    modelDry->select();
}

void FormGraph::updDryFinished()
{
    if (ui->tableViewDry->model()->rowCount()) {
        ui->tableViewDry->selectRow(ui->tableViewDry->model()->rowCount() - 1);
        ui->tableViewDry->scrollToBottom();
    } else {
        ui->pushButtonSave->setEnabled(false);
        ui->lineEditT->clear();
        ui->lineEditW->clear();
        ui->lineEditZms->clear();
        ui->lineEditKvo->clear();
        ui->lineEditProcBeg->clear();
        ui->lineEditProcEnd->clear();
        ui->lineEditS->clear();
        ui->lineEditEnerg->clear();
        ui->lineEditRab->clear();
        ui->lineEditProg->clear();
        plotData->clearData();
        plot->setTitle("");
    }
    double tmax = ui->radioButtonEl->isChecked() ? 450 : 900;
    cfgPlot->setYmax(tmax);
}

void FormGraph::updGraph()
{
    setCurrentProc(ui->tableViewDry->currentIndex());
}

void FormGraph::setCurrentProc(QModelIndex index)
{
    ui->pushButtonSave->setEnabled(false);

    if (index.row() < 0) {
        ui->lineEditT->clear();
        ui->lineEditW->clear();
        ui->lineEditZms->clear();
        ui->lineEditKvo->clear();
        ui->lineEditProcBeg->clear();
        ui->lineEditProcEnd->clear();
        ui->lineEditS->clear();
        ui->lineEditEnerg->clear();
        ui->lineEditRab->clear();
        ui->lineEditProg->clear();
        return;
    }

    const int id_owrab = modelDry->getModelData(index.row(), "id").toInt();
    const int id_own   = modelDry->getModelData(index.row(), "id_o").toInt();
    const QDateTime dt_beg = modelDry->getModelData(index.row(), "dt_beg").toDateTime();
    const QVariant dt_end  = modelDry->getModelData(index.row(), "dt_end");
    const QVariant energ   = modelDry->getModelData(index.row(), "energ");

    if (id_owrab <= 0 || id_own <= 0) return;

    m_loadingData = true;

    const QString strEnerg = energ.isNull() ? "" : QLocale().toString(energ.toDouble(), 'f', 1);
    const QString strEnd   = dt_end.isNull() ? "" : dt_end.toDateTime().toString("dd.MM.yyyy, HH:mm:ss");
    const QString endDisplay = dt_end.isNull()
                                   ? tr("в процессе")
                                   : dt_end.toDateTime().toString("dd.MM.yy hh:mm:ss");

    qint64 tpi = dt_end.isNull() ? 0 : dt_beg.secsTo(dt_end.toDateTime());
    QString tp = secToStr(tpi);
    if (tp.isEmpty() && dt_end.isNull()) tp = tr("в процессе");

    plot->setbaseTime(dt_beg);

    ui->lineEditT->setText(modelDry->getModelData(index.row(), "t0").toString());
    ui->lineEditW->setText(modelDry->getModelData(index.row(), "w0").toString());
    ui->lineEditZms->setText(modelDry->getModelData(index.row(), "zms").toString());
    ui->lineEditKvo->setText(modelDry->getModelData(index.row(), "kvo").toString());
    ui->lineEditProcBeg->setText(dt_beg.toString("dd.MM.yyyy, HH:mm:ss"));
    ui->lineEditProcEnd->setText(strEnd);
    ui->lineEditS->setText(tp);
    ui->lineEditEnerg->setText(strEnerg);
    ui->lineEditRab->setText(modelDry->getModelData(index.row(), "rab").toString());
    ui->lineEditProg->setText(modelDry->getModelData(index.row(), "prog_nam").toString());

    m_loadingData = false;

    plotData->refresh(id_owrab, true, ui->checkBoxOst->isChecked());

    const QString own = modelDry->getModelData(index.row(), "num").toString();
    const QString kvo = modelDry->getModelData(index.row(), "kvo").toString();

    const double sec = plot->canvasMap(QwtPlot::xBottom).s2();
    const QString vr = ui->checkBoxOst->isChecked() ? secToStr(sec) : tp;

    QString baseTitle;
    if (ui->radioButtonEl->isChecked()) {
        // Электроды: марка, партия, замес, t0/w0
        const QString mark = modelDry->getModelData(index.row(), "marka").toString();
        const QString part = modelDry->getModelData(index.row(), "n_s").toString();
        const QString zms  = modelDry->getModelData(index.row(), "zms").toString();
        const QString wl   = modelDry->getModelData(index.row(), "w0").toString();
        const QString tl   = modelDry->getModelData(index.row(), "t0").toString();

        baseTitle = "<FONT SIZE=2>" + own + tr(", ") + mark + tr(", п.") + part +
                    tr(", замес ") + zms + tr(", ") + "<br>" +
                    kvo + tr("кг, w0=") + wl + tr(", t0=") + tl + ",<br>" +
                    dt_beg.toString("dd.MM.yy hh:mm:ss") + " - " + endDisplay +
                    " (" + vr + ")" + "</FONT>";
    } else {
        // Проволока: только программа, загрузка и время
        const QString prog = modelDry->getModelData(index.row(), "prog_nam").toString();

        baseTitle = "<FONT SIZE=2>" + own + tr(", прог. ") + prog + tr(", ") +
                    kvo + tr("кг, ") +
                    dt_beg.toString("dd.MM.yy hh:mm:ss") + " - " + endDisplay +
                    " (" + vr + ")" + "</FONT>";
    }
    requestSensorInfo(id_own, dt_beg.date(), baseTitle);
}

void FormGraph::setCurrentOven(QModelIndex index)
{
    if (index.row() < 0) {
        plotData->clearData();
        return;
    }
    const int id_oven = modelOven->getModelData(index.row(), "id").toInt();
    const QString nam = modelOven->getModelData(index.row(), "num").toString();

    plot->setbaseTime(ui->dateTimeEditBeg->dateTime());
    plotData->refresh(ui->dateTimeEditBeg->dateTime(), ui->dateTimeEditEnd->dateTime(), id_oven);

    const QString baseTitle = "<FONT SIZE=2>" + nam + "<br>" +
                              ui->dateTimeEditBeg->dateTime().toString("dd.MM.yy hh:mm:ss") +
                              " - " + ui->dateTimeEditEnd->dateTime().toString("dd.MM.yy hh:mm:ss") +
                              "</FONT>";

    requestSensorInfo(id_oven, ui->dateTimeEditBeg->dateTime().date(), baseTitle);
}

void FormGraph::updOven()
{
    m_savedOvenId = -1;
    const int row = ui->tableViewOven->currentIndex().row();
    if (row >= 0) {
        m_savedOvenId = modelOven->getModelData(row, "id").toInt();
    }
    m_restoreOvenRow = (sender() == ui->pushButtonOven);

    modelOven->setPath("api/proc/ovens/list");
    modelOven->select();
}

void FormGraph::updOvenFinished()
{
    if (m_restoreOvenRow && m_savedOvenId > 0) {
        const int row = modelOven->findRowByKey(m_savedOvenId, "id");
        if (row >= 0) {
            ui->tableViewOven->selectRow(row);
            ui->tableViewOven->scrollTo(modelOven->index(row, 0));
        }
    }

    m_restoreOvenRow = false;
}

void FormGraph::printGraph()
{
    QPrinter printer;
    printer.setColorMode(QPrinter::Color);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        print(&printer);
    }
}

void FormGraph::saveGraph()
{
    QSettings settings("szsm", QApplication::applicationName());
    QDir dir(settings.value("savePath", QDir::homePath()).toString());
    QString exportname = QFileDialog::getSaveFileName(this, tr("Сохранить PDF"),
                                                      dir.path() + "/" + tr("график.pdf"),
                                                      "*.pdf");
    if (!exportname.isEmpty()) {
        QPrinter p;
        p.setOutputFormat(QPrinter::PdfFormat);
        p.setOutputFileName(exportname);
        p.setColorMode(QPrinter::Color);
        print(&p);
        QFile file(exportname);
        QFileInfo info(file);
        settings.setValue("savePath", info.path());
    }
}

void FormGraph::requestSensorInfo(int id_owen, QDate date, const QString &baseTitle)
{
    if (m_sensorReply) {
        QNetworkReply *old = m_sensorReply.data();
        m_sensorReply = nullptr;
        old->abort();
        old->deleteLater();
    }

    m_pendingBaseTitle = baseTitle;
    m_pendingIdOwen    = id_owen;
    m_pendingDate      = date;

    const QString path = QString("api/proc/ovens/sensorinfo/%1/%2")
                             .arg(id_owen)
                             .arg(date.toString("yyyy-MM-dd"));

    QNetworkReply *reply = RestConnection::instance()->sendGet(
        RestConnection::instance()->buildUrl(path));
    m_sensorReply = reply;
    connect(reply, &QNetworkReply::finished, this, &FormGraph::onSensorInfoReceived);
}

void FormGraph::onSensorInfoReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    const auto err = reply->error();
    reply->deleteLater();

    if (err == QNetworkReply::OperationCanceledError) return;

    if (m_sensorReply != reply) return;
    m_sensorReply = nullptr;

    QString sensorHtml;
    if (err == QNetworkReply::NoError) {
        const QByteArray data = reply->readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            sensorHtml = doc.object().value("html").toString();
        }
    } else {
        sensorHtml = tr("Ошибка загрузки данных о датчиках.");
    }

    plot->setTitle(m_pendingBaseTitle + "<FONT SIZE=1>" + sensorHtml + "</FONT>");
}

void FormGraph::saveProc()
{
    const int row = ui->tableViewDry->currentIndex().row();
    if (row < 0) return;

    const int id = modelDry->getModelData(row, "id").toInt();
    if (id <= 0) return;

    if (ui->lineEditKvo->text().isEmpty() ||
        ui->lineEditW->text().isEmpty()   ||
        ui->lineEditT->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Внимание"),
                             tr("Заполните все числовые поля (Загр., W, T)"));
        return;
    }

    QJsonObject body;
    body["zms"] = ui->lineEditZms->text();
    body["kvo"] = ui->lineEditKvo->text().toDouble();
    body["w0"]  = ui->lineEditW->text().toDouble();
    body["t0"]  = ui->lineEditT->text().toDouble();

    QByteArray data;
    const bool ok = RestConnection::instance()->sendSyncRequest(
        "api/proc/ovens/rab/" + QString::number(id),
        "PATCH", QJsonDocument(body).toJson(), data);

    if (!ok) {
        RestError::critical(tr("Ошибка сохранения"), QString::fromUtf8(data));
        return;
    }

    // Обновляем только изменённые ячейки в памяти
    ui->pushButtonSave->setEnabled(false);

    auto updateCell = [this, row](const QString &col, const QVariant &val) {
        const int c = modelDry->columnIndex(col);
        if (c >= 0) {
            modelDry->setData(modelDry->index(row, c), val, Qt::EditRole);
        }
    };

    updateCell("t0",  body["t0"].toDouble());
    updateCell("w0",  body["w0"].toDouble());
    updateCell("zms", body["zms"].toString());
    updateCell("kvo", body["kvo"].toDouble());
    setCurrentProc(ui->tableViewDry->currentIndex());
}

void FormGraph::deleteProc()
{
    const int row = ui->tableViewDry->currentIndex().row();
    if (row < 0) return;

    const int id = modelDry->getModelData(row, "id").toInt();
    if (id <= 0) return;

    if (!modelDry->getModelData(row, "dt_end").isNull()) {
        QMessageBox::warning(this, tr("Внимание"),
                             tr("Нельзя удалить завершённую прокалку"));
        return;
    }

    const int rc = QMessageBox::question(
        this, tr("Подтвердите удаление"),
        tr("Удалить незавершённую прокалку?"),
        QMessageBox::Yes | QMessageBox::No);

    if (rc != QMessageBox::Yes) return;

    QByteArray data;
    const bool ok = RestConnection::instance()->sendSyncRequest(
        "api/proc/ovens/rab/" + QString::number(id),
        "DELETE", "{}", data);

    if (!ok) {
        return;
    }

    modelDry->removeRow(row);
}