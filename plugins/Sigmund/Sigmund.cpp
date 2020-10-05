// PluginSigmund.cpp
// Cameron Sexton (cameron.t.sexton@gmail.com)

#include "SC_PlugIn.hpp"
#include "Sigmund.hpp"

InterfaceTable* ft;

namespace Sigmund {

Sigmund::Sigmund() {
    mCalcFunc = make_calc_function<Sigmund, &Sigmund::next>();
    numTracks = in0(1);

    state = sigmund_new();

    // NECESSARY FOR GET_BUF
    unit->mWorld = mWorld;
    unit->mParent = mParent;
    unit->mInBuf = mInBuf;
}

void Sigmund::next(int nSamples) {
    const float* input = in(2);
    sigmund_perform(state, input, nSamples);
    sigmund_tick(state);

    GET_BUF

    for (int i = 0; i < state->x_ntrack; i++)
    {
        const int index = i * 3;
        bufData[index] = state->x_trackv[i].p_freq;
        bufData[index + 1] = 2*state->x_trackv[i].p_amp;
        bufData[index + 2] = state->x_trackv[i].p_tmp;
    }
}

} // namespace Sigmund

PluginLoad(SigmundUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<Sigmund::Sigmund>(ft, "Sigmund");
}
