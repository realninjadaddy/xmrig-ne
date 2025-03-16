/* XMRig
 * Copyright 2025      NinjaDaddy https://github.com/realninjadaddy/xmrig-ne
 *
 *   This file is part of XMRig and is licensed under the GNU General Public License v3 (GPLv3).
 *   You can find a copy of the license at <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <functional>
#include <ixwebsocket/IXWebSocket.h>

namespace xmrig {

class Controller;
class WebsocketCommandHandler;
class WebsocketClient {
public:
WebsocketClient(Controller *controller, std::shared_ptr<WebsocketCommandHandler> handler,
    const std::string &url, const std::string &user, const std::string &secret);

~WebsocketClient();

    void start();
    void stop();
    void send(const std::string &json);
    void sendShare(const std::string &jobId, uint64_t diff, uint64_t actual, const char *error);
    void sendJob(const std::string &algo, uint64_t diff, uint64_t height, int txCount);
    bool isConnected() const;
    void sendStats(double h10s, double h60s, double h15m, uint64_t uptime);

private:
    void setupHandlers();
    Controller *m_controller; 
    std::shared_ptr<WebsocketCommandHandler> m_handler;
    bool m_started { false };
    
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
