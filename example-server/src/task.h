#ifndef RR_EXAMPLE_SERVER_TASK_H
#define RR_EXAMPLE_SERVER_TASK_H

#include <QObject>
#include <memory>

class RRTcpServer;

class Task : public QObject {
Q_OBJECT

public:
    explicit Task(QObject *parent = nullptr);

public slots:

    void run();

private:

    std::shared_ptr<RRTcpServer> tcpServer;

};

#endif //RR_EXAMPLE_SERVER_TASK_H
