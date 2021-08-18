#include "RRTcpClient.h"

#include <binn.h>

#include <iostream>

RRTcpClient::RRTcpClient(const QString &hostAddress, int hostPort) {
    tcpSocket = std::make_shared<QTcpSocket>(this);

    tcpSocket->connectToHost(hostAddress, hostPort);

    connect(tcpSocket.get(), SIGNAL(connected()), SLOT(slotConnected()));
    connect(tcpSocket.get(), SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(tcpSocket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError))
    );
}

void RRTcpClient::sendRequest(void *requestData, std::function<void(void *, std::runtime_error *)> responseHandler) {
    binn *request = binn_object();

    binn_object_set_uint64(request, "request", nextId);
    binn_object_set_object(request, "data", requestData);

    uint32_t size = binn_size(request);
    tcpSocket->write((char *) &size, sizeof(size));
    tcpSocket->write((char *) binn_ptr(request), size);

    responseHandlers[nextId] = std::move(responseHandler);

    binn_free(request);

    ++nextId;
}

bool RRTcpClient::isWaitingResponse() const {
    return !responseHandlers.empty();
}

void RRTcpClient::slotReadyRead() {
    for (;;) {
        if (nextBlockSize == 0) {
            if (tcpSocket->bytesAvailable() < sizeof(nextBlockSize)) {
                break;
            }
            tcpSocket->read(reinterpret_cast<char *>(&nextBlockSize), sizeof(nextBlockSize));
            rxData.clear();
            rxData.reserve(nextBlockSize);
        }
        if (tcpSocket->bytesAvailable() == 0)
            break;

        uint32_t sz = std::min(nextBlockSize - rxData.size(), (uint32_t) tcpSocket->bytesAvailable());
        uint32_t prevSize = rxData.size();
        uint32_t newSize = prevSize + sz;
        rxData.resize(newSize);
        tcpSocket->read((char *) &rxData[prevSize], sz);

        if (rxData.size() == nextBlockSize) {
            nextBlockSize = 0;
            onData(std::move(rxData));
        }
    }
}

void RRTcpClient::slotError(QAbstractSocket::SocketError err) {
    std::string strError = "Error: ";
    strError.append(err == QAbstractSocket::HostNotFoundError ?
                    "The host was not found." :
                    err == QAbstractSocket::RemoteHostClosedError ?
                    "The remote host is closed." :
                    err == QAbstractSocket::ConnectionRefusedError ?
                    "The connection was refused." :
                    tcpSocket->errorString().toStdString());

    std::runtime_error error(strError);
    for(auto &p : responseHandlers) {
        p.second(nullptr, &error);
    }
    responseHandlers.clear();
}

void RRTcpClient::slotConnected() {
    std::cout << "TCP Client is connected\n";
}

void RRTcpClient::onData(std::vector<uint8_t> data) {
    binn response;
    if (!binn_load(data.data(), &response)) {
        std::cerr << "Received invalid response\n";
        return;
    }

    uint64_t responseId;
    void *responseData;

    if (!binn_object_get_uint64(&response, "response", &responseId)) {
        std::cerr << "Received object doesn't have 'response' field\n";
        return;
    }

    if (!binn_object_get_object(&response, "data", &responseData)) {
        std::cerr << "Received object doesn't have 'data' field\n";
        return;
    }

    auto it = responseHandlers.find(responseId);
    if (it == responseHandlers.end()) {
        std::cerr << "Received unexpected response\n";
        return;
    }

    it->second(responseData, nullptr);
    responseHandlers.erase(it);
}