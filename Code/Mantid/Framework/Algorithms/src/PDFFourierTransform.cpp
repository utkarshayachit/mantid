/*WIKI*

The algorithm PDFFourierTransform transforms <math>S(Q)</math>, <math>S(Q)-1</math>, or <math>Q[S(Q)-1]</math>
(as a fuction of MomentumTransfer or dSpacing) to a PDF (pair distribution function) as described below.

The input Workspace can have the unit in d-space of Q-space.  The algorithm itself is able to identify
the unit.  The allowed unit are MomentumTransfer and d-spacing.

'''Note:''' All other forms are calculated by transforming <math>G(r)</math>.

==== Output Options ====

=====G(r)=====

<math> G(r) = 4\pi r[\rho(r)-\rho_0] = \frac{2}{\pi} \int_{0}^{\infty} Q[S(Q)-1]sin(Qr)dQ </math>

and in this algorithm, it is implemented as

<math> G(r) =  \frac{2}{\pi} \sum_{Q_{min}}^{Q_{max}} Q[S(Q)-1]sin(Qr)\Delta Q </math>

=====g(r)=====

<math>G(r) = 4 \pi \rho_0 r [g(r)-1]</math>

transforms to

<math>g(r) = \frac{G(r)}{4 \pi \rho_0 r} + 1</math>

=====RDF(r)=====

<math>RDF(r) = 4 \pi \rho_0 r^2 g(r)</math>

transforms to

<math>RDF(r) = r G(r) + 4 \pi \rho_0 r^2</math>

*WIKI*/

#include "MantidAlgorithms/PDFFourierTransform.h"
#include "MantidKernel/System.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/PhysicalConstants.h"
#include "MantidKernel/UnitFactory.h"
#include <sstream>
#include <math.h>

using namespace Mantid::API;
using namespace Mantid::Kernel;

using namespace std;

namespace Mantid 
{
namespace Algorithms
{

  using std::string;

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM( PDFFourierTransform)

  using namespace Mantid::Kernel;
  using namespace Mantid::API;

  namespace { // anonymous namespace
    /// Crystalline PDF
    const string BIG_G_OF_R("G(r)");
    /// Liquids PDF
    const string LITTLE_G_OF_R("g(r)");
    /// Radial distribution function
    const string RDF_OF_R("RDF(r)");

    /// Normalized intensity
    const string S_OF_Q("S(Q)");
    /// Asymptotes to zero
    const string S_OF_Q_MINUS_ONE("S(Q)-1");
    /// Kernel of the Fourier transform
    const string Q_S_OF_Q_MINUS_ONE("Q[S(Q)-1]");
  }

  //----------------------------------------------------------------------------------------------
  /** Constructor
    */
  PDFFourierTransform::PDFFourierTransform()
  {
  }

  //----------------------------------------------------------------------------------------------
  /** Destructor
    */
  PDFFourierTransform::~PDFFourierTransform()
  {
  }

  const std::string PDFFourierTransform::name() const
  {
    return "PDFFourierTransform";
  }

  int PDFFourierTransform::version() const
  {
    return 1;
  }

  const std::string PDFFourierTransform::category() const
  {
    return "Diffraction";
  }

  //----------------------------------------------------------------------------------------------
  /** Sets documentation strings for this algorithm
    */
  void PDFFourierTransform::initDocs()
  {
    setWikiSummary("PDFFourierTransform() does Fourier transform from S(Q) to G(r), "
                   "which is paired distribution function (PDF). "
                   "G(r) will be stored in another named workspace.");
    setOptionalMessage("Fourier transform from S(Q) to G(r), which is paired distribution function (PDF). "
                       "G(r) will be stored in another named workspace.");
  }

  //--------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
    */
  void PDFFourierTransform::init()
  {
    auto uv = boost::make_shared<API::WorkspaceUnitValidator>("MomentumTransfer");

    declareProperty(new WorkspaceProperty<> ("InputWorkspace", "", Direction::Input, uv),
                    S_OF_Q + ", " + S_OF_Q_MINUS_ONE + ", or " + Q_S_OF_Q_MINUS_ONE);
    declareProperty(new WorkspaceProperty<> ("OutputWorkspace", "", Direction::Output),
                      "Result paired-distribution function");

    // Set up input data type
    std::vector<std::string> inputTypes;
    inputTypes.push_back(S_OF_Q);
    inputTypes.push_back(S_OF_Q_MINUS_ONE);
    inputTypes.push_back(Q_S_OF_Q_MINUS_ONE);
    declareProperty("InputSofQType", S_OF_Q, boost::make_shared<StringListValidator>(inputTypes),
                    "To identify whether input function");

    auto mustBePositive = boost::make_shared<BoundedValidator<double> >();
    mustBePositive->setLower(0.0);

    declareProperty("Qmin", EMPTY_DBL(), mustBePositive,
                    "Minimum Q in S(Q) to calculate in Fourier transform (optional).");
    declareProperty("Qmax", EMPTY_DBL(), mustBePositive,
                    "Maximum Q in S(Q) to calculate in Fourier transform. (optional)");

    // Set up output data type
    std::vector<std::string> outputTypes;
    outputTypes.push_back(BIG_G_OF_R);
    outputTypes.push_back(LITTLE_G_OF_R);
    outputTypes.push_back(RDF_OF_R);
    declareProperty("PDFType", BIG_G_OF_R, boost::make_shared<StringListValidator>(outputTypes),
                    "Type of output PDF including G(r)");


    declareProperty("DeltaR", EMPTY_DBL(), mustBePositive,
                    "Step size of r of G(r) to calculate.  Default = <math>\\frac{\\pi}{Q_{max}}</math>.");
    declareProperty("Rmax", 20., mustBePositive, "Maximum r for G(r) to calculate.");
    declareProperty("rho0", EMPTY_DBL(), mustBePositive,
                    "Average number density used for g(r) and RDF(r) conversions (optional)");

    string recipGroup("Reciprocal Space");
    setPropertyGroup("InputSofQType", recipGroup);
    setPropertyGroup("Qmin", recipGroup);
    setPropertyGroup("Qmax", recipGroup);

    string realGroup("Real Space");
    setPropertyGroup("PDFType", realGroup);
    setPropertyGroup("DeltaR", realGroup);
    setPropertyGroup("Rmax", realGroup);
    setPropertyGroup("rho0", realGroup);
  }

  //----------------------------------------------------------------------------------------------
  /** Validate inputs
    */
  std::map<string, string> PDFFourierTransform::validateInputs()
  {
    std::map<string, string> result;

    double Qmin = getProperty("Qmin");
    double Qmax = getProperty("Qmax");
    if ((!isEmpty(Qmin)) && (!isEmpty(Qmax)))
      if (Qmax <= Qmin)
        result["Qmax"] = "Must be greater than Qmin";

    API::MatrixWorkspace_const_sptr inputWS =  getProperty("InputWorkspace");
    if (inputWS->getNumberHistograms() != 1)
    {
      result["InputWorkspace"] = "Input workspace must have only one spectrum";
    }

    return result;
  }

  //----------------------------------------------------------------------------------------------
  /** Process input properties
    */
  void PDFFourierTransform::processProperties()
  {
    // Process input properties for Q and S(Q)
    m_dataWS =  getProperty("InputWorkspace");
    m_wsIndex = 0;

    // Convert unit if necessary
    const std::string inputXunit = m_dataWS->getAxis(0)->unit()->unitID();
    if (inputXunit == "MomentumTransfer")
    {
      // nothing to do
    }
    else if (inputXunit == "dSpacing")
    {
      IAlgorithm_sptr converter = createChildAlgorithm("ConvertUnits", 0.0, 0.1, true);
      converter->initialize();
      converter->setProperty("InputWorkspace", m_dataWS);
      converter->setPropertyValue("OutputWorkspace", "TempSofQWS");
      converter->setProperty("Target", "MomentumTransfer");
      converter->setProperty("EMode0", "Direct");

      converter->executeAsChildAlg();
      if (!converter->isExecuted())
      {
        throw runtime_error("Unable to convert unites to momentum transfer (Q)");
      }

      m_dataWS = converter->getProperty("OutputWorkspace");
    }
    else
    {
      std::stringstream msg;
      msg << "Input data x-axis with unit \"" << inputXunit
          << "\" is not supported (use \"MomentumTransfer\" or \"dSpacing\")";
      throw runtime_error(msg.str());
    }

    const MantidVec& vecQ = m_dataWS->readX(m_wsIndex);

    // determine Q-range
    qmin = getProperty("Qmin");
    if (isEmpty(qmin))
    {
      qmin = vecQ.front();
    }
    else if (qmin < vecQ.front())
    {
      g_log.debug() << "Specified Qmin < range of data. Adjusting to data range.\n";
      qmin = vecQ.front();
    }

    qmax = getProperty("Qmax");
    if (isEmpty(qmax))
    {
      qmax = vecQ.back();
    }
    else if (qmax > vecQ.back())
    {
      g_log.debug() << "Specified Qmax > range of data. Adjusting to data range.\n";
    }

    g_log.information() << "User specified Qmin = " << qmin << "Angstroms^-1 and Qmax = "
                  << qmax << "Angstroms^-1\n";

    // get pointers for the data range
    {
      // keep variable scope small
      auto qmin_ptr = std::upper_bound(vecQ.begin(), vecQ.end(), qmin);
      qmin_index = std::distance(vecQ.begin(), qmin_ptr);
      if (qmin_index == 0)
        qmin_index += 1; // so there doesn't have to be a check below
      auto qmax_ptr = std::lower_bound(vecQ.begin(), vecQ.end(), qmax);
      qmax_index = std::distance(vecQ.begin(), qmax_ptr);
    }
    size_t qmi_out = qmax_index;
    if (qmi_out == vecQ.size())
      qmi_out--; // prevent unit test problem under windows (and probably other hardly identified problem)
    g_log.information() << "Adjusting to data: Qmin = " << vecQ[qmin_index]
                        << " Qmax = " << vecQ[qmi_out] << "\n";

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** create the output workspace
   */
  void PDFFourierTransform::determineRrange()
  {
    // determine r axis for result
    const double rmax = getProperty("RMax");
    double rdelta = getProperty("DeltaR");
    if (isEmpty(rdelta))
      rdelta = M_PI/qmax;
    size_t sizer = static_cast<size_t>(rmax/rdelta);

    outputWS
        = WorkspaceFactory::Instance().create("Workspace2D", 1, sizer, sizer);

    // X-axis label
    outputWS->getAxis(0)->unit() = UnitFactory::Instance().create("Label");
    Unit_sptr unit = outputWS->getAxis(0)->unit();
    boost::shared_ptr<Units::Label> label = boost::dynamic_pointer_cast<Units::Label>(unit);
    label->setLabel("AtomicDistance", "Angstrom");
    outputWS->setYUnitLabel("PDF");
    MantidVec& outputR = outputWS->dataX(0);
    for (size_t i = 0; i < sizer; i++)
    {
      outputR[i] = rdelta * static_cast<double>(1 + i);
    }
    g_log.information() << "Using rmin = " << outputR.front() << "Angstroms and rmax = "
                        << outputR.back() << "Angstroms\n";

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Convert to Q[S(Q)-1]
   */
  void PDFFourierTransform::convertToQSm1()
  {
    // Init
    const MantidVec& vecY = m_dataWS->readY(m_wsIndex);
    const MantidVec& vecE = m_dataWS->readE(m_wsIndex);
    m_vecQSm1.resize(vecY.size(), 0.);
    m_vecErrQSm1.resize(vecY.size(), 0.);

    // Input type
    string soqType = getProperty("InputSofQType");

    if (soqType == Q_S_OF_Q_MINUS_ONE)
    {
      // Input is Q[S(Q)-1]: duplicate the vector
      ::copy(vecY.begin(), vecY.end(), m_vecQSm1.begin());
      ::copy(vecE.begin(), vecE.end(), m_vecErrQSm1.begin());
    }
    else if (soqType == S_OF_Q || soqType == S_OF_Q_MINUS_ONE)
    {
      // Input is S(Q) or S(Q)-1
      if (soqType == S_OF_Q)
      {
        // Input is S(Q): calculate S(Q)-1. There is no error propagation for subtracting one
        g_log.debug("Subtracting one from all values.");
        ::transform(vecY.begin(), vecY.end(), m_vecQSm1.begin(), ::bind2nd(::minus<double>(), 1.));
        soqType = S_OF_Q_MINUS_ONE;
      }

      // At this point, vecQSm1 = S(Q)-1
      if (soqType == S_OF_Q_MINUS_ONE)
      {
        // Calculate Q[S(Q)-1] from S(Q)-1
        g_log.debug("Multiplying all values by Q.");

        // Do [S(Q)-1] * Q
        const MantidVec& vecQ = m_dataWS->readX(m_wsIndex);
        ::transform(m_vecQSm1.begin(), m_vecQSm1.end(), vecQ.begin(), m_vecQSm1.begin(),
                    ::multiplies<double>());

        // Error propagation: vecE now is error for S(Q) (same to S(Q)-1)
        const MantidVec& inputDQ = m_dataWS->readDx(m_wsIndex);
        if (inputDQ.size() >= vecE.size())
        {
          for (size_t i = 0; i < vecE.size(); ++i)
          {
            m_vecErrQSm1[i] = vecQ[i] * vecE[i] + m_vecQSm1[i] * inputDQ[i];
          }
        }
        else
        {
          for (size_t i = 0; i < vecE.size(); ++i)
          {
            m_vecErrQSm1[i] = vecQ[i] * vecE[i];
          }
        }

        // soqType = Q_S_OF_Q_MINUS_ONE;
      }
    }
    else
    {
      // Exception check!
      std::stringstream msg;
      msg << "Do not understand InputSofQType = " << soqType;
      throw std::runtime_error(msg.str());
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Do Foureir transform to Q[S(Q)-1]
    */
  void PDFFourierTransform::doFourierTransform()
  {
    // do the math
    const MantidVec& vecR = outputWS->readX(0);
    MantidVec& vecGofR = outputWS->dataY(0);
    MantidVec& vecErrG = outputWS->dataE(0);
    const MantidVec& vecQ = m_dataWS->readX(m_wsIndex);
    size_t sizer = vecR.size();
    for (size_t r_index = 0; r_index < sizer; r_index ++)
    {
      const double r = vecR[r_index];
      double fs = 0;
      double error = 0;
      for (size_t q_index = qmin_index; q_index < qmax_index; q_index++)
      {
        double q = vecQ[q_index];
        double deltaq = vecQ[q_index] - vecQ[q_index - 1];
        double sinus  = sin(q * r) * deltaq;
        fs    += sinus * m_vecQSm1[q_index];
        error += q * q * (sinus*m_vecErrQSm1[q_index]) * (sinus*m_vecErrQSm1[q_index]);
        // g_log.debug() << "q[" << i << "] = " << q << "  dq = " << deltaq << "  S(q) =" << s;
        // g_log.debug() << "  d(gr) = " << temp << "  gr = " << gr << std::endl;
      }

      // put the information into the output
      vecGofR[r_index] = fs * 2 / M_PI;
      vecErrG[r_index] = error*2/M_PI; //TODO: Wrong!
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Add some constant or multiply factor to the Fourier transformed vector
    */
  void PDFFourierTransform::postProcess()
  {
    // convert to the correct form of PDF
    string pdfType = getProperty("PDFType");
    double rho0 = getProperty("rho0");
    if (isEmpty(rho0))
    {
      const Kernel::Material &material = m_dataWS->sample().getMaterial();
      double materialDensity = material.numberDensity();

      if (!isEmpty(materialDensity) && materialDensity > 0)
        rho0 = materialDensity;
      else
        rho0 = 1.;
      // write out that it was reset if the value is coming into play
      if (pdfType == LITTLE_G_OF_R || pdfType == RDF_OF_R)
        g_log.information() << "Using rho0 = " << rho0 << "\n";
    }

    // Add and multiply constants to data
    const MantidVec& vecR = outputWS->readX(0);
    MantidVec& vecGofR = outputWS->dataY(0);
    MantidVec& vecErrG = outputWS->dataE(0);

    if (pdfType == BIG_G_OF_R)
    {
      // nothing to do
    }
    else if (pdfType == LITTLE_G_OF_R)
    {
      const double factor = 1./(4.*M_PI*rho0);
      for (size_t i = 0; i < vecGofR.size(); ++i)
      {
        // error propogation - assuming uncertainty in r = 0
        vecErrG[i] = vecErrG[i] / vecR[i];
        // transform the data
        vecGofR[i] = 1. + factor*vecGofR[i]/vecR[i];
      }
    }
    else if (pdfType == RDF_OF_R)
    {
      const double factor = 4.*M_PI*rho0;
      for (size_t i = 0; i < vecGofR.size(); ++i)
      {
        // error propogation - assuming uncertainty in r = 0
        vecErrG[i] = vecErrG[i] * vecR[i];
        // transform the data
        vecGofR[i] = vecR[i] * vecGofR[i] + factor * vecR[i] * vecR[i];
      }
    }
    else
    {
      // should never get here
      std::stringstream msg;
      msg << "Do not understand PDFType = " << pdfType;
      throw std::runtime_error(msg.str());
    }
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
    */
  void PDFFourierTransform::exec()
  {
    // Process input properties (esp. Q)
    processProperties();

    // Determine the output R range
    determineRrange();

    // convert from histogram to density
    if (!m_dataWS->isHistogramData())
    {
      g_log.warning() << "This algorithm has not been tested on density data (only on histograms)\n";
      /* Don't do anything for now
        double deltaQ;
        for (size_t i = 0; i < inputFOfQ.size(); ++i)
        {
        deltaQ = inputQ[i+1] -inputQ[i];
        inputFOfQ[i] = inputFOfQ[i]*deltaQ;
        inputDfOfQ[i] = inputDfOfQ[i]*deltaQ; // TODO feels wrong
        inputQ[i] += -.5*deltaQ;
        inputDQ[i] += .5*(inputDQ[i] + inputDQ[i+1]); // TODO running average
        }
        inputQ.push_back(inputQ.back()+deltaQ);
        inputDQ.push_back(inputDQ.back()); // copy last value
        */
    }

    // convert to Q[S(Q)-1]
    convertToQSm1();

    doFourierTransform();

    postProcess();

    // set property
    setProperty("OutputWorkspace", outputWS);
  }

} // namespace Mantid
} // namespace Algorithms

