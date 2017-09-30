#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <glog/logging.h>
#include <QDesktopWidget>

using namespace google;
int main(int argc, char *argv[])
{
	// Initialize Google's logging library.
	QString currentDir = QDir::currentPath() + "/log";
	if (!QDir(currentDir).exists())
		QDir().mkdir(currentDir);
	InitGoogleLogging(argv[0]);
	SetLogDestination(GLOG_INFO, (currentDir+"/Info").toLatin1().data());
	SetLogDestination(GLOG_WARNING, (currentDir + "/Warning").toLatin1().data());
	SetLogDestination(GLOG_ERROR, (currentDir + "/Error").toLatin1().data());
	SetLogDestination(GLOG_FATAL, (currentDir + "/Fatal").toLatin1().data());

	QApplication app( argc, argv );
	app.setOrganizationName("ECAT");
	app.setApplicationName("Vision Tool");
	//set app style by qss file
	QFile fstyle(":qdarkstyle/style.qss");
	if (!fstyle.exists())
		qDebug("Unable to set stylesheet, file not found\n");
	else{
		fstyle.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&fstyle);
		app.setStyleSheet(ts.readAll());
	}
	//
	QDesktopWidget* desktop = QApplication::desktop();
	int w = desktop->width();
	int h = desktop->height();
	MainWindow mainWindow;
	//mainWindow.setWindowFlags(mainWindow.windowFlags()& ~Qt::WindowCloseButtonHint);   //Disable the window close btn
	/*mainWindow.setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
	mainWindow.setMouseTracking(true);*/
	//mainWindow.move((w - mainWindow.width()) / 2, (h - mainWindow.height()) / 2);
	mainWindow.show();
	return( app.exec() );
}

