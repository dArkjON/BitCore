// Copyright (c) 2018 The Bitcoin Core developers
// Copyright (c) 2025 Bitcore BTX developers
// Distributed under the MIT software license

#include <interfaces/node.h>

#include <addrdb.h>
#include <banman.h>
#include <chain.h>
#include <chainparams.h>
#include <init.h>
#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include <net.h>
#include <net_processing.h>
#include <netaddress.h>
#include <netbase.h>
#include <policy/feerate.h>
#include <policy/fees.h>
#include <primitives/block.h>
#include <rpc/server.h>
#include <shutdown.h>
#include <sync.h>
#include <txmempool.h>
#include <ui_interface.h>
#include <util/system.h>
#include <util/memory.h>
#include <validation.h>
#include <warnings.h>

#ifdef ENABLE_WALLET
#include <wallet/fees.h>
#include <wallet/wallet.h>
#endif

#include <univalue.h>

namespace interfaces {
namespace {

class NodeImpl : public Node
{
public:
    bool parseParameters(int argc, const char* const argv[], std::string& error) override
    {
        return gArgs.ParseParameters(argc, argv, error);
    }
    bool readConfigFiles(std::string& error) override
    {
        return gArgs.ReadConfigFiles(error, true);
    }
    bool softSetArg(const std::string& arg, const std::string& value) override
    {
        return gArgs.SoftSetArg(arg, value);
    }
    bool softSetBoolArg(const std::string& arg, bool value) override
    {
        return gArgs.SoftSetBoolArg(arg, value);
    }
    void selectParams(const std::string& network) override
    {
        SelectParams(network);
    }
    uint64_t getAssumedBlockchainSize() override
    {
        // Bitcore: Return estimated blockchain size (in GB)
        return 5; // ~5GB for Bitcore mainnet
    }
    uint64_t getAssumedChainStateSize() override
    {
        // Bitcore: Return estimated chainstate size (in GB)
        return 1; // ~1GB for Bitcore mainnet
    }
    std::string getNetwork() override
    {
        return Params().NetworkIDString();
    }
    void initLogging() override
    {
        InitLogging();
    }
    void initParameterInteraction() override
    {
        InitParameterInteraction();
    }
    std::string getWarnings(const std::string& type) override
    {
        return GetWarnings(type);
    }
    uint32_t getLogCategories() override
    {
        // Bitcore: Logging categories
        return 0; // Return all categories enabled
    }
    bool baseInitialize() override
    {
        return AppInitBasicSetup() && AppInitParameterInteraction() && AppInitSanityChecks() &&
               AppInitLockDataDirectory();
    }
    bool appInitMain() override
    {
        return AppInitMain();
    }
    void appShutdown() override
    {
        Interrupt();
        Shutdown();
    }
    void startShutdown() override
    {
        StartShutdown();
    }
    bool shutdownRequested() override
    {
        return ShutdownRequested();
    }
    void setupServerArgs() override
    {
        SetupServerArgs();
    }
    void mapPort(bool use_upnp) override
    {
        if (use_upnp) {
            StartMapPort();
        } else {
            InterruptMapPort();
            StopMapPort();
        }
    }
    bool getProxy(Network net, proxyType& proxy_info) override
    {
        return GetProxy(net, proxy_info);
    }
    size_t getNodeCount(CConnman::NumConnections flags) override
    {
        return g_connman ? g_connman->GetNodeCount(flags) : 0;
    }
    bool getNodesStats(NodesStats& stats) override
    {
        stats.clear();
        if (g_connman) {
            std::vector<CNodeStats> stats_temp;
            g_connman->GetNodeStats(stats_temp);
            stats.reserve(stats_temp.size());
            for (auto& node_stats_temp : stats_temp) {
                stats.emplace_back(std::move(node_stats_temp), false, CNodeStateStats());
            }
            TRY_LOCK(::cs_main, locked_main);
            if (!locked_main) {
                return false;
            }
            for (auto& node_stats : stats) {
                std::get<1>(node_stats) =
                    GetNodeStateStats(std::get<0>(node_stats).nodeid, std::get<2>(node_stats));
            }
            return true;
        }
        return false;
    }
    bool getBanned(banmap_t& banmap) override
    {
        if (g_banman) {
            g_banman->GetBanned(banmap);
            return true;
        }
        return false;
    }
    bool ban(const CNetAddr& net_addr, BanReason reason, int64_t ban_time_offset) override
    {
        if (g_banman) {
            g_banman->Ban(net_addr, reason, ban_time_offset);
            return true;
        }
        return false;
    }
    bool unban(const CSubNet& ip) override
    {
        if (g_banman) {
            g_banman->Unban(ip);
            return true;
        }
        return false;
    }
    bool disconnect(const CNetAddr& net_addr) override
    {
        // Bitcore: DisconnectNode only accepts string or NodeId
        if (g_connman) {
            return g_connman->DisconnectNode(net_addr.ToString());
        }
        return false;
    }
    bool disconnect(NodeId id) override
    {
        if (g_connman) {
            return g_connman->DisconnectNode(id);
        }
        return false;
    }
    int64_t getTotalBytesRecv() override
    {
        return g_connman ? g_connman->GetTotalBytesRecv() : 0;
    }
    int64_t getTotalBytesSent() override
    {
        return g_connman ? g_connman->GetTotalBytesSent() : 0;
    }
    size_t getMempoolSize() override
    {
        return ::mempool.size();
    }
    size_t getMempoolDynamicUsage() override
    {
        return ::mempool.DynamicMemoryUsage();
    }
    bool getHeaderTip(int& height, int64_t& block_time) override
    {
        LOCK(::cs_main);
        if (::pindexBestHeader) {
            height = ::pindexBestHeader->nHeight;
            block_time = ::pindexBestHeader->GetBlockTime();
            return true;
        }
        return false;
    }
    int getNumBlocks() override
    {
        LOCK(::cs_main);
        return ::chainActive.Height();
    }
    int64_t getLastBlockTime() override
    {
        LOCK(::cs_main);
        if (::chainActive.Tip()) {
            return ::chainActive.Tip()->GetBlockTime();
        }
        return 0;
    }
    double getVerificationProgress() override
    {
        const CBlockIndex* tip;
        {
            LOCK(::cs_main);
            tip = ::chainActive.Tip();
        }
        return GuessVerificationProgress(Params().TxData(), tip);
    }
    bool isInitialBlockDownload() override
    {
        return IsInitialBlockDownload();
    }
    bool getReindex() override
    {
        return fReindex;
    }
    bool getImporting() override
    {
        return fImporting;
    }
    void setNetworkActive(bool active) override
    {
        if (g_connman) {
            g_connman->SetNetworkActive(active);
        }
    }
    bool getNetworkActive() override
    {
        return g_connman && g_connman->GetNetworkActive();
    }
    CAmount getMaxTxFee() override
    {
        return ::maxTxFee;
    }
    CFeeRate estimateSmartFee(int num_blocks, bool conservative, int* returned_target) override
    {
        FeeCalculation fee_calc;
        CFeeRate result = ::feeEstimator.estimateSmartFee(num_blocks, &fee_calc, conservative);
        if (returned_target) {
            *returned_target = fee_calc.returnedTarget;
        }
        return result;
    }
    CFeeRate getDustRelayFee() override
    {
        // Bitcore: Use minRelayTxFee as dust relay fee
        return ::minRelayTxFee;
    }
    UniValue executeRpc(const std::string& command, const UniValue& params, const std::string& uri) override
    {
        JSONRPCRequest req;
        req.params = params;
        req.strMethod = command;
        req.URI = uri;
        return ::tableRPC.execute(req);
    }
    std::vector<std::string> listRpcCommands() override
    {
        return ::tableRPC.listCommands();
    }
    void rpcSetTimerInterfaceIfUnset(RPCTimerInterface* iface) override
    {
        RPCSetTimerInterfaceIfUnset(iface);
    }
    void rpcUnsetTimerInterface(RPCTimerInterface* iface) override
    {
        RPCUnsetTimerInterface(iface);
    }
    bool getUnspentOutput(const COutPoint& output, Coin& coin) override
    {
        LOCK(::cs_main);
        return ::pcoinsTip->GetCoin(output, coin);
    }
    std::string getWalletDir() override
    {
#ifdef ENABLE_WALLET
        return GetWalletDir().string();
#else
        return "";
#endif
    }
    std::vector<std::string> listWalletDir() override
    {
        std::vector<std::string> paths;
#ifdef ENABLE_WALLET
        // Bitcore: ListWalletDir not available, return empty
#endif
        return paths;
    }
    std::vector<std::unique_ptr<Wallet>> getWallets() override
    {
        std::vector<std::unique_ptr<Wallet>> wallets;
#ifdef ENABLE_WALLET
        for (const std::shared_ptr<CWallet>& wallet : GetWallets()) {
            wallets.emplace_back(MakeWallet(wallet));
        }
#endif
        return wallets;
    }
    std::unique_ptr<Wallet> loadWallet(const std::string& name, std::string& error, std::string& warning) override
    {
#ifdef ENABLE_WALLET
        // Bitcore: LoadWallet not available, wallets loaded via init
        (void)name;
        (void)warning;
        error = "Wallet loading not supported in Bitcore 0.18";
        return nullptr;
#else
        return nullptr;
#endif
    }
    std::unique_ptr<Handler> handleInitMessage(InitMessageFn fn) override
    {
        return MakeHandler(::uiInterface.InitMessage.connect(fn));
    }
    std::unique_ptr<Handler> handleMessageBox(MessageBoxFn fn) override
    {
        return MakeHandler(::uiInterface.ThreadSafeMessageBox.connect(fn));
    }
    std::unique_ptr<Handler> handleQuestion(QuestionFn fn) override
    {
        return MakeHandler(::uiInterface.ThreadSafeQuestion.connect(fn));
    }
    std::unique_ptr<Handler> handleShowProgress(ShowProgressFn fn) override
    {
        return MakeHandler(::uiInterface.ShowProgress.connect(fn));
    }
    std::unique_ptr<Handler> handleLoadWallet(LoadWalletFn fn) override
    {
#ifdef ENABLE_WALLET
        return MakeHandler(::uiInterface.LoadWallet.connect([fn](std::shared_ptr<CWallet> wallet) {
            fn(MakeWallet(wallet));
        }));
#else
        return nullptr;
#endif
    }
    std::unique_ptr<Handler> handleNotifyNumConnectionsChanged(NotifyNumConnectionsChangedFn fn) override
    {
        return MakeHandler(::uiInterface.NotifyNumConnectionsChanged.connect(fn));
    }
    std::unique_ptr<Handler> handleNotifyNetworkActiveChanged(NotifyNetworkActiveChangedFn fn) override
    {
        return MakeHandler(::uiInterface.NotifyNetworkActiveChanged.connect(fn));
    }
    std::unique_ptr<Handler> handleNotifyAlertChanged(NotifyAlertChangedFn fn) override
    {
        return MakeHandler(::uiInterface.NotifyAlertChanged.connect(fn));
    }
    std::unique_ptr<Handler> handleBannedListChanged(BannedListChangedFn fn) override
    {
        return MakeHandler(::uiInterface.BannedListChanged.connect(fn));
    }
    std::unique_ptr<Handler> handleNotifyBlockTip(NotifyBlockTipFn fn) override
    {
        return MakeHandler(
            ::uiInterface.NotifyBlockTip.connect([fn](bool initial_download, const CBlockIndex* block) {
                fn(initial_download, block->nHeight, block->GetBlockTime(),
                    GuessVerificationProgress(Params().TxData(), block));
            }));
    }
    std::unique_ptr<Handler> handleNotifyHeaderTip(NotifyHeaderTipFn fn) override
    {
        return MakeHandler(
            ::uiInterface.NotifyHeaderTip.connect([fn](bool initial_download, const CBlockIndex* block) {
                fn(initial_download, block->nHeight, block->GetBlockTime(),
                    GuessVerificationProgress(Params().TxData(), block));
            }));
    }
};

} // namespace

std::unique_ptr<Node> MakeNode()
{
    return MakeUnique<NodeImpl>();
}

} // namespace interfaces
