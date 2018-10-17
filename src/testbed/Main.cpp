#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
	QApplication application(argc, argv);
	MainWindow window;
	Q_UNUSED(window);
	return application.exec();
}
