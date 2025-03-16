/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XMRIG_CONTROLLER_H
#define XMRIG_CONTROLLER_H


#include "base/kernel/Base.h"
#include "base/net/stratum/NetworkState.h"
#include "core/config/Config.h"

#include <memory>


namespace xmrig {


class HwApi;
class Job;
class Miner;
class Network;
class WebsocketClient; 

class Controller : public Base, public std::enable_shared_from_this<Controller>
{
public:
    XMRIG_DISABLE_COPY_MOVE_DEFAULT(Controller)

    std::shared_ptr<IConfig> createConfig() const;
    const xmrig::Config* config() const;

    Controller(Process *process);
    ~Controller() override;

    int init() override;
    void start() override;
    void stop() override;

    Miner *miner() const;
    Network *network() const;
    void execCommand(char command) const;

    uint64_t timestamp() const { return m_timestamp; }
    WebsocketClient* websocketClient() const { return m_wsClient.get(); }
    void setWebsocketClient(std::unique_ptr<xmrig::WebsocketClient> client) { m_wsClient = std::move(client); }
    std::string m_wsUrl;
    std::string m_wsUser;
    std::string m_wsSecret;
    

private:
    std::shared_ptr<Miner> m_miner;
    std::shared_ptr<Network> m_network;
    uint64_t m_timestamp = 0;
    std::unique_ptr<xmrig::WebsocketClient> m_wsClient;

#   ifdef XMRIG_FEATURE_API
    std::shared_ptr<HwApi> m_hwApi;
#   endif
};


} // namespace xmrig


#endif /* XMRIG_CONTROLLER_H */