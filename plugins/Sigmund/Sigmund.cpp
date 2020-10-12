// PluginSigmund.cpp
// Cameron Sexton (cameron.t.sexton@gmail.com)

#include "SC_PlugIn.hpp"
#include "Sigmund.hpp"
#include <iostream>

InterfaceTable* ft;

namespace Sigmund {

Sigmund::Sigmund() {
    mCalcFunc = make_calc_function<Sigmund, &Sigmund::next>();
    numTracks = in0(1);

    std::cout << "MAKING STATE" << std::endl;
    state = sigmund_new(&x);
    std::cout << "MADE STATE" << std::endl;
}

void Sigmund::get_buf() {
    const Unit* unit = this;

    float fbufnum = ZIN0(0);
    if (fbufnum < 0.f) {   
        fbufnum = 0.f;
    }
    uint32 bufnum = (int)fbufnum;
    World* world = unit->mWorld;
    if (bufnum >= world->mNumSndBufs) {
        int localBufNum = bufnum - world->mNumSndBufs;
        Graph* parent = unit->mParent;
        if (localBufNum <= parent->localBufNum) {
            m_buf = parent->mLocalSndBufs + localBufNum;
        } else {
            bufnum = 0;
            m_buf = world->mSndBufs + bufnum;
        }
    } else {
        m_buf = world->mSndBufs + bufnum;
    }
    m_fbufnum = fbufnum;
    SndBuf* buf = m_buf;
    bufData = buf->data;
    bufChannels = buf->channels;
    bufSamples = buf->samples;
    bufFrames = buf->frames;
}

void Sigmund::next(int nSamples) {
    const float* input = in(2);
    sigmund_perform(state, input, nSamples);
    sigmund_tick(state);

    get_buf();

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
