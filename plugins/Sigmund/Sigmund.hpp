// PluginSigmund.hpp
// Cameron Sexton (cameron.t.sexton@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"

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
};

} // namespace Sigmund
