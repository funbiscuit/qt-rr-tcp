#include "task.h"

#include "RRTcpServer.h"

#include <binn.h>

#include <iostream>
#include <QTimer>

Task::Task(QObject *parent) : QObject(parent) {
}

void Task::run() {
    std::cout << "Request-Response Example Server\n";

    tcpServer = std::make_shared<RRTcpServer>(4141);

    tcpServer->setRequestHandler([](void *request, std::shared_ptr<ResponseWriter> responseWriter) {
        std::cout << "Received request:\n";

        binn_iter iter;
        binn value;
        char key[256];  // only for objects
        binn_iter_init(&iter, request, BINN_OBJECT);
        while (binn_object_next(&iter, key, &value)) {
            std::cout << "[" << key << "] = ";
            if (value.type == BINN_STRING) {
                std::cout << (char *) value.ptr;
            } else if (value.type == BINN_DOUBLE) {
                std::cout << value.vdouble;
            } else {
                std::cout << "?";
            }
            std::cout << "\n";
        }
        auto response = (binn *) responseWriter->getBinnObj();
        binn_object_set_str(response, "str", (char *) "hello");

        double num;
        if (binn_object_get_double(request, "num", &num)) {
            binn_object_set_double(response, "num", num / 5);
        }

        if (binn_object_get_double(request, "delay", &num)) {
            QTimer::singleShot((int) num, [responseWriter]() {
                responseWriter->send();
            });
        } else {
            responseWriter->send();
        }
    });
}
