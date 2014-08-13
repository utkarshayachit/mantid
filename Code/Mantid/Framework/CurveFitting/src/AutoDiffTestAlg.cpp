#include "MantidCurveFitting/AutoDiffTestAlg.h"
#include "MantidCurveFitting/GaussianNumDiff.h"
#include "MantidCurveFitting/GaussianAutoDiff.h"
#include "MantidCurveFitting/GaussianHandCoded.h"
#include "MantidAPI/FunctionDomain1D.h"
#include "MantidAPI/FunctionValues.h"
#include "MantidCurveFitting/Jacobian.h"
#include "MantidAPI/CompositeFunction.h"
#include "MantidKernel/Timer.h"
#include "MantidKernel/Logger.h"

namespace Mantid
{
namespace CurveFitting
{

using Mantid::Kernel::Direction;
using Mantid::API::WorkspaceProperty;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(AutoDiffTestAlg)

using namespace API;

//----------------------------------------------------------------------------------------------
/** Constructor
   */
AutoDiffTestAlg::AutoDiffTestAlg()
{
}

//----------------------------------------------------------------------------------------------
/** Destructor
   */
AutoDiffTestAlg::~AutoDiffTestAlg()
{
}


//----------------------------------------------------------------------------------------------

const std::string AutoDiffTestAlg::name() const { return "AutoDiffTestAlg"; }

/// Algorithm's version for identification. @see Algorithm::version
int AutoDiffTestAlg::version() const { return 1;}

/// Algorithm's category for identification. @see Algorithm::category
const std::string AutoDiffTestAlg::category() const { return "Test"; }

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string AutoDiffTestAlg::summary() const { return "Test"; }

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
   */
void AutoDiffTestAlg::init()
{
    declareProperty(new WorkspaceProperty<MatrixWorkspace>("InputWorkspace","",Direction::Input), "Data with 20 gaussian peaks.");
    declareProperty(new WorkspaceProperty<MatrixWorkspace>("OutputWorkspace","",Direction::Output), "Data with 20 gaussian peaks.");
    declareProperty("DerivativeType","adept","How to calculate derivatives.");

}

//----------------------------------------------------------------------------------------------
namespace
{

  // Logger
  Kernel::Logger g_log("AutoDiffTestAlg");

  IFunction_sptr getFunction(const std::string& type)
  {
    if ( type == "adept" )
    {
      return IFunction_sptr(new GaussianAutoDiff);
    } else if ( type == "num") {
    /*
    API::CompositeFunction_sptr comp(new API::CompositeFunction);
    auto gauss = IFunction_sptr(new GaussianNumDiff);
    gauss->initialize();
    comp->addFunction( gauss );
    comp->setAttributeValue("NumDeriv",true);
    return comp;
    */
    return IFunction_sptr(new GaussianNumDiff);
    }

    return IFunction_sptr(new GaussianHandCoded);
  }
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
   */
void AutoDiffTestAlg::exec()
{
    MatrixWorkspace_sptr fitData = getProperty("InputWorkspace");
    m_derType = getPropertyValue("DerivativeType");

    // reference positions
    std::vector<double> peakPos;
    peakPos.push_back(0.827127667318377 * 1e3);
    peakPos.push_back(0.446507516865451 * 1e3);
    peakPos.push_back(3.212031089462408 * 1e3);
    peakPos.push_back(0.339620248883351 * 1e3);
    peakPos.push_back(0.853734126295672 * 1e3);
    peakPos.push_back(3.533866873710056 * 1e3);
    peakPos.push_back(2.675258995780313 * 1e3);
    peakPos.push_back(2.804395617510907 * 1e3);
    peakPos.push_back(4.080720843054712 * 1e3);
    peakPos.push_back(4.009692942511339 * 1e3);
    peakPos.push_back(2.608962017782574 * 1e3);
    peakPos.push_back(1.527833877767375 * 1e3);
    peakPos.push_back(0.774391717294182 * 1e3);
    peakPos.push_back(4.455669248757245 * 1e3);
    peakPos.push_back(4.857387157338189 * 1e3);
    peakPos.push_back(4.724126301815355 * 1e3);
    peakPos.push_back(3.263015036059334 * 1e3);
    peakPos.push_back(0.634832729798558 * 1e3);
    peakPos.push_back(0.558598951514288 * 1e3);
    peakPos.push_back(1.075764493763790 * 1e3);

    std::vector<double> peakHeight;
    peakHeight.push_back(2.245209864353441 * 1e2);
    peakHeight.push_back(1.510637181301955 * 1e2);
    peakHeight.push_back(0.976472331098110 * 1e2);
    peakHeight.push_back(2.155967444583149 * 1e2);
    peakHeight.push_back(2.096849388602564 * 1e2);
    peakHeight.push_back(2.001412921426884 * 1e2);
    peakHeight.push_back(0.813583834136194 * 1e2);
    peakHeight.push_back(0.870991001783003 * 1e2);
    peakHeight.push_back(1.859893820857897 * 1e2);
    peakHeight.push_back(1.702790456488836 * 1e2);
    peakHeight.push_back(1.285023552530342 * 1e2);
    peakHeight.push_back(2.235335496958465 * 1e2);
    peakHeight.push_back(1.252322988661303 * 1e2);
    peakHeight.push_back(2.289811034304849 * 1e2);
    peakHeight.push_back(1.281157243695577 * 1e2);
    peakHeight.push_back(1.306906792985985 * 1e2);
    peakHeight.push_back(1.837545751838062 * 1e2);
    peakHeight.push_back(1.076168964141773 * 1e2);
    peakHeight.push_back(1.206518519074316 * 1e2);
    peakHeight.push_back(0.565707936300742 * 1e2);

    std::vector<double> sigma;
    sigma.push_back(22.452098643534413);
    sigma.push_back(15.106371813019551);
    sigma.push_back(9.764723310981100);
    sigma.push_back(21.559674445831490);
    sigma.push_back(20.968493886025641);
    sigma.push_back(20.014129214268841);
    sigma.push_back(8.135838341361936);
    sigma.push_back(8.709910017830035);
    sigma.push_back(18.598938208578964);
    sigma.push_back(17.027904564888356);
    sigma.push_back(12.850235525303422);
    sigma.push_back(22.353354969584650);
    sigma.push_back(12.523229886613029);
    sigma.push_back(22.898110343048490);
    sigma.push_back(12.811572436955773);
    sigma.push_back(13.069067929859852);
    sigma.push_back(18.375457518380621);
    sigma.push_back(10.761689641417730);
    sigma.push_back(12.065185190743160);
    sigma.push_back(5.657079363007425);

    // make composite function
    /*
    CompositeFunction_sptr bigFunction(new CompositeFunction);
    for(size_t i = 0; i < 20; ++i) {
        IFunction_sptr g = getFunction(m_derType);
        g->initialize();
        g->setParameter(0, peakHeight[i] * 1.1);
        g->setParameter(2, sigma[i] * 1.15);
        g->setParameter(1, peakPos[i]);

        //g->addTies("Height = 10*Sigma");

        bigFunction->addFunction(g);
    }
    */

    Kernel::Timer timer;

    for(size_t i = 0; i < 10; ++i) {
        IFunction_sptr g = getFunction(m_derType);
        g->initialize();
        g->setParameter(0, 4.0); // h
        g->setParameter(1, 0.0); // x0
        g->setParameter(2, 3.5); // s

        IAlgorithm_sptr fitAlgorithm = createChildAlgorithm("Fit", -1, -1, false);
        fitAlgorithm->setProperty("CreateOutput", true);
        fitAlgorithm->setProperty("Output", "FitPeaks1D");
        fitAlgorithm->setProperty("CalcErrors", true);
        fitAlgorithm->setProperty("Function", g);
        fitAlgorithm->setProperty("InputWorkspace", fitData);
        fitAlgorithm->setProperty("WorkspaceIndex", 0);

        fitAlgorithm->execute();
    }

    g_log.warning() << "Fit took " << timer.elapsed() << " seconds to complete" << std::endl;

    IFunction_sptr g = getFunction(m_derType);
    g->initialize();
    g->setParameter(0, 4.0); // h
    g->setParameter(1, 0.0); // x0
    g->setParameter(2, 3.5); // s

    FunctionDomain1DVector x(fitData->readX(0));
    FunctionValues y(x);
    CurveFitting::Jacobian J(500, 3);

    g->function(x, y);
    g->functionDeriv(x, J);

    MatrixWorkspace_sptr t = WorkspaceFactory::Instance().create(fitData);

    MantidVec &yd = t->dataY(0);
    const MantidVec &yr = fitData->readY(0);
    for(size_t i = 0; i < 500; ++i) {
        yd[i] = yr[i] - y[i];
    }

    MantidVec &dfdx0 = t->dataY(1);
    const MantidVec &dfdx0r = fitData->readY(1);
    for(size_t i = 0; i < 500; ++i) {
        dfdx0[i] = dfdx0r[i] - J.get(i, 1);
    }

    MantidVec &dfdh = t->dataY(2);
    const MantidVec &dfdhr = fitData->readY(2);
    for(size_t i = 0; i < 500; ++i) {
        dfdh[i] = dfdhr[i] - J.get(i, 0);
    }

    MantidVec &dfds = t->dataY(3);
    const MantidVec &dfdsr = fitData->readY(3);
    for(size_t i = 0; i < 500; ++i) {
        dfds[i] = dfdsr[i] - J.get(i, 2);
    }

    for(size_t i = 0; i < 4; ++i) {
        MantidVec &xd = t->dataX(i);
        xd = fitData->readX(i);
    }

    // test accuracy
    setProperty("OutputWorkspace", t);
}



} // namespace CurveFitting
} // namespace Mantid
