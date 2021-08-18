#ifndef RR_EXAMPLE_CLIENT_TASK_H
#define RR_EXAMPLE_CLIENT_TASK_H

#include <QObject>
#include <memory>

class RRTcpClient;

class Task : public QObject {
Q_OBJECT

public:
    explicit Task(QObject *parent = nullptr);
    void run();

private:

    std::shared_ptr<RRTcpClient> tcpClient;
};

#endif //RR_EXAMPLE_CLIENT_TASK_H
