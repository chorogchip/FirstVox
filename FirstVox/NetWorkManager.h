#pragma once

namespace vox::net
{

    int NMInit();
    void NMClear();

    bool NMIsClient();
    bool NMHasConnection();


    enum class EnumPacketFlag : int
    {
        SET_BLOCK,
        LOAD_CHUNK,
        GEN_CHUNK,
        DATA_CHUNK,
        SIGNAL,
    };
    struct DefPacket
    {
        EnumPacketFlag flag;
        int x;
        int y;
        int z;
        void* data;
    };

    void NMSendDefPacket(const DefPacket* packet);


}