#ifndef REQUEST_RESPONSE_TCP_SERVER_H
#define REQUEST_RESPONSE_TCP_SERVER_H

#include <QTcpServer>

#include <memory>
#include <map>
#include <chrono>
#include <functional>

class ResponseWriter {
public:
    ResponseWriter(uint64_t responseId, std::function<void(const std::vector<uint8_t> &)> sendImpl);

    ~ResponseWriter();

    /**
     * Get access to binn object where all response data
     * should be written
     * @return
     */
    void *getBinnObj();

    /**
     * Finalize response and send it to client
     */
    void send();

private:
    uint64_t responseId;
    void *responseData = nullptr;

    std::function<void(const std::vector<uint8_t> &)> sendImpl;
};

class RRTcpServer : public QObject {
Q_OBJECT

public:
    explicit RRTcpServer(int port);

    /**
     * Set handler for incoming requests
     * Response should be written using given response writer
     * To send response call send() in given response writer
     */
    void setRequestHandler(
            std::function<void(void *requestData, std::shared_ptr<ResponseWriter> responseWriter)>);

public slots:

    void onNewConnection();

    void onReadClient();

    void onDisconnected();

private:

    struct ClientInfo {
        QTcpSocket *socket;
        uint32_t bytesLeft = 0;
        /**
         * Time when size was received
         */
        std::chrono::steady_clock::time_point t0;

        /**
         * How many milliseconds request reception took
         */
        size_t durationMillis;
        std::vector<uint8_t> data;
    };

    std::shared_ptr<QTcpServer> tcpServer;

    /**
     * Maps client port to client information (which has client request)
     * As soon as all bytes are received, request data is processed and cleared
     */
    std::map<uint16_t, ClientInfo> clients;

    std::function<void(void *requestData,
                       std::shared_ptr<ResponseWriter> responseWriter)> requestHandler;

    /**
     * Send data to client on specific port
     * @param port
     * @param data
     */
    void sendData(const std::vector<uint8_t> &data, uint16_t port);

    void addClient(QTcpSocket *socket);

    void onData(uint16_t port, std::vector<uint8_t> data);

};

#endif //REQUEST_RESPONSE_TCP_SERVER_H
