#include <cstdlib>
#include <uv.h>

#include "App.h"
#include "backend/cpu/Cpu.h"
#include "base/io/Console.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/io/Signals.h"
#include "base/kernel/Platform.h"
#include "core/config/Config.h"
#include "core/Controller.h"
#include "Summary.h"
#include "version.h"
#include "base/net/websocket/WebsocketClient.h"
#include "base/net/websocket/WebsocketCommandHandler.h"
#include "net/Network.h"
#include "base/net/stratum/NetworkState.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

xmrig::App::App(Process *process)
{
    m_controller = std::make_shared<Controller>(process);
}

xmrig::App::~App()
{
    Cpu::release();
}

int xmrig::App::exec()
{
    if (!m_controller->isReady()) {
        LOG_EMERG("no valid configuration found, try https://xmrig.com/wizard");
        return 2;
    }

    int rc = 0;
    if (background(rc)) {
        return rc;
    }

    m_signals = std::make_shared<Signals>(this);

    rc = m_controller->init();
    if (rc != 0) {
        return rc;
    }

    const auto &ws = m_controller->config()->websocket();

    if (ws.isEnabled()) {
        auto handler = std::make_shared<WebsocketCommandHandler>(m_controller.get());
        auto wsClient = std::make_unique<xmrig::WebsocketClient>(m_controller.get(), ws.url(), ws.user(), ws.secret());
        wsClient->setOnSetUrl([handler](const std::string &url) {
            json args = { { "--url", url } };
            handler->handleArgs(args);
        });
        ;

        wsClient->start();
        m_controller->network()->state()->setWebsocketClient(wsClient.get());
        m_controller->setWebsocketClient(std::move(wsClient));
    }

    if (!m_controller->isBackground()) {
        m_console = std::make_shared<Console>(this);
    }

    Summary::print(m_controller.get());

    if (m_controller->config()->isDryRun()) {
        LOG_NOTICE("%s " WHITE_BOLD("OK"), Tags::config());
        return 0;
    }

    m_controller->start();

    rc = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_loop_close(uv_default_loop());

    return rc;
}

void xmrig::App::onConsoleCommand(char command)
{
    if (command == 3) {
        LOG_WARN("%s " YELLOW("Ctrl+C received, exiting"), Tags::signal());
        close();
    }
    else {
        m_controller->execCommand(command);
    }
}

void xmrig::App::onSignal(int signum)
{
    switch (signum)
    {
    case SIGHUP:
    case SIGTERM:
    case SIGINT:
        return close();

    default:
        break;
    }
}

void xmrig::App::close()
{
    m_signals.reset();
    m_console.reset();

    m_controller->stop();

    if (m_wsClient) {
        m_wsClient->stop();
    }
    Log::destroy();
}
