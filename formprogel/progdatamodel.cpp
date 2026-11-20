#include "progdatamodel.h"
#include <QUrlQuery>
#include <QDebug>

ProgDataModel::ProgDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

ProgDataModel::~ProgDataModel()
{
    abortRequest();
}

void ProgDataModel::abortRequest()
{
    if (m_reply) {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    m_loading = false;
}

void ProgDataModel::load(int id_prog, double vl, double temp, int id_oven)
{
    abortRequest();

    // Формируем путь. vl и temp передаём с точкой (QLocale::c)
    const QString path = QString("api/proc/progdata/%1/%2/%3/%4")
                             .arg(id_prog)
                             .arg(QString::number(vl, 'f', 2))
                             .arg(QString::number(temp, 'f', 2))
                             .arg(id_oven);

    const QUrl url = RestConnection::instance()->buildUrl(path);
    m_reply = RestConnection::instance()->sendGet(url);
    if (!m_reply) {
        emit errorOccurred(tr("Не удалось создать запрос"));
        emit loadingFinished(false);
        return;
    }

    m_loading = true;
    emit loadingStarted();
    connect(m_reply, &QNetworkReply::finished,
            this, &ProgDataModel::onReplyFinished);
}

void ProgDataModel::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    // Если это уже неактуальный запрос — игнорируем
    if (reply != m_reply) {
        reply->deleteLater();
        return;
    }

    m_reply = nullptr;
    m_loading = false;

    const auto err = reply->error();
    const QByteArray data = reply->readAll();
    reply->deleteLater();

    if (err == QNetworkReply::OperationCanceledError) {
        // Штатная отмена — не считаем ошибкой
        emit loadingFinished(false);
        return;
    }

    if (err != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        emit loadingFinished(false);
        return;
    }

    parseXml(data);
    emit loadingFinished(true);
}

void ProgDataModel::parseXml(const QByteArray &xml)
{
    beginResetModel();
    m_data.clear();

    QXmlStreamReader reader(xml);
    Item current;
    bool inItem = false;

    while (!reader.atEnd() && !reader.hasError()) {
        reader.readNext();

        if (reader.isStartElement()) {
            const QString name = reader.name().toString();
            if (name == QLatin1String("item")) {
                inItem = true;
                current = Item();
            } else if (inItem) {
                if (name == QLatin1String("step")) {
                    current.step = reader.readElementText().toInt();
                } else if (name == QLatin1String("tim")) {
                    current.tim = reader.readElementText().toDouble();
                } else if (name == QLatin1String("tra")) {
                    current.tra = reader.readElementText().toDouble();
                }
            }
        } else if (reader.isEndElement()) {
            if (reader.name() == QLatin1String("item") && inItem) {
                m_data.append(current);
                inItem = false;
            }
        }
    }

    if (reader.hasError()) {
        qWarning() << "ProgDataModel: XML parse error:" << reader.errorString();
    }

    endResetModel();
}

void ProgDataModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

int ProgDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_data.size();
}

int ProgDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return 3; // step, tim, tra
}

QVariant ProgDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= m_data.size()) return QVariant();
    if (index.column() < 0 || index.column() >= 3) return QVariant();

    const Item &item = m_data.at(index.row());

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return item.step;
        case 1: return item.tim;
        case 2: return item.tra;
        }
    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return item.step;
        case 1: return QLocale().toString(item.tim,'f',1);
        case 2: return QLocale().toString(item.tra,'f',1);
        }
    } else if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    }

    return QVariant();
}

QVariant ProgDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Шаг");
        case 1: return tr("Время");
        case 2: return tr("Температура");
        }
    }

    return QVariant();
}

Qt::ItemFlags ProgDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}