#include "base/net/websocket/WebsocketClient.h"
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>

using json = nlohmann::json;

namespace xmrig {

WebsocketClient::WebsocketClient(const std::string &url,
                                 const std::string &user,
                                 const std::string &secret)
    : m_url(url), m_user(user), m_secret(secret) {
    ix::initNetSystem();
}

WebsocketClient::~WebsocketClient() {
    std::cout << "[WS] ~WebsocketClient() called, stopping thread...\n";
    stop();
}

void WebsocketClient::setupHandlers() {
    m_socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            m_connected = true;
            std::cout << "[WS] Connected\n";

            json hello = {
                {"type", "hello"},
                {"user", m_user},
                {"secret", m_secret}
            };
            m_socket.sendText(hello.dump());
        }
        else if (msg->type == ix::WebSocketMessageType::Close) {
            m_connected = false;
            std::cout << "[WS] Disconnected\n";
        }
        else if (msg->type == ix::WebSocketMessageType::Message) {
            std::cout << "[WS] Received: " << msg->str << "\n";
            try {
                json data = json::parse(msg->str);
                if (data["type"] == "reload_config") {
                    std::cout << "[WS] Trigger config reload!\n";
                    // TODO: Hook til reload-funktion
                }
            } catch (...) {
                std::cerr << "[WS] Invalid JSON received\n";
            }
        }
    });
}

void WebsocketClient::start() {
    m_running = true;
    m_thread = std::thread([this]() {
        m_socket.setUrl(m_url);
        setupHandlers();
        m_socket.start();

        while (m_running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));

            if (m_connected) {
                json ping = {
                    {"type", "ping"},
                    {"user", m_user},
                    {"timestamp", std::time(nullptr)}
                };
                send(ping.dump());
            }
        }

        m_socket.stop(1000, "XMRig shutting down");

    });
}

void WebsocketClient::stop() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void WebsocketClient::send(const std::string &json) {
    if (m_connected) {
        m_socket.sendText(json);
    }
}

void WebsocketClient::sendShare(const std::string &jobId, uint64_t diff, uint64_t actual) {
    if (!m_connected) return;

    json msg = {
        {"type", "share"},
        {"user", m_user},
        {"result", "accepted"},
        {"difficulty", diff},
        {"actualDifficulty", actual},
        {"jobId", jobId},
        {"timestamp", std::time(nullptr)}
    };

    std::cout << "[WS] Sending share: jobId=" << jobId
              << " diff=" << diff
              << " actual=" << actual << std::endl;
    std::cout << "[WS] Payload: " << msg.dump() << std::endl;

    send(msg.dump());
}

void WebsocketClient::sendJob(const std::string &algo, uint64_t diff, uint64_t height, int txCount) {
    if (!m_connected) return;

    json msg = {
        {"type", "job"},
        {"user", m_user},
        {"algo", algo},
        {"difficulty", diff},
        {"height", height},
        {"txCount", txCount},
        {"timestamp", std::time(nullptr)}
    };

    std::cout << "[WS] Sending job: height=" << height
              << " algo=" << algo
              << " diff=" << diff
              << " tx=" << txCount << std::endl;

    send(msg.dump());
}

bool WebsocketClient::isConnected() const {
    return m_connected;
}

} // namespace xmrig