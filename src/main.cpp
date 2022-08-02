/*****************************************************************************
 * main.cpp
 * May 14, 2015
 *
 *****************************************************************************/


#include <QApplication>
//#include <QWebSettings>
#include <stdio.h>
#include "mainobject.h"
#include "lockfile.h"

int main(int argc, char *argv[]) {
        QApplication app(argc, argv);
        app.setApplicationName("envgui");
        app.setApplicationVersion("1.0");
        app.setOrganizationName("Howard Logic");
        app.setOrganizationDomain("howardlogic.com");

        LockFile lockFile("envgui");
        if(!lockFile.lock()) {
                printf("There is already an instance of this application running\n");
                return 1;
        }

        MainObject o;
        return app.exec();
}

