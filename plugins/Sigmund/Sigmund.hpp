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
    t_sigmund* state;
    t_sigmund x;

    // bufstuff
    void get_buf();
    SndBuf* m_buf;
    uint32 m_fbufnum;
};

} // namespace Sigmund
