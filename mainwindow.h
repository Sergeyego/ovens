#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "tabmanager.h"
#include "rest/restconnection.h"
#include "formgraph/formgraph.h"
#include "formprogwire/formprogwire.h"
#include "formoven/formoven.h"
#include "formprofile/formprofile.h"
#include "rest_olap/cubewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    TabManager *tabManager;
    void actAction(QAction *a, void (MainWindow::*sl)(), int lev);
    void loadSettings();
    void saveSettings();
    void loadAnalytics();
    void clearEmptyMenus();

private slots:
    void newFormGraph();
    void newFormProgWire();
    void newFormOven();
    void newFormProfile();
    void newAnalytics();
};
#endif // MAINWINDOW_H
