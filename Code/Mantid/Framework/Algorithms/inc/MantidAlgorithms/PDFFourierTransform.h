#ifndef MANTID_ALGORITHMS_PDFFourierTransform_H_
#define MANTID_ALGORITHMS_PDFFourierTransform_H_
    
#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h" 

namespace Mantid
{
namespace Algorithms
{
  // inline static double lorchDamp(double, double);

  /// Zeroth order Bessell function  Consider the case x = 0
  inline static double j0(double x)
  {
    if (fabs(x) > 1.0E-20)
    {
      return sin(x)/x;
    }
    return 1.0;
  }

  /// Lorch damping function
  struct lorchDampFunction
  {
    inline double operator()(double q, double qmax)
    {
      return j0(M_PI*q/qmax);
    }
  };

  struct noDampFunction
  {
    double operator()(double, double) const { return 1.; }
  };

  /** PDFFourierTransform : TODO: DESCRIPTION
   */
  class DLLExport PDFFourierTransform  : public API::Algorithm
  {
  public:
    PDFFourierTransform();
    ~PDFFourierTransform();
    
    /// Algorithm's name for identification 
    virtual const std::string name() const;
    /// Algorithm's version for identification 
    virtual int version() const;
    /// Algorithm's category for identification
    virtual const std::string category() const;  // category better be in diffraction than general
    /// @copydoc Algorithm::validateInputs()
    virtual std::map<std::string, std::string> validateInputs();

  private:
    /// Sets documentation strings for this algorithm
    virtual void initDocs();
    /// Initialise the properties
    void init();
    /// Run the algorithm
    void exec();

    /// Process input properties
    void processProperties();

    /// Create output workspace
    void determineRrange();

    /// Convert inpt to Q[S(Q)-1]
    void convertToQSm1();
    /// Convert input to S(Q)
    void convertToSofQ();

    /// Do Fourier transform
    void doFourierTransform();
    /// Do Foureir transform to S(Q) for g(r)
    void doFourierTransformSmallGofR(boost::function<double(double, double)> dampfunction);

    void postProcessBigGofR();
    void postProcessRDFofR();
    void postProcessSmallGofR();

    /// Input workspace
    API::MatrixWorkspace_const_sptr m_dataWS;
    /// Input data index
    size_t m_wsIndex;
    /// Output workspace
    API::MatrixWorkspace_sptr outputWS;

    /// Q-min
    double qmin;
    /// Q-max
    double qmax;
    size_t qmin_index;
    size_t qmax_index;

    /// Vector of Q[S(Q)-1]
    std::vector<double> m_vecQSm1;
    /// Vector of error of Q[S(Q)-1]
    std::vector<double> m_vecErrQSm1;
    /// Vector of S(Q)
    std::vector<double> m_vecSofQ;
    /// Vector of error of S(Q)
    std::vector<double> m_vecErrSofQ;
    /// Type of PDF
    std::string m_pdfType;

    boost::function<double(double, double)> m_dampingFunction;

  };


} // namespace Mantid
} // namespace Algorithms

#endif  /* MANTID_ALGORITHMS_PDFFourierTransform_H_ */
