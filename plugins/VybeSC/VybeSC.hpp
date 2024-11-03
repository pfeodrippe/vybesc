// PluginVybeSC.hpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"

namespace VybeSC {

    class VybeSC : public SCUnit {

    public:
        VybeSC();

        // Destructor
        // ~VybeSC();

    private:
        // Calc function
        void next(int nSamples);

        // Member variables
    };

} // namespace VybeSC
