#ifndef PROGDATAMODEL_H
#define PROGDATAMODEL_H
#include <QAbstractTableModel>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QVector>
#include "rest/restconnection.h"

class ProgDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ProgDataModel(QObject *parent = nullptr);
    ~ProgDataModel() override;

    // Запускает асинхронную загрузку данных
    void load(int id_prog, double vl, double temp, int id_oven);
    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:
    void loadingStarted();
    void loadingFinished(bool success);
    void errorOccurred(const QString &message);

private slots:
    void onReplyFinished();

private:
    struct Item {
        int step = 0;
        double tim = 0.0;
        double tra = 0.0;
    };

    QVector<Item> m_data;
    QPointer<QNetworkReply> m_reply;
    bool m_loading = false;

    void abortRequest();
    void parseXml(const QByteArray &xml);
};

#endif // PROGDATAMODEL_H