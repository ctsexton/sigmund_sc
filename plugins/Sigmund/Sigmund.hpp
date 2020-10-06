// PluginSigmund.hpp
// Cameron Sexton (cameron.t.sexton@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"
#include "glue.cpp"

namespace Sigmund {

class Sigmund : public SCUnit {
public:
    Sigmund();

    // Destructor
    // ~Sigmund();

private:
    // Calc function
    void next(int nSamples);

    // Member variables
    int numTracks = 0;
    t_sigmund x;
    t_sigmund* state;

    // bufstuff
    void get_buf();
    SndBuf* m_buf;
    uint32 m_fbufnum;
    float* bufData;
    uint32 bufChannels;
    uint32 bufSamples;
    uint32 bufFrames;
};

} // namespace Sigmund
