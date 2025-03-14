#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <functional>
#include <ixwebsocket/IXWebSocket.h>

namespace xmrig {

class Controller; // Fremad-deklaration
class WebsocketClient {
public:
    WebsocketClient(Controller *controller,
                    const std::string &url,
                    const std::string &user = {},
                    const std::string &secret = {});
    ~WebsocketClient();

    void start();
    void stop();
    void send(const std::string &json);
    void sendShare(const std::string &jobId, uint64_t diff, uint64_t actual);
    void sendJob(const std::string &algo, uint64_t diff, uint64_t height, int txCount);
    bool isConnected() const;
    void sendStats(double h10s, double h60s, double h15m, uint64_t uptime);

    // NYT
    void setOnSetUrl(const std::function<void(const std::string &)> &cb) { m_onSetUrl = cb; }

private:
    void setupHandlers();
    Controller *m_controller; // Gem en reference til Controller

    std::string m_url;
    std::string m_user;
    std::string m_secret;

    ix::WebSocket m_socket;
    std::atomic<bool> m_connected{false};
    std::thread m_thread;
    std::atomic<bool> m_running{false};

    std::function<void(const std::string &)> m_onSetUrl;
};

} // namespace xmrig
