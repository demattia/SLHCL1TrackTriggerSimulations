#ifndef AMSimulation_SuperstripArbiter_h_
#define AMSimulation_SuperstripArbiter_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Helper.h"

#include <vector>

namespace slhcl1tt {

class SuperstripArbiter {
  public:
    // Constructors
    // subLadderSize, subModuleSize, subLadderECSize, subModuleECSize are
    // expressed as if each module has 32 subLadders and 1024 subModules
    SuperstripArbiter(unsigned subladder, unsigned submodule)
    : barrel_n_(6), endcap_n_(15),
      barrel_subladder_sizes_(barrel_n_, subladder), barrel_submodule_sizes_(barrel_n_, submodule),
      endcap_subladder_sizes_(endcap_n_, subladder), endcap_submodule_sizes_(endcap_n_, submodule) {
        init();
    }

    SuperstripArbiter(const std::vector<unsigned>& b_subladders, const std::vector<unsigned>& b_submodules,
                      const std::vector<unsigned>& e_subladders, const std::vector<unsigned>& e_submodules)
    : barrel_n_(6), endcap_n_(15),
      barrel_subladder_sizes_(b_subladders), barrel_submodule_sizes_(b_submodules),
      endcap_subladder_sizes_(e_subladders), endcap_submodule_sizes_(e_submodules) {
        init();
    }

    // Destructor
    ~SuperstripArbiter() {}


    // Operators
    // Return the superstrip address given the strip address
    unsigned superstripLayer(const unsigned& lay) const;
    unsigned superstrip(unsigned lay, unsigned lad, unsigned mod, unsigned col, unsigned row, bool isHalfStrip=true) const;

    // Debug
    void print();

  private:
    // Initialize
    void init();

  private:
    // Member data
    const unsigned barrel_n_;
    const unsigned endcap_n_;

    // Variable size
    std::vector<unsigned> barrel_subladder_sizes_;
    std::vector<unsigned> barrel_submodule_sizes_;
    std::vector<unsigned> endcap_subladder_sizes_;
    std::vector<unsigned> endcap_submodule_sizes_;

    // Compress using geometry knowledge
    std::vector<unsigned> barrel_z_divisions_;
    std::vector<unsigned> barrel_phi_divisions_;
    std::vector<unsigned> barrel_layer_offsets_;
    std::vector<unsigned> endcap_ring_divisions_;
    std::vector<unsigned> endcap_ring_offsets_;
    std::vector<unsigned> calo_offsets_;
    std::vector<unsigned> muon_offsets_;
    std::vector<unsigned> fake_offsets_;

    unsigned max_subladder_bits_;
    unsigned max_submodule_bits_;
};

}  // namespace slhcl1tt

#endif
