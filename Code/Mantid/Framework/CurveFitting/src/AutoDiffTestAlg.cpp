#include "MantidCurveFitting/AutoDiffTestAlg.h"
#include "MantidCurveFitting/GaussianNumDiff.h"
#include "MantidCurveFitting/GaussianAutoDiff.h"
#include "MantidCurveFitting/GaussianHandCoded.h"
#include "MantidCurveFitting/LorentzianFamily.h"
#include "MantidAPI/FunctionDomain1D.h"
#include "MantidAPI/FunctionValues.h"
#include "MantidCurveFitting/Jacobian.h"
#include "MantidAPI/CompositeFunction.h"
#include "MantidKernel/Timer.h"
#include "MantidKernel/Logger.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

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
    declareProperty("GaussianParameters", "", "Gaussian parameters (h, x0, s)");
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
      return IFunction_sptr(new Lorentzians::LorentzianAutoDiff);
      //return IFunction_sptr(new GaussianAutoDiff);
    } else if ( type == "num") {
      return IFunction_sptr(new Lorentzians::LorentzianNumDiff);
      //return IFunction_sptr(new GaussianNumDiff);
    }
    return IFunction_sptr(new Lorentzians::LorentzianHandCoded);
    //return IFunction_sptr(new GaussianHandCoded);
  }
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
   */
void AutoDiffTestAlg::exec()
{
    MatrixWorkspace_sptr fitData = getProperty("InputWorkspace");
    m_derType = getPropertyValue("DerivativeType");

    g_log.warning() << "Using " << m_derType << " derivatives." << std::endl;

    std::string parameterString = getProperty("GaussianParameters");
    std::vector<double> paramValues = parameterValues(parameterString);

    if(fitData->getNumberHistograms() > 4) {
        IFunction_sptr f = getFunction(m_derType);
        f->initialize();

        for(size_t i = 0; i < paramValues.size(); ++i) {
            f->setParameter(i, paramValues[i] - 0.001 * paramValues[i]);
        }

        IAlgorithm_sptr fitAlgorithm = createChildAlgorithm("Fit", -1, -1, true);
        fitAlgorithm->setProperty("CreateOutput", true);
        fitAlgorithm->setProperty("Output", "FitPeaks1D");
        fitAlgorithm->setProperty("CalcErrors", true);
        fitAlgorithm->setProperty("Function", f);
        fitAlgorithm->setProperty("InputWorkspace", fitData);
        fitAlgorithm->setProperty("WorkspaceIndex", 4);

        fitAlgorithm->execute();

        g_log.notice() << "Fit results:" << std::endl;
        for(size_t i = 0; i < paramValues.size(); ++i) {
            g_log.notice() << "  " << f->parameterName(i) << " = " << f->getParameter(i) << " (" << f->getError(i) << ")" << std::endl;
        }
    }

    IFunction_sptr g = getFunction(m_derType);
    g->initialize();

    for(size_t i = 0; i < paramValues.size(); ++i) {
        g->setParameter(i, paramValues[i]);
    }

    FunctionDomain1DVector x(fitData->readX(0));
    FunctionValues y(x);
    CurveFitting::Jacobian J(x.size(), 3);

    Kernel::Timer timerF;

    for(size_t i = 0; i < 10000; ++i) {
        g->function(x, y);
    }

    g_log.warning() << "Calculating function took " << timerF.elapsed() / 10000 << " seconds to complete." << std::endl;


    Kernel::Timer timerDf;

    for(size_t i = 0; i < 10000; ++i) {
        g->functionDeriv(x, J);
    }

    g_log.warning() << "Calculating derivatives took " << timerDf.elapsed() / 10000 << " seconds to complete." << std::endl;


    MatrixWorkspace_sptr t = WorkspaceFactory::Instance().create(fitData);

    MantidVec &yd = t->dataY(0);
    const MantidVec &yr = fitData->readY(0);
    for(size_t i = 0; i < x.size(); ++i) {
        yd[i] = 1.0 - y[i] / yr[i];

        if(std::isinf(yd[i])) {
            yd[i] = 0.0;
        }
    }

    MantidVec &dfdx0 = t->dataY(1);
    const MantidVec &dfdx0r = fitData->readY(1);
    for(size_t i = 0; i < x.size(); ++i) {
        dfdx0[i] = 1.0 - J.get(i, 1) / dfdx0r[i];

        if(std::isinf(dfdx0[i])) {
            dfdx0[i] = 0.0;
        }
    }

    MantidVec &dfdh = t->dataY(2);
    const MantidVec &dfdhr = fitData->readY(2);
    for(size_t i = 0; i < x.size(); ++i) {
        dfdh[i] = 1.0 - J.get(i, 0) / dfdhr[i];

        if(std::isinf(dfdh[i])) {
            dfdh[i] = 0.0;
        }
    }

    MantidVec &dfds = t->dataY(3);
    const MantidVec &dfdsr = fitData->readY(3);
    for(size_t i = 0; i < x.size(); ++i) {
        dfds[i] = 1.0 - J.get(i, 2) / dfdsr[i];

        if(std::isinf(dfds[i])) {
            dfds[i] = 0.0;
        }
    }

    for(size_t i = 0; i < fitData->getNumberHistograms(); ++i) {
        MantidVec &xd = t->dataX(i);
        xd = fitData->readX(i);
    }

    // test accuracy
    setProperty("OutputWorkspace", t);
}

std::vector<double> AutoDiffTestAlg::parameterValues(const std::string &parameterString) const
{
    std::vector<std::string> strings;
    boost::split(strings, parameterString, boost::is_any_of(", "));

    std::vector<double> values(strings.size());

    std::cout << std::setprecision(17);
    for(size_t i = 0; i < values.size(); ++i) {
        values[i] = boost::lexical_cast<double>(strings[i]);
    }

    return values;
}



} // namespace CurveFitting
} // namespace Mantid
