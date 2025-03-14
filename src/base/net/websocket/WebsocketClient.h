#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <ixwebsocket/IXWebSocket.h>

namespace xmrig {

class WebsocketClient {
public:
    WebsocketClient(const std::string &url,
                    const std::string &user = {},
                    const std::string &secret = {});

    ~WebsocketClient();

    void start();
    void stop();
    void send(const std::string &json);
    void sendShare(const std::string &jobId, uint64_t diff, uint64_t actual);
    bool isConnected() const;

private:
    void setupHandlers();

    std::string m_url;
    std::string m_user;
    std::string m_secret;

    ix::WebSocket m_socket;
    std::atomic<bool> m_connected{false};
    std::thread m_thread;
    std::atomic<bool> m_running{false};
};

} // namespace xmrig
