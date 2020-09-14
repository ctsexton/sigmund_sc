// PluginSigmund.cpp
// Cameron Sexton (cameron.t.sexton@gmail.com)

#include "SC_PlugIn.hpp"
#include "Sigmund.hpp"

static InterfaceTable* ft;

namespace Sigmund {

Sigmund::Sigmund() {
    mCalcFunc = make_calc_function<Sigmund, &Sigmund::next>();
    next(1);
}

void Sigmund::next(int nSamples) {
    const float* input = in(0);
    const float* gain = in(0);
    float* outbuf = out(0);

    // simple gain function
    for (int i = 0; i < nSamples; ++i) {
        outbuf[i] = input[i] * gain[i];
    }
}

} // namespace Sigmund

PluginLoad(SigmundUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<Sigmund::Sigmund>(ft, "Sigmund");
}
