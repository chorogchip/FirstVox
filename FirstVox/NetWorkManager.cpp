#include "NetWorkManager.h"

#include "WinSockHeader.h"

#include <iostream>
#include <thread>
#include <cassert>
#include <map>
#include <mutex>

#include "ChunkManager.h"

namespace vox::net
{


    struct NetInfoFile
    {
        int is_client;
        unsigned short port;
        char server_ip[58];  // if this is client

        void LoadFromFile(FILE* fp)
        {
            fscanf_s(fp, "%d", &is_client);
            fscanf_s(fp, "%hu", &port);
            if (is_client)
                fscanf_s(fp, "%58s\n", server_ip, (unsigned)_countof(server_ip));
            char buf[256];
            sprintf_s(buf, 256, "socket info : %d %d %s\n", is_client, port, server_ip);
            OutputDebugStringA(buf);
        }
    };
    static NetInfoFile net_info_;

    static WSADATA wsaData_;
    static SOCKET connect_socket_ = INVALID_SOCKET;
    static sockaddr_in sock_addr;

    static std::thread network_thread_;
    static volatile bool network_thread_is_running_ = false;
    static char packet_buf[1024 * 1024 * 2];
    static std::map<std::tuple<int,int,int>, void*> load_chunk_map_;
    static std::mutex map_lock_;

    static int InitServer();
    static int InitClient();
    static void ServerThreadFunc();
    static void ClientThreadFunc();

    bool NMIsClient()
    {
        return net_info_.is_client;
    }
    bool NMHasConnection()
    {
        return network_thread_is_running_;
    }

    int NMInit()
    {

        int iResult;
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData_);
        if (iResult != 0)
        {
            OutputDebugStringA("wsa startup failed\n");
            return -1;
        }

        FILE *fp;
        fopen_s(&fp, "GameData/netinfo.txt", "r");
        if (fp == nullptr)
        {
            OutputDebugStringA("open file GameData/netinfo.txt failed\n");
            WSACleanup();
            return -1;
        }

        net_info_.LoadFromFile(fp);
        fclose(fp);

        if (net_info_.is_client) return InitClient();
        else return InitServer();
    }

    void NMClear()
    {
        if (network_thread_is_running_)
        {
            network_thread_is_running_ = false;
        }
        if (connect_socket_ != INVALID_SOCKET)
            closesocket(connect_socket_);
        network_thread_.join();
        WSACleanup();
    }

    static int InitServer()
    {
        if ((connect_socket_ = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        {
            OutputDebugStringA("create socket failed\n");
            return -1;
        }

        ZeroMemory(&sock_addr, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = INADDR_ANY;
        sock_addr.sin_port = htons( net_info_.port );
        
        network_thread_is_running_ = true;
        network_thread_ = std::thread(ServerThreadFunc);

        return 0;
    }

    static int InitClient()
    {
        if ((connect_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
        {
            OutputDebugStringA("create socket failed\n");
            return -1;
        }

        ZeroMemory(&sock_addr, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        //InetPton(AF_INET, (PCWSTR)net_info_.server_ip, &sock_addr.sin_addr.s_addr);
        sock_addr.sin_port = htons( net_info_.port );
        //sock_addr.sin_addr.S_un.S_addr = inet_addr(net_info_.server_ip);
        InetPton(AF_INET, L"127.0.0.1", &sock_addr.sin_addr.s_addr);

        DefPacket sig_packet;
        sig_packet.flag = EnumPacketFlag::SIGNAL;
        sendto(connect_socket_, (const char*)&packet_buf, sizeof(sig_packet), 0,
            (sockaddr*)&sock_addr, sizeof(sockaddr));

        network_thread_is_running_ = true;
        network_thread_ = std::thread(ClientThreadFunc);

        return 0;
    }

    static void ServerThreadFunc()
    {
        int sz = sizeof(sockaddr);
        char buf[64];
        while (network_thread_is_running_)
        {
            if (bind(connect_socket_, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
            {
                sprintf_s(buf, sizeof(buf), "bind socket failed : %d\n", WSAGetLastError());
                OutputDebugStringA(buf);
                return;
            }

            OutputDebugStringA("bind socket succeed\n");

            while (network_thread_is_running_)
            {
                recvfrom(connect_socket_, packet_buf, sizeof(packet_buf), 0,
                    (sockaddr*)&sock_addr, &sz);
                DefPacket* recv_packet = (DefPacket*)packet_buf;
                switch(recv_packet->flag)
                {
                case EnumPacketFlag::SET_BLOCK:
                    core::chunkmanager::ProcessSetBlockPacket(
                        recv_packet->x, recv_packet->y, recv_packet->z,
                        data::Block((data::EBlockID)(size_t)recv_packet->data));
                    break;
                case EnumPacketFlag::LOAD_CHUNK:
                    core::chunkmanager::ReplyLoadChunkPacket(recv_packet->x, recv_packet->y, recv_packet->z);
                    break;
                case EnumPacketFlag::GEN_CHUNK:
                case EnumPacketFlag::DATA_CHUNK:
                default:
                    assert(0);
                    break;
                case EnumPacketFlag::SIGNAL:
                    break;
                }
            }
        }
    }

    static void ClientThreadFunc()
    {
        int sz = sizeof(sockaddr);

        while (network_thread_is_running_)
        {
            recvfrom(connect_socket_, packet_buf, sizeof(packet_buf), 0,
                (sockaddr*)&sock_addr, &sz);
            DefPacket* recv_packet = (DefPacket*)packet_buf;
            switch(recv_packet->flag)
            {
            case EnumPacketFlag::SET_BLOCK:
                core::chunkmanager::ProcessSetBlockPacket(
                    recv_packet->x, recv_packet->y, recv_packet->z,
                    data::Block((data::EBlockID)(size_t)recv_packet->data));
                break;
            case EnumPacketFlag::GEN_CHUNK:
            {
                map_lock_.lock();
                auto it = load_chunk_map_.find(std::make_tuple(recv_packet->x, recv_packet->y, recv_packet->z));
                if (it != load_chunk_map_.end())
                {
                    const auto c_ptr = it->second;
                    load_chunk_map_.erase(it);
                    map_lock_.unlock();
                    core::chunkmanager::ProcessGenChunkPacket(c_ptr);
                }
                else
                {
                    map_lock_.unlock();
                }
                break;
            }
            case EnumPacketFlag::DATA_CHUNK:
            {
                map_lock_.lock();
                auto it = load_chunk_map_.find(std::make_tuple(recv_packet->x, recv_packet->y, recv_packet->z));
                const auto c_ptr = it->second;
                load_chunk_map_.erase(it);
                map_lock_.unlock();
                core::chunkmanager::ProcessDataChunkPacket(c_ptr,
                    (size_t*)&recv_packet->data + 1, (size_t)recv_packet->data - sizeof(recv_packet));
                break;
            }
            case EnumPacketFlag::LOAD_CHUNK:
            default:
                assert(0);
                break;
            }
        }
    }

    void NMSendDefPacket(const DefPacket* packet)
    {
        switch(packet->flag)
        {
        case EnumPacketFlag::LOAD_CHUNK:

            map_lock_.lock();

            load_chunk_map_.insert(std::make_pair(
                std::make_tuple(packet->x, packet->y, packet->z),
                packet->data));

            map_lock_.unlock();
            break;

        case EnumPacketFlag::DATA_CHUNK:
            sendto(connect_socket_, (const char*)packet, (int)(size_t)packet->data, 0,
                (const sockaddr*)&sock_addr, sizeof(sockaddr_in));
            return;
        }
        sendto(connect_socket_, (const char*)packet, sizeof(*packet), 0,
            (const sockaddr*)&sock_addr, sizeof(sockaddr_in));
    }


}