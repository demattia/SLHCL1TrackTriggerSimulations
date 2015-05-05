#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PCAMatrix.h"
using namespace slhcl1tt;

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>


int PCAMatrix::read(std::string txt, unsigned nvariables, unsigned nparameters) {

    // Read from file
    std::ifstream infile(txt.c_str());
    if (!infile) {
        std::cout << "Unable to open " << txt << std::endl;
        throw std::runtime_error("Unable to open input file.");
    }

    double x;

    meansR = Eigen::VectorXd::Zero(nvariables/2);
    for (unsigned ivar=0; ivar<nvariables/2; ++ivar) {
        infile >> x;
        meansR(ivar) = x;
    }

    meansC = Eigen::VectorXd::Zero(1);
    infile >> x;
    meansC(0) = x;

    meansT = Eigen::VectorXd::Zero(1);
    infile >> x;
    meansT(0) = x;

    solutionsC = Eigen::MatrixXd::Zero(1,nvariables/2);
    for (unsigned ivar=0; ivar<nvariables/2; ++ivar) {
        infile >> x;
        solutionsC(0, ivar) = x;
    }

    solutionsT = Eigen::MatrixXd::Zero(1,nvariables/2);
    for (unsigned ivar=0; ivar<nvariables/2; ++ivar) {
        infile >> x;
        solutionsT(0, ivar) = x;
    }

    sqrtEigenvalues = Eigen::VectorXd::Zero(nvariables);
    for (unsigned ivar=0; ivar<nvariables; ++ivar) {
        infile >> x;
        sqrtEigenvalues(ivar) = x;
    }

    meansV = Eigen::VectorXd::Zero(nvariables);
    for (unsigned ivar=0; ivar<nvariables; ++ivar) {
        infile >> x;
        meansV(ivar) = x;
    }

    meansP = Eigen::VectorXd::Zero(nparameters);
    for (unsigned ipar=0; ipar<nparameters; ++ipar) {
        infile >> x;
        meansP(ipar) = x;
    }

    V = Eigen::MatrixXd::Zero(nvariables, nvariables);
    for (unsigned ivar=0; ivar<nvariables; ++ivar) {
        for (unsigned jvar=0; jvar<nvariables; ++jvar) {
            infile >> x;
            V(ivar, jvar) = x;
        }
    }

    D = Eigen::MatrixXd::Zero(nparameters, nvariables);
    for (unsigned ipar=0; ipar<nparameters; ++ipar) {
        for (unsigned jvar=0; jvar<nvariables; ++jvar) {
            infile >> x;
            D(ipar, jvar) = x;
        }
    }

    DV = Eigen::MatrixXd::Zero(nparameters, nvariables);
    DV = D * V;

    infile.close();
    return 0;
}

int PCAMatrix::write(std::string txt) {

    // Write to a file
    std::ofstream outfile(txt.c_str());
    if (!outfile) {
        std::cout << "Unable to open " << txt << std::endl;
        throw std::runtime_error("Unable to open output file.");
    }

    outfile << std::setprecision(6);
    outfile << meansR;
    outfile << std::endl << std::endl;
    outfile << meansC;
    outfile << std::endl << std::endl;
    outfile << meansT;
    outfile << std::endl << std::endl;
    outfile << solutionsC;
    outfile << std::endl << std::endl;
    outfile << solutionsT;
    outfile << std::endl << std::endl;

    outfile << sqrtEigenvalues;
    outfile << std::endl << std::endl;
    outfile << meansV;
    outfile << std::endl << std::endl;
    outfile << meansP;
    outfile << std::endl << std::endl;
    outfile << V;
    outfile << std::endl << std::endl;
    outfile << D;
    outfile << std::endl << std::endl;

    outfile.close();
    return 0;
}

void PCAMatrix::print() {
    std::ios::fmtflags flags = std::cout.flags();
    std::cout << std::setprecision(4);

    std::cout << "meansR: " << std::endl;
    std::cout << meansR << std::endl << std::endl;
    std::cout << "meansC: " << std::endl;
    std::cout << meansC << std::endl << std::endl;
    std::cout << "meansT: " << std::endl;
    std::cout << meansT << std::endl << std::endl;
    std::cout << "solutionsC: " << std::endl;
    std::cout << solutionsC << std::endl << std::endl;
    std::cout << "solutionsT: " << std::endl;
    std::cout << solutionsT << std::endl << std::endl;

    std::cout << "sqrtEigenvalues: " << std::endl;
    std::cout << sqrtEigenvalues << std::endl << std::endl;
    std::cout << "meansV: " << std::endl;
    std::cout << meansV << std::endl << std::endl;
    std::cout << "meansP: " << std::endl;
    std::cout << meansP << std::endl << std::endl;
    std::cout << "V: " << std::endl;
    std::cout << V << std::endl << std::endl;
    std::cout << "D: " << std::endl;
    std::cout << D << std::endl << std::endl;

    std::cout.flags(flags);
}
