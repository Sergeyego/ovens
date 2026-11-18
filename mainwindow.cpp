#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tabManager = new TabManager(ui->tabWidget,this);

    actAction(ui->actionGraph,&MainWindow::newFormGraph,3);
    actAction(ui->actionProgWire,&MainWindow::newFormProgWire,3);

    actAction(ui->actionOven,&MainWindow::newFormOven,3);
    actAction(ui->actionProfile,&MainWindow::newFormProfile,3);

    loadAnalytics();

    tabManager->loadSettings();
    loadSettings();
    clearEmptyMenus();
}

MainWindow::~MainWindow()
{
    tabManager->saveSettings();
    saveSettings();
    delete ui;
}

void MainWindow::actAction(QAction *a, void (MainWindow::*sl)(), int lev)
{
    if (RestConnection::instance()->groups().contains(lev)){
        connect(a, &QAction::triggered, this, sl);
        tabManager->actAction(a);
    } else {
        delete a;
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->restoreState(settings.value("main_state").toByteArray());
    this->restoreGeometry(settings.value("main_geometry").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("main_state", this->saveState());
    settings.setValue("main_geometry", this->saveGeometry());
}

void MainWindow::loadAnalytics()
{
    QByteArray data;
    bool ok = RestConnection::instance()->sendSyncGet("api/olap/prog/"+QApplication::applicationName(),data);
    if (ok){
        QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray arr=doc.array();
        for (const QJsonValue &value : arr) {
            int id=value.toObject().value("id").toInt();
            QString nam=value.toObject().value("nam").toString();
            QAction *act = ui->menu_analytics->addAction(nam);
            act->setProperty("id_olap",id);
            actAction(act,&MainWindow::newAnalytics,3);
        }
    }
}

void MainWindow::clearEmptyMenus()
{
    QMenuBar* menuBar = this->menuBar();
    QList<QAction*> actions = menuBar->actions();

    for (int i = actions.count() - 1; i >= 0; --i) {
        QAction* action = actions.at(i);
        QMenu* menu = action->menu();
        if (menu != nullptr && menu->isEmpty()) {
            menuBar->removeAction(action);
            menu->deleteLater();
        }
    }
}

void MainWindow::newFormGraph()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormGraph(),sender());
    }
}

void MainWindow::newFormProgWire()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormProgWire(),sender());
    }
}

void MainWindow::newFormOven()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormOven(),sender());
    }
}

void MainWindow::newAnalytics()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action){
        int id = action->property("id_olap").toInt();
        if (!tabManager->exist(sender()) && id>0){
            tabManager->addSubWindow(new CubeWidget(id),sender());
        }
    }
}

void MainWindow::newFormProfile()
{
    if (!tabManager->exist(sender())){
        tabManager->addSubWindow(new FormProfile(),sender());
    }
}
