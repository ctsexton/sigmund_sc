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

    // NECESSARY FOR GET_BUF
    struct unitPlaceholder {
      float m_fbufnum;
      SndBuf* m_buf;
      World* mWorld;
      Graph* mParent;
      float** mInBuf;
    };
    struct unitPlaceholder* unit;
};

} // namespace Sigmund
