#include "RRTcpServer.h"

#include <QTcpSocket>
#include <binn.h>

#include <iostream>

ResponseWriter::ResponseWriter(uint64_t responseId, std::function<void(const std::vector<uint8_t> &)> sendImpl) :
        responseId(responseId), sendImpl(std::move(sendImpl)) {
    responseData = binn_object();
}

ResponseWriter::~ResponseWriter() {
    binn_free((binn *) responseData);
}

void *ResponseWriter::getBinnObj() {
    return responseData;
}

void ResponseWriter::send() {
    binn *root = binn_object();

    binn_object_set_uint64(root, "response", responseId);
    binn_object_set_object(root, "data", responseData);

    std::vector<uint8_t> vec(binn_size(root));
    memcpy(vec.data(), binn_ptr(root), vec.size());

    sendImpl(vec);

    binn_free(root);
}

RRTcpServer::RRTcpServer(int port) {
    tcpServer = std::make_shared<QTcpServer>(this);
    if (!tcpServer->listen(QHostAddress::Any, port)) {
        std::string description = "Server Error: Unable to start the server: ";
        description.append(tcpServer->errorString().toStdString());
        tcpServer->close();
        throw std::exception(description.c_str());
    }
    connect(tcpServer.get(), SIGNAL(newConnection()),
            this, SLOT(onNewConnection()));

    std::cout << "Started listening on port " << port << "\n";
}

void RRTcpServer::setRequestHandler(std::function<void(void *,
                                                       std::shared_ptr<ResponseWriter>)> handler) {
    requestHandler = std::move(handler);
}

void RRTcpServer::onNewConnection() {
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, SIGNAL(disconnected()),
            this, SLOT(onDisconnected()));
    connect(clientSocket, SIGNAL(readyRead()),
            this, SLOT(onReadClient()));

    //for now support only single connection and just disconnect all others
    for (auto &client : clients) {
        client.second.socket->deleteLater();
    }

    addClient(clientSocket);

    std::cout << "Established connection with " << clientSocket->peerPort() << "\n";
}

void RRTcpServer::onReadClient() {
    auto *clientSocket = (QTcpSocket *) sender();
    if (clientSocket == nullptr)
        return;

    auto peerPort = clientSocket->peerPort();
    auto it = clients.find(peerPort);
    if (it == clients.end()) {
        // should not happen, all connected clients are added to clients map
        std::cerr << "Received request from unregistered client! Ignoring!\n";
        clientSocket->deleteLater();
        return;
    }

    while (clientSocket->bytesAvailable() > 0) {
        auto &request = clients[peerPort];
        if (request.bytesLeft == 0) {
            if (clientSocket->bytesAvailable() >= sizeof(uint32_t)) {
                clientSocket->read(reinterpret_cast<char *>(&request.bytesLeft), sizeof(uint32_t));
                request.t0 = std::chrono::steady_clock::now();
                request.data.reserve(request.bytesLeft);
                continue;
            } else {
                // not enough data yet
                return;
            }
        }

        auto readSize = std::min(request.bytesLeft, (uint32_t) clientSocket->bytesAvailable());
        auto arr = clientSocket->read(readSize);

        std::copy(arr.begin(), arr.end(), std::back_inserter(request.data));
        request.bytesLeft -= readSize;
        if (request.bytesLeft == 0) {
            using namespace std::chrono;
            request.durationMillis = duration_cast<milliseconds>(steady_clock::now() - request.t0).count();

            onData(peerPort, std::move(request.data));
            request.data.clear();
        }
    }
}

void RRTcpServer::onDisconnected() {
    auto *clientSocket = (QTcpSocket *) sender();
    if (clientSocket == nullptr)
        return;
    auto peerPort = clientSocket->peerPort();
    auto it = clients.find(peerPort);
    if (it != clients.end()) {
        clients.erase(it);
    }

    std::cout << "Client " << peerPort << " disconnected\n";
    clientSocket->deleteLater();
}

void RRTcpServer::sendData(const std::vector<uint8_t> &data, uint16_t port) {
    QByteArray datagram;

    std::copy(data.begin(), data.end(), std::back_inserter(datagram));

    uint32_t sz = data.size();

    auto client = clients.find(port);
    if (client == clients.end())
        return;
    client->second.socket->write(reinterpret_cast<const char *>(&sz), sizeof(sz));
    client->second.socket->write(datagram);
}

void RRTcpServer::addClient(QTcpSocket *socket) {
    auto peerPort = socket->peerPort();
    auto it = clients.find(peerPort);
    if (it == clients.end()) {
        clients.emplace(peerPort, ClientInfo());
    }
    clients[peerPort].socket = socket;
}

void RRTcpServer::onData(uint16_t port, std::vector<uint8_t> data) {
    binn request;

    if (!binn_load(data.data(), &request)) {
        std::cerr << "Received invalid data\n";
        return;
    }

    uint64_t requestId;

    if (!binn_object_get_uint64(&request, "request", &requestId)) {
        std::cerr << "Received object doesn't have 'request' field\n";
        return;
    }

    void *requestData;
    if (!binn_object_get_object(&request, "data", &requestData)) {
        std::cerr << "Received object doesn't have 'data' field\n";
        return;
    }

    if (!requestHandler)
        return;

    auto response = std::make_shared<ResponseWriter>(requestId, [this, port](const std::vector<uint8_t> &data) {
        sendData(data, port);
    });
    requestHandler(requestData, std::move(response));
}
