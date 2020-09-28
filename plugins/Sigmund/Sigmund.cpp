// PluginSigmund.cpp
// Cameron Sexton (cameron.t.sexton@gmail.com)

#include "SC_PlugIn.hpp"
#include "Sigmund.hpp"

InterfaceTable* ft;

namespace Sigmund {

Sigmund::Sigmund() {
    mCalcFunc = make_calc_function<Sigmund, &Sigmund::next>();
    numTracks = in0(1);

    // NECESSARY FOR GET_BUF
    unit->mWorld = mWorld;
    unit->mParent = mParent;
    unit->mInBuf = mInBuf;

    next(1);
}

void Sigmund::next(int nSamples) {
    const float* input = in(2);
    const int trackFrequency = 500;
    const int trackAmplitude = 1;
    const int trackStart = 0;
    GET_BUF

    // write each track
    for (int i = 0; i < numTracks; ++i) {
      const int index = i * 3;
      bufData[index] = trackFrequency;
      bufData[index + 1] = trackAmplitude;
      bufData[index + 2] = trackStart;
    }
}

} // namespace Sigmund

PluginLoad(SigmundUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<Sigmund::Sigmund>(ft, "Sigmund");
}
