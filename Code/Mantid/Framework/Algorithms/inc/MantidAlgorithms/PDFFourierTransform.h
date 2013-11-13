#ifndef MANTID_ALGORITHMS_PDFFourierTransform_H_
#define MANTID_ALGORITHMS_PDFFourierTransform_H_
    
#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h" 

namespace Mantid
{
namespace Algorithms
{

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

    /// Do Fourier transform
    void doFourierTransform();

    void postProcess();

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
  };


} // namespace Mantid
} // namespace Algorithms

#endif  /* MANTID_ALGORITHMS_PDFFourierTransform_H_ */
