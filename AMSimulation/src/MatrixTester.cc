#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/MatrixTester.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Statistics.h"
#include <iomanip>
#include <fstream>


// _____________________________________________________________________________
int MatrixTester::setupTriggerTower(TString datadir) {
    TString csvfile1 = datadir + "trigger_sector_map.csv";
    TString csvfile2 = datadir + "trigger_sector_boundaries.csv";

    try {
        ttmap_ -> readTriggerTowerMap(csvfile1);

    } catch (const std::invalid_argument& e) {
        std::cout << Error() << "Failed to parse: " << csvfile1 << ". What: " << e.what() << std::endl;
        return 1;
    }

    try {
        ttmap_ -> readTriggerTowerBoundaries(csvfile2);

    } catch (const std::invalid_argument& e) {
        std::cout << Error() << "Failed to parse: " << csvfile2 << ". What: " << e.what() << std::endl;
        return 1;
    }

    //ttmap_ -> print();
    return 0;
}

// _____________________________________________________________________________
// Build matrices
int MatrixTester::testMatrices(TString src) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and generating patterns." << std::endl;

    // _________________________________________________________________________
    // For reading
    TTStubReader reader(verbose_);
    if (reader.init(src, false)) {
        std::cout << Error() << "Failed to initialize TTStubReader." << std::endl;
        return 1;
    }

    // _________________________________________________________________________
    // Get trigger tower reverse map
    const std::map<unsigned, bool>& ttrmap = ttmap_ -> getTriggerTowerReverseMap(po_.tower);

    // _________________________________________________________________________
    // Loop over all events (filter)

    if (verbose_)  std::cout << Info() << "Begin event filtering" << std::endl;

    // Event decisions
    std::vector<bool> keepEvents;

    // Bookkeepers
    long int nRead = 0, nKept = 0;

//#define Use_keepEvents_txt_
#ifdef Use_keepEvents_txt_
    TString txt = "keepEvents.txt";
    std::ifstream infile(txt.Data());
    if (!infile) {
        std::cout << "Unable to open " << txt << std::endl;
        return 1;
    }
    int x;
    while (infile >> x) {
        long long ievt = nRead;
        if (verbose_>1 && ievt%100000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7ld", ievt, nKept) << std::endl;

        keepEvents.push_back(x);

        if (x)
            ++nKept;
        ++nRead;
    }
#else
    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%100000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7ld", ievt, nKept) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        // Apply track pt requirement
        float simPt = reader.vp_pt->front();
        if (simPt < po_.minPt || po_.maxPt < simPt) {
            ++nRead;
            keepEvents.push_back(false);
            continue;
        }

        // Apply trigger tower acceptance
        unsigned ngoodstubs = 0;
        for (unsigned istub=0; istub<nstubs; ++istub) {
            unsigned moduleId = reader.vb_modId   ->at(istub);
            if (ttrmap.find(moduleId) != ttrmap.end()) {
                ++ngoodstubs;
            }
        }
        if (ngoodstubs != po_.nLayers) {
            ++nRead;
            keepEvents.push_back(false);
            continue;
        }
        assert(nstubs == po_.nLayers);

        ++nKept;
        ++nRead;
        keepEvents.push_back(true);
    }
#endif

    if (nRead == 0) {
        std::cout << Error() << "Failed to read any event." << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << Form("Read: %7ld, kept: %7ld", nRead, nKept) << std::endl;


    // _________________________________________________________________________
    // Loop over all events

    if (verbose_)  std::cout << Info() << "Begin first loop on tracks" << std::endl;

    // Containers
    std::vector<TTTrack2> tracks;
    tracks.reserve(300);

    // Statistics
    std::vector<Statistics> statV(nvariables_);
    std::vector<Statistics> statP(nparameters_);

    // Bookkeepers
    nRead = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%100000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7ld", ievt, nKept) << std::endl;

        if (!keepEvents.at(ievt)) {
            ++nRead;
            continue;
        }

        // _____________________________________________________________________
        // Start fitting tracks

        tracks.clear();
        int fitstatus = 0;

        TTRoadComb acomb;
        acomb.roadRef    = 0;
        acomb.patternRef = 0;
        acomb.tower      = po_.tower;

        acomb.stubRefs.clear();
        for (unsigned istub=0; istub<nstubs; ++istub) {
            acomb.stubRefs.push_back(istub);
        }

        acomb.nstubs     = 0;
        acomb.stubs_r   .clear();
        acomb.stubs_phi .clear();
        acomb.stubs_z   .clear();
        acomb.stubs_bool.clear();
        for (unsigned istub=0; istub<acomb.stubRefs.size(); ++istub) {
            const unsigned stubRef = acomb.stubRefs.at(istub);
            if (stubRef != 999999) {
                ++acomb.nstubs;
                acomb.stubs_r   .push_back(reader.vb_r   ->at(stubRef));
                acomb.stubs_phi .push_back(reader.vb_phi ->at(stubRef));
                acomb.stubs_z   .push_back(reader.vb_z   ->at(stubRef));
                acomb.stubs_bool.push_back(true);
            } else {
                acomb.stubs_r   .push_back(0.);
                acomb.stubs_phi .push_back(0.);
                acomb.stubs_z   .push_back(0.);
                acomb.stubs_bool.push_back(false);
            }
        }

        // _____________________________________________________________________
        // Fit
        TTTrack2 atrack;
        fitstatus = fitterPCA_->fit(acomb, atrack);

        atrack.setTower    (acomb.tower);
        atrack.setHitBits  (acomb.hitbits());
        atrack.setPtSegment(acomb.ptsegment());
        atrack.setRoadRef  (acomb.roadRef);
        atrack.setStubRefs (acomb.stubRefs);
        tracks.push_back(atrack);

        if (verbose_>2)  std::cout << Debug() << "... track: " << 0 << " status: " << fitstatus << std::endl;


        std::vector<float> principals(nvariables_);
        std::vector<double> parameters(nparameters_);
        std::vector<double> parameters_fit(nparameters_);

        // _____________________________________________________________________
        // Get sim info
        // Get sim info
        double simCotTheta     = std::sinh(reader.vp_eta->front());
        double simChargeOverPt = float(reader.vp_charge->front())/reader.vp_pt->front();
        double simPhi          = reader.vp_phi->front();
        double simVz           = reader.vp_vz->front();
        //double simC = 0.5 * (0.003 * 3.8 * simChargeOverPt);  // 1/(2 x radius of curvature)
        //double simT = simCotTheta;
        {
            unsigned ipar = 0;
            parameters.at(ipar++) = simPhi;
            parameters.at(ipar++) = simCotTheta;
            parameters.at(ipar++) = simVz;
            parameters.at(ipar++) = simChargeOverPt;
        }

        // _____________________________________________________________________
        // Get fit info
        {
            unsigned ipar = 0;
            parameters_fit.at(ipar++) = atrack.phi0();
            parameters_fit.at(ipar++) = atrack.cottheta();
            parameters_fit.at(ipar++) = atrack.z0();
            parameters_fit.at(ipar++) = atrack.rinv() / (0.003 * 3.8);

            assert(atrack.principals().size() == nvariables_);
            principals = atrack.principals();
        }


        for (unsigned ivar=0; ivar<nvariables_; ++ivar) {
            statV.at(ivar).fill(principals.at(ivar));
        }

        for (unsigned ipar=0; ipar<nparameters_; ++ipar) {
            statP.at(ipar).fill(parameters_fit.at(ipar) - parameters.at(ipar));
        }

        ++nKept;
        ++nRead;
    }

    if (verbose_>1) {
        std::ios::fmtflags flags = std::cout.flags();
        std::cout << std::setprecision(4);
        std::cout << Info() << "statV: " << std::endl;
        for (unsigned ivar=0; ivar<nvariables_; ++ivar) {
            std::cout << "principal " << ivar << ": " << statV.at(ivar).getEntries() << " " << statV.at(ivar).getMean() << " " << statV.at(ivar).getSigma() << std::endl;
        }
        std::cout << Info() << "statP: " << std::endl;
        for (unsigned ipar=0; ipar<nparameters_; ++ipar) {
            std::cout << "parameter " << ipar << ": " << statP.at(ipar).getEntries() << " " << statP.at(ipar).getMean() << " " << statP.at(ipar).getSigma() << std::endl;
        }
        std::cout.flags(flags);
    }

    return 0;
}


// _____________________________________________________________________________
// Main driver
int MatrixTester::run() {
    int exitcode = 0;
    Timing(1);

    exitcode = setupTriggerTower(po_.datadir);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = testMatrices(po_.input);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
