#include "task.h"

#include "RRTcpClient.h"

#include <binn.h>

#include <iostream>
#include <chrono>

#include <QCoreApplication>

Task::Task(QObject *parent) : QObject(parent) {
}

void Task::run() {
    std::cout << "Request-Response Example Server\n";

    tcpClient = std::make_shared<RRTcpClient>("localhost", 4141);

    binn *request = binn_object();

    binn_object_set_double(request, "num", 21);
    binn_object_set_double(request, "delay", 50);
    std::vector<uint8_t> test(1024 * 1024);
    binn_object_set_blob(request, "test", test.data(), (int) test.size());

    using namespace std::chrono;
    auto t0 = steady_clock::now();

    tcpClient->sendRequest(request, [t0](void *response, std::runtime_error *error) {
        if (error != nullptr) {
            std::cerr << "Error while waiting for response:\n" << error->what() << "\n";
            QCoreApplication::quit();
            return;
        }
        auto ms = duration_cast<milliseconds>(steady_clock::now() - t0).count();

        std::cout << "Received response (took " << ms << "ms):\n";
        binn_iter iter;
        binn value;
        char key[256];
        binn_iter_init(&iter, response, BINN_OBJECT);
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

        QCoreApplication::quit();
    });

    binn_free(request);
}
