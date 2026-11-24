#include <cstdint>
#include <cstring>
#include <cstdio>
#include <vector>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace {

ISteamNetworkingSockets* g_pInterface = nullptr;
HSteamNetPollGroup g_hPollGroup = k_HSteamNetPollGroup_Invalid;
std::vector<uint32_t> g_connectionEvents;

void OnServerConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_Connecting:
        if (g_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
            g_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
            return;
        }
        if (!g_pInterface->SetConnectionPollGroup(pInfo->m_hConn, g_hPollGroup)) {
            g_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
            return;
        }
        break;

    case k_ESteamNetworkingConnectionState_Connected:
        g_connectionEvents.push_back(1u);
        g_connectionEvents.push_back(static_cast<uint32_t>(pInfo->m_hConn));
        break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
            g_connectionEvents.push_back(2u);
            g_connectionEvents.push_back(static_cast<uint32_t>(pInfo->m_hConn));
        } else if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
            g_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        }
        break;

    default:
        break;
    }
}

void OnClientConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
    switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_Connecting:
        if (g_pInterface->SetConnectionPollGroup(pInfo->m_hConn, g_hPollGroup)) {
            return;
        }
        g_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        g_connectionEvents.push_back(2u);
        g_connectionEvents.push_back(static_cast<uint32_t>(pInfo->m_hConn));
        break;

    case k_ESteamNetworkingConnectionState_Connected:
        g_connectionEvents.push_back(1u);
        g_connectionEvents.push_back(static_cast<uint32_t>(pInfo->m_hConn));
        break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        g_connectionEvents.push_back(2u);
        g_connectionEvents.push_back(static_cast<uint32_t>(pInfo->m_hConn));
        break;

    default:
        break;
    }
}

} // namespace

extern "C" {

void __cdecl GNS_Startup() {
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
        std::printf("GameNetworkingSockets_Init failed. %s\n", errMsg);
    }

    g_pInterface = SteamNetworkingSockets();

    SteamNetworkingIdentity identity;
    identity.SetLocalHost();
    g_pInterface->ResetIdentity(&identity);

    g_hPollGroup = g_pInterface->CreatePollGroup();
    if (g_hPollGroup == k_HSteamNetPollGroup_Invalid) {
        std::printf("Failed to create poll group\n");
    }
}

void __cdecl GNS_Shutdown() {
    if (g_hPollGroup != k_HSteamNetPollGroup_Invalid) {
        g_pInterface->DestroyPollGroup(g_hPollGroup);
        g_hPollGroup = k_HSteamNetPollGroup_Invalid;
    }

    if (g_pInterface != nullptr) {
        GameNetworkingSockets_Kill();
        g_pInterface = nullptr;
    }
}

uint32_t __cdecl GNS_Recieve(uint8_t* transportBuffer, int32_t* resultSize, int32_t maxTransportSize) {
    *resultSize = 0;
    if (g_pInterface == nullptr) {
        return 0u;
    }

    ISteamNetworkingMessage* pMsg = nullptr;
    if (g_pInterface->ReceiveMessagesOnPollGroup(g_hPollGroup, &pMsg, 1) < 1) {
        return 0u;
    }

    const uint32_t conn = static_cast<uint32_t>(pMsg->m_conn);
    if (conn != 0u) {
        memcpy_s(transportBuffer, static_cast<size_t>(maxTransportSize), pMsg->m_pData, static_cast<size_t>(pMsg->m_cbSize));
        *resultSize = pMsg->m_cbSize;
    }

    pMsg->Release();
    return conn;
}

int32_t __cdecl GNS_ConnectionEvents(uint32_t* eventBuffer, int32_t maxEventSize) {
    (void)maxEventSize;
    g_pInterface->RunCallbacks();

    int32_t count = 0;
    for (uint32_t value : g_connectionEvents) {
        eventBuffer[count++] = value;
    }
    g_connectionEvents.clear();
    return count / 2;
}

uint32_t __cdecl GNS_ListenIPPort(const char* addrStr) {
    SteamNetworkingIPAddr addr;
    addr.ParseString(addrStr);

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               reinterpret_cast<void*>(&OnServerConnectionStatusChanged));

    return static_cast<uint32_t>(g_pInterface->CreateListenSocketIP(addr, 1, &opt));
}

uint32_t __cdecl GNS_Listen(int32_t port) {
    SteamNetworkingIPAddr addr;
    addr.Clear();
    addr.m_port = static_cast<uint16_t>(port);

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               reinterpret_cast<void*>(&OnServerConnectionStatusChanged));

    return static_cast<uint32_t>(g_pInterface->CreateListenSocketIP(addr, 1, &opt));
}

void __cdecl GNS_CloseListen(uint32_t socket) {
    g_pInterface->CloseListenSocket(static_cast<HSteamListenSocket>(socket));
}

uint32_t __cdecl GNS_ConnectString(const char* addrStr) {
    SteamNetworkingIPAddr addr;
    addr.ParseString(addrStr);

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               reinterpret_cast<void*>(&OnClientConnectionStatusChanged));

    return static_cast<uint32_t>(g_pInterface->ConnectByIPAddress(addr, 1, &opt));
}

uint32_t __cdecl GNS_Connect(const SteamNetworkingIPAddr* addr) {
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               reinterpret_cast<void*>(&OnClientConnectionStatusChanged));

    return static_cast<uint32_t>(g_pInterface->ConnectByIPAddress(*addr, 1, &opt));
}

void __cdecl GNS_CloseSocket(uint32_t socket) {
    g_pInterface->CloseConnection(static_cast<HSteamNetConnection>(socket), 0, "Closed", true);
}

void __cdecl GNS_SocketSend(uint32_t socket, const uint8_t* sendBuffer, int32_t sendAmount) {
    g_pInterface->SendMessageToConnection(
        static_cast<HSteamNetConnection>(socket),
        sendBuffer,
        static_cast<uint32_t>(sendAmount),
        k_nSteamNetworkingSend_Reliable,
        nullptr);
}

bool __cdecl GNS_GetSocketAddress(uint32_t socket, char* addressBuffer, int32_t addressBufferSize) {
    SteamNetConnectionInfo_t info;
    if (!g_pInterface->GetConnectionInfo(static_cast<HSteamNetConnection>(socket), &info)) {
        return false;
    }

    info.m_addrRemote.ToString(addressBuffer, static_cast<size_t>(addressBufferSize), true);
    return true;
}

} // extern "C"
