#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoLTF.h"
using namespace slhcl1tt;

int TrackFitterAlgoLTF::fit(const TTRoadComb& acomb, TTTrack2& atrack)
{
    normChi2_ = -1.;

    if (emu_ == 0) {
        // Arrange the stub coordinates in the format expected by the fitter
        std::vector<double> vars;
        for (unsigned istub = 0; istub < acomb.stubs_phi.size(); ++istub) {
            vars.push_back(acomb.stubs_phi.at(istub));
            vars.push_back(acomb.stubs_r.at(istub));
            vars.push_back(acomb.stubs_z.at(istub));
        }
        normChi2_ = linearizedTrackFitter_->fit(vars, acomb.hitBits);
        fillTrack(linearizedTrackFitter_, atrack);
        atrack.setPreEstimatedPt(linearizedTrackFitter_->preEstimatedPt());
    }
    else {
        // Arrange the stub coordinates in the format expected by the fitter
        std::vector<bigInt> varsInt;
        std::vector<int> stripIndexes;
        for (unsigned istub = 0; istub<acomb.stubs_phi.size(); ++istub) {
            varsInt.push_back(acomb.stubs_phi_int(istub));
            varsInt.push_back(acomb.stubs_r_int(istub));
            varsInt.push_back(acomb.stubs_z_int(istub));
            stripIndexes.push_back(int(acomb.strip_int(istub)));
        }
        normChi2_ = linearizedTrackFitterEmulator_->fit(varsInt, stripIndexes, acomb.hitBits);
        // // Use this to run the emulator on floating point input with internal encoding
        // std::vector<double> vars;
        // for (unsigned istub = 0; istub < acomb.stubs_phi.size(); ++istub) {
        //     vars.push_back(acomb.stubs_phi.at(istub));
        //     vars.push_back(acomb.stubs_r.at(istub));
        //     vars.push_back(acomb.stubs_z.at(istub));
        // }
        // normChi2_ = linearizedTrackFitterEmulator_->fit(vars, stripIndexes, acomb.hitBits);
        fillTrack(linearizedTrackFitterEmulator_, atrack);
        atrack.setParsInt(linearizedTrackFitterEmulator_->estimatedParsInt());
        atrack.setChi2TermsInt(linearizedTrackFitterEmulator_->chi2TermsInt());
    }

    return 0;
}
