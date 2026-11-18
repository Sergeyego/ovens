#include "plotdata.h"

PlotData::PlotData(Plot *p, QWidget *parent) :
    QWidget(parent), plot(p)
{
    this->setLayout(new QVBoxLayout(this));
}

void PlotData::clearData()
{
    for (SrcChannel *s : srcData) {
        s->clear();
        this->layout()->removeWidget(s);
        delete s;
    }
    srcData.clear();
    plot->setXmax(0);
}

// ==========================================
// Формирование запроса и постановка в очередь
// ==========================================
void PlotData::refresh(int id_owrab, bool new_format, bool ost)
{
    clearData();

    if (!new_format) {
        // Старая логика (new_format=false) не поддерживается сервером — ничего не делаем
        queue.clear();
        return;
    }

    QUrl url = RestConnection::instance()->buildUrl("api/proc/ovens/plotdata");
    QUrlQuery query;
    query.addQueryItem("id_owrab", QString::number(id_owrab));
    if (ost) {
        query.addQueryItem("ost", "true");
    }
    url.setQuery(query);

    // Актуален только последний запрос
    queue.clear();
    queue.enqueue(url);

    // Если предыдущий запрос в полёте — прерываем его, результат уже не нужен
    if (currentReply) {
        currentReply->disconnect();
        currentReply->abort();
        currentReply->deleteLater();
        currentReply = nullptr;
        isProcessing = false;
    }

    processNextRequest();
}

void PlotData::refresh(QDateTime dBeg, QDateTime dEnd, int id_oven)
{
    clearData();

    QUrl url = RestConnection::instance()->buildUrl("api/proc/ovens/plotdata");
    QUrlQuery query;
    query.addQueryItem("id_oven", QString::number(id_oven));
    query.addQueryItem("dBeg", dBeg.toString(Qt::ISODate));
    query.addQueryItem("dEnd", dEnd.toString(Qt::ISODate));
    url.setQuery(query);

    queue.clear();
    queue.enqueue(url);

    if (currentReply) {
        currentReply->disconnect();
        currentReply->abort();
        currentReply->deleteLater();
        currentReply = nullptr;
        isProcessing = false;
    }

    processNextRequest();
}

// ==========================================
// Очередь запросов
// ==========================================
void PlotData::processNextRequest()
{
    if (queue.isEmpty()) {
        isProcessing = false;
        return;
    }

    isProcessing = true;
    QUrl url = queue.dequeue();

    currentReply = RestConnection::instance()->sendGet(url);
    connect(currentReply, &QNetworkReply::finished, this, &PlotData::onDataReceived);
}

// ==========================================
// Обработка ответа
// ==========================================
void PlotData::onDataReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    if (currentReply == reply) {
        currentReply = nullptr;
    }

    const auto err = reply->error();
    const QString errStr = reply->errorString();
    reply->deleteLater();

    // Штатная отмена — просто переходим к следующему (или завершаем)
    if (err == QNetworkReply::OperationCanceledError) {
        processNextRequest();
        return;
    }

    // Пока ждали ответ — пришёл новый запрос. Текущий результат уже устарел
    if (!queue.isEmpty()) {
        processNextRequest();
        return;
    }

    isProcessing = false;

    if (err != QNetworkReply::NoError) {
        RestError::critical(tr("Ошибка"), errStr);
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isArray()) {
        RestError::critical(tr("Ошибка"), tr("Некорректный ответ сервера"));
        return;
    }

    const QJsonArray arr = doc.array();
    double maxX = 0.0;

    for (const QJsonValue &val : arr) {
        auto *srcCh = new SrcChannel(plot, this);
        double x = srcCh->initFromJson(val.toObject());
        maxX = qMax(maxX, x);
        srcData.push_back(srcCh);
        this->layout()->addWidget(srcCh);
    }

    plot->setXmax(maxX);
}