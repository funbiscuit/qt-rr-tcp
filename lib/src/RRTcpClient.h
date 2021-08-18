#ifndef REQUEST_RESPONSE_TCP_CLIENT_H
#define REQUEST_RESPONSE_TCP_CLIENT_H

#include <QTcpSocket>

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>

class RRTcpClient : public QObject {
Q_OBJECT
public:
    RRTcpClient(const QString &hostAddress, int hostPort);

    /**
     * Send request
     * When response is received, callback will be called
     * If there was an error while waiting for response, it will be nullptr
     * and error will be non null
     * @param requestData
     * @param responseHandler
     */
    void sendRequest(void *requestData, std::function<void(void *response, std::runtime_error *error)> responseHandler);

    bool isWaitingResponse() const;

private slots:

    void slotReadyRead();

    void slotError(QAbstractSocket::SocketError);

    void slotConnected();

private:

    uint64_t nextId = 1;

    /**
     * Socket for connection to tcp server
     */
    std::shared_ptr<QTcpSocket> tcpSocket;

    /**
     * Size of block that is being receive (0 if none)
     */
    uint32_t nextBlockSize = 0;
    std::vector<uint8_t> rxData;

    std::map<uint64_t, std::function<void(void *response, std::runtime_error *error)>> responseHandlers;

    void onData(std::vector<uint8_t> data);
};

#endif //REQUEST_RESPONSE_TCP_CLIENT_H
