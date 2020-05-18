#include "MainWin.hh"

#include <QApplication>

int main(int argc, char * * argv) {
	
	QApplication app { argc, argv };
	
	JAPortalMainWin * win = new JAPortalMainWin;
	win->show();
	
	return app.exec();
}
