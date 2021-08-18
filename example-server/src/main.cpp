#include "task.h"

#include <QCoreApplication>
#include <QTimer>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("com.funbiscuit");
    QCoreApplication::setApplicationName("Qt RR Example Server");
    QCoreApplication::setApplicationVersion("0.1.0");

    // Task parented to the application so that it
    // will be deleted by the application.
    Task task(&app);

    // This will run the task from the application event loop.
    QTimer::singleShot(0, [&task]() {
        task.run();
    });

    return QCoreApplication::exec();
}
