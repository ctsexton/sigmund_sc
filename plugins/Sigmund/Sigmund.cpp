// PluginSigmund.cpp
// Cameron Sexton (cameron.t.sexton@gmail.com)

#include "SC_PlugIn.hpp"
#include "Sigmund.hpp"
#include <iostream>
#include <string>

InterfaceTable* ft;

namespace Sigmund {

Sigmund::Sigmund() {
    mCalcFunc = make_calc_function<Sigmund, &Sigmund::next>();
    numTracks = in0(1);

    std::cout << "MAKING STATE" << std::endl;
    state = &x;

    sigmund_preinit(state);
    std::cout << "preinit done" << std::endl;
    
    // set npeaks
    sigmund_npeak(state, 20);
    
    // toggle tracks calculation

    std::cout << "setting up tracks" << std::endl;
    state->x_dotracks = 1;
    state->x_dopitch = 1;
    state->x_ntrack = state->x_npeak;
    state->x_trackv = (t_peak *)getbytes(state->x_ntrack * sizeof(*state->x_trackv));
    
    std::cout << "setting up infill" << std::endl;
    state->x_infill = 0;
    state->x_countdown = 0;
    state->x_sr = 48000;
    sigmund_npts(state, state->x_npts);

    std::cout << "setting bigbuf size" << std::endl;
    int bigbuf_size = sizeof (t_float) * (2*NEGBINS + 6 * state->x_npts);
    std::cout << "bigbuf size in bytes is: " << std::to_string(bigbuf_size) << std::endl;
    std::cout << "Allocating bigbuf" << std::endl;
    state->bigbuf = (t_float*)RTAlloc(this->mWorld, bigbuf_size);

    notefinder_init(&state->x_notefinder);
    sigmund_clear(state);

    std::cout << "MADE STATE" << std::endl;
}

void Sigmund::get_buf() {
    const Unit* unit = this;

    loginfo("get fbufnum...");
    float fbufnum = ZIN0(0);
    if (fbufnum < 0.f) {   
        fbufnum = 0.f;
    }
    if (fbufnum != m_fbufnum) {
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
    }
    /* loginfo("lock sndbuf..."); */
    /* LOCK_SNDBUF(m_buf); */
}

void Sigmund::next(int nSamples) {
    loginfo("computing: " + std::to_string(nSamples));
    const float* input = in(2);
    loginfo("perform...");
    sigmund_perform(state, input, nSamples);
    loginfo("tick...");
    sigmund_tick(state);

    loginfo("get_buf..");
    get_buf();

    if (state->x_infill == state->x_npts) 
    {
      loginfo("loop over ntrack...");
      for (int i = 0; i < state->x_ntrack; i++)
      {
          const int index = i * 3;
          loginfo("set freq...");
          m_buf->data[index] = state->x_trackv[i].p_freq;
          loginfo("set amp...");
          m_buf->data[index + 1] = 2*state->x_trackv[i].p_amp;
          loginfo("set tmp...");
          m_buf->data[index + 2] = state->x_trackv[i].p_tmp;
      }
    }
}

} // namespace Sigmund

PluginLoad(SigmundUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<Sigmund::Sigmund>(ft, "Sigmund");
}
