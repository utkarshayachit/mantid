/*WIKI*
A back-to-back exponential convoluted with pseudo-voigt peakshape function is defined as: 

:<math>F(X) = I\cdot\Omega(X) </math>

where

:<math>\Omega(X) = (1-\eta)N\{e^uerfc(y)+e^verfc(z)\} - \frac{2N\eta}{\pi}\{\Im[e^pE_1(p)]+\Im[e^qE_1(q)]\} </math>

----

:<math> N = \frac{A\cdot B}{2(A+B)} </math>

----
:<math> H_G = \sqrt{8 \cdot \log(2)}\cdot S;
:<math> H^5 = H_G^5 +2.69269H_G^4\cdot\gamma +2.42843H_G^3\gamma^2 +4.47163H_G^2\gamma^3 + 0.07842H_G\gamma^4 +\gamma^5 </math>
:<math> \eta = 1.36603\cdot \frac{\gamma}{H} - 0.47719 \cdot (\frac{\gamma}{H})^2 + 0.11116 \cdot (\frac{\gamma}{H})^3 </math>

----

:<math> u = \frac{1}{2}A(A\cdot S^2+2(X-X_0)) </math>
:<math> y = \frac{A\cdot S^2+(X-X_0)}{\sqrt{2}S} </math>
:<math> v = \frac{1}{2}B(B\cdot S^2 - 2(X-X_0))   </math>
:<math> z = \frac{B\cdot S^2-(X-X_0)}{\sqrt{2}S} </math>

----

:<math> p = A(X-X_0) + \frac{iA\cdot H}{2}  </math>
:<math> q = -B(X-X_0) + \frac{iB\cdot H}{2}   </math>

----
:<math>erfc(x) = 1-erf(x) = 1-\frac{2}{\sqrt{\pi}}\int_0^xe^{-u^2}du = \frac{2}{\sqrt{\pi}}\int_x^{\infty}e^{-u^2}du </math>

:<math> E_1(z) = \int_z^{\infty}\frac{e^{-t}}{t}dt </math>


This peakshape function represent the convolution of back-to-back exponential and a pseudo-voigt function and is designed to
be used for the data analysis of time-of-flight neutron powder diffraction data, see Ref. 1.

The parameters <math>A</math> and <math>B</math> represent the absolute value of the exponential rise and decay constants (modelling the neutron pulse coming from the moderator)
and <math>S</math> and <math>\gamma</math> represent the standard deviation of the gaussian and Lorentzian respectively. The parameter <math>X_0</math> is the location of the peak; more specifically it represent 
the point where the exponentially modelled neutron pulse goes from being exponentially rising to exponentially decaying. <math>I</math> is the integrated intensity.

If the lorentzian <math>\gamma</math> goes to zero, then this peak profile is same as [[BackToBackExponential|Back to back exponential convoluted with Gaussian]].

For information about how to convert Fullprof back-to-back exponential convoluted pseudo-voigt parameters into those used for this function see [[CreateBackToBackParameters]].

References

1. R.B. Von Dreele, J.D. Jorgensen & C.G. Windsor, J. Appl. Cryst., 15, 581-589, 1982

The figure below illustrate this peakshape function fitted to a TOF peak when Lorentzian is equal to zero:

[[Image:BackToBackExponentialWithConstBackground.png]]

== Properties ==

''Note the initial default guesses for in particular A and B are only based on fitting a couple of peaks in a dataset collected on the ISIS's HRPD instrument.''
 *WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidCurveFitting/BackToBackExponentialPV.h"
#include "MantidAPI/FunctionFactory.h"

#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_multifit_nlin.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <cmath>
#include <limits>

namespace Mantid
{
namespace CurveFitting
{

  using namespace Kernel;
  using namespace API;

  DECLARE_FUNCTION(BackToBackExponentialPV)

  const double PEAKEXTENT = 100;

  const double TWO_OVER_PI = 2./M_PI;
  const double PI = M_PI;

  //----------------------------------------------------------------------------------------------
  /** Initialization
    */
  void BackToBackExponentialPV::init()
  {
    // Do not change the order of these parameters!
    declareParameter("I", 0.0, "integrated intensity of the peak");                          // 0
    declareParameter("A", 1.0, "exponentialPV constant of rising part of neutron pulse");      // 1
    declareParameter("B", 0.05, "exponentialPV constant of decaying part of neutron pulse");   // 2
    declareParameter("X0", 0.0, "peak position");                                            // 3
    declareParameter("S", 1.0, "standard deviation of gaussian part of peakshape function"); // 4
    declareParameter("Gamma", 0.0, "standard deviation of Lorentzian part of the peak function"); // 5
  }

  //----------------------------------------------------------------------------------------------
  /** Get approximate height of the peak: function value at X0.
   */
  double BackToBackExponentialPV::height()const
  {
    double x0 = getParameter(3);
    std::vector<double> vec(1, x0);
    FunctionDomain1DVector domain(vec);
    FunctionValues values(domain);

    function(domain, values);

    return values[0];
  }

  //----------------------------------------------------------------------------------------------
   /** Set new height of the peak. This method does this approximately.
    * @param h :: New value for the height.
    */
   void BackToBackExponentialPV::setHeight(const double h)
   {
     double h0 = height();
     if ( h0 == 0.0 )
     {
       setParameter( 0, 1e-6 );
       h0 = height();
     }
     double area = getParameter( 0 ); // == I
     area *= h / h0;
     if ( area <= 0.0 )
     {
       area = 1e-6;
     }
     if ( boost::math::isnan( area ) || boost::math::isinf( area ) )
     {
       area = std::numeric_limits<double>::max() / 2;
     }
     setParameter( 0, area );
   }
  
   //----------------------------------------------------------------------------------------------
   /** Get approximate peak width.
    */
   double BackToBackExponentialPV::fwhm()const
   {
     // Re-calcualte peak width if any parameter value has been changed since last calculation
     if (m_hasNewParameterValue)
     {
       // FIXME - 1. Think of make m_hasNewParameterValue to m_hasNewSigmaGamma
       // FIXME - 2. Think of moving m_fhwm, m_eta and m_hasNewSigmaGamma into calHandEta()
       const double s = getParameter(4);
       const double gamma = getParameter(5);
       double sigma2 = s*s;
       calHandEta(sigma2, gamma, m_fwhm, m_eta);
       m_hasNewParameterValue = false;
     }

     return m_fwhm;
   }
  
   //----------------------------------------------------------------------------------------------
   /** Set new peak width approximately.
     * @param w :: New value for the width.
     */
   void BackToBackExponentialPV::setFwhm(const double w)
   {
     if (w <= 0.0)
       throw std::runtime_error("Peak width cannot be equal or less than 0. ");
     setParameter("S",w/2.0);
     setParameter("Gamma", 0.);
   }
  
   //----------------------------------------------------------------------------------------------
   /** Calculate function for an array of x values
     */
   void BackToBackExponentialPV::function1D(double* out, const double* xValues, const size_t nData)const
   {
     // Get parameters
     const double I = getParameter(0);
     const double a = getParameter(1);
     const double b = getParameter(2);
     const double x0 = getParameter(3);
     const double s = getParameter(4);
     const double gamma = getParameter(5);
 
     // Find the reasonable extent of the peak ~100 fwhm
     double extent = expWidth();
     if ( s > extent ) extent = s;
     extent *= PEAKEXTENT;
 
     // Prepare constants
#if 1
     // Calcualte H for the peak
     calHandEta(s*s, gamma, m_fwhm, m_eta);
     double sigma_square = s*s;
     double invert_sqrt2_sigma = 1./(sqrt(2.)*s);

     const double N = a*b*0.5/(a+b);
#else
     double s2 = s*s;
     double normFactor = a * b / (a + b) / 2;
     //Needed for IntegratePeaksMD for cylinder profile fitted with b=0
     if (normFactor == 0.0) normFactor = 1.0;
#endif

     // Loop around
     for (size_t i = 0; i < nData; i++)
     {
       double diff=xValues[i]-x0;
       if ( fabs(diff) < extent )
       {
#if 1
         out[i] = I * calOmega(diff, a, b, sigma_square, invert_sqrt2_sigma, m_fwhm, m_eta, N);
#else
         double val = 0.0;
         double arg1 = a/2*(a*s2+2*diff);
         val += exp(  arg1 + gsl_sf_log_erfc((a*s2+diff)/sqrt(2*s2)) ); //prevent overflow
         double arg2 = b/2*(b*s2-2*diff);
         val += exp( arg2 + gsl_sf_log_erfc((b*s2-diff)/sqrt(2*s2)) ); //prevent overflow
         out[i] = I*val*normFactor;        
#endif
       }
       else
       {
         // Outside of extent
         out[i] = 0.0;
       }
     } // ENDFOR loop on all data points

     return;
   }

   //----------------------------------------------------------------------------------------------
   /** Calcualte peak width (H) and Lorenzian weight (eta) for the peak
    */
   void BackToBackExponentialPV::calHandEta(double sigma2, double gamma, double& H, double& eta) const
   {
     // 1. Calculate H
     double H_G = sqrt(8.0 * sigma2 * log(2.0));
     double H_L = gamma;

     double temp1 = std::pow(H_L, 5) + 0.07842*H_G*std::pow(H_L, 4) + 4.47163*std::pow(H_G, 2)*std::pow(H_L, 3) +
         2.42843*std::pow(H_G, 3)*std::pow(H_L, 2) + 2.69269*std::pow(H_G, 4)*H_L + std::pow(H_G, 5);

     H = std::pow(temp1, 0.2);

     // 2. Calculate eta
     double gam_pv = H_L/H;
     eta = 1.36603 * gam_pv - 0.47719 * std::pow(gam_pv, 2) + 0.11116 * std::pow(gam_pv, 3);

     if (eta > 1 || eta < 0)
     {
       g_log.warning() << "Calculated eta = " << eta << " is out of range [0, 1].\n";
     }
     else
     {
       g_log.debug() << "[DBx121] Eta = " << eta << "; Gamma = " << gamma << ".\n";
     }

     return;
   }

   //----------------------------------------------------------------------------------------------
   /** Calculate the function value of a single data point without peak height
     * @param x :: tof-tof_h
     * @param alpha :: a
     * @param beta :: b
     * @param sigma2 :: sigma^2
     * @param invert_sigma2 :: 1/sigma^2
     * @param H :: effective peak width considering both gaussian and lorentzian
     * @param N :: amplitude of ... ...
     * @param eta :: ratio of Lorenzian part
     */
   double BackToBackExponentialPV::calOmega(const double x,  const double alpha, const double beta,
                                            const double sigma2, const double invert_sqrt2sigma,
                                            const double H, const double eta, const double N) const
   {
     //------------------------------------------
     // Calculate Gaussian part
     //------------------------------------------
     // rising part
     const double u = 0.5*alpha*(alpha*sigma2+2.*x);
     const double y = (alpha*sigma2 + x)*invert_sqrt2sigma;

     const double erfcy = gsl_sf_erfc(y);
     double part1(0.);
     if (fabs(erfcy) > DBL_MIN)
       part1 = exp(u)*erfcy;

     // decaying part
     const double v = 0.5*beta*(beta*sigma2 - 2.*x);
     const double z = (beta*sigma2 - x)*invert_sqrt2sigma;

     const double erfcz = gsl_sf_erfc(z);
     double part2(0.);
     if (fabs(erfcz) > DBL_MIN)
       part2 = exp(v)*erfcz;

     const double omega_gauss = (1.-eta)*N*(part1 + part2);

     // g_log.debug() << "Eta = " << eta << "; X = " << x << " N = " << N << ".\n";

     //------------------------------------------
     // Calculate Lorenzian part
     //------------------------------------------
     double omega_lorenz(0.);

     if (eta >= 1.0E-8)
     {
       const double SQRT_H_5 = sqrt(H)*.5;
       std::complex<double> p(alpha*x, alpha*SQRT_H_5);
       std::complex<double> q(-beta*x, beta*SQRT_H_5);
       double omega2a = imag(exp(p)*E1(p));
       double omega2b = imag(exp(q)*E1(q));
       omega_lorenz = -1.0*N*eta*(omega2a + omega2b)*TWO_OVER_PI;

       /*
       g_log.debug() << "Exp(p) = " << exp(p) << ", Exp(q) = " << exp(q) << ".\n";

       if (omega_lorenz != omega_lorenz)
       {
         g_log.debug() << "Omega2 is not physical.  Omega2a = " << omega2a
                       << ", Omega2b = " << omega2b << ", p = " << p.real() << ", " << p.imag() << ".\n";
       }
       else
       {
         g_log.debug() << "X = " << x << " is OK. Omega 2 = " << omega_lorenz << ", Omega2A = " << omega2a
                       << ", Omega2B = " << omega2b << "\n";
       }
       */
     }

     // Add 2 gaussian and lorenzian together
     const double omega = omega_gauss+omega_lorenz;

     /*
     if (explicitoutput || omega != omega)
     {
       if (omega <= NEG_DBL_MAX || omega >= DBL_MAX || omega != omega)
       {
         stringstream errss;
         errss << "Peak (" << mH << mK << mL << "): TOF = " << m_centre << ", dX = " << x << ", (" << x/m_fwhm << " FWHM) ";
         errss << "Omega = " << omega << " is infinity! omega1 = " << omega_gauss << ", omega2 = " << omega_lorenz << "\n";
         errss << "  u = " << u << ", v = " << v << ", erfc(y) = " << gsl_sf_erfc(y)
               << ", erfc(z) = " << gsl_sf_erfc(z) << "\n";
         errss << "  alpha = " << alpha << ", beta = " << beta << " sigma2 = " << sigma2
               << ", N = " << N << "\n";
         g_log.warning(errss.str());
       }
     }
     g_log.debug() << "[DB] Final Value of Omega = " << omega << ".\n";
     */

     return omega;
   }
  
   //----------------------------------------------------------------------------------------------
   /** Evaluate function derivatives numerically.
    */
   void BackToBackExponentialPV::functionDeriv1D(Jacobian* jacobian, const double* xValues, const size_t nData)
   {
     FunctionDomain1DView domain(xValues,nData);
     this->calNumericalDeriv(domain,*jacobian);
   }
 
   //----------------------------------------------------------------------------------------------
   /** Calculate contribution to the width by the exponentials.
    *  @return :: if a*b is 0, log(2); otherwise, log(2)*(a+b)/(a*b)
    */
   double BackToBackExponentialPV::expWidth() const
   {
     const double a = getParameter(1);
     const double b = getParameter(2);
     //Needed for IntegratePeaksMD for cylinder profile fitted with b=0
     if (a * b == 0.0)
       return M_LN2;
     return M_LN2 * (a + b) / (a * b);
   }

   //----------------------------------------------------------------------------------------------
   /** Calculate contour integral E1
    */
   std::complex<double> BackToBackExponentialPV::E1(std::complex<double> z) const
   {
     std::complex<double> exp_e1;

     double rz = real(z);
     double az = abs(z);

     if (fabs(az) < 1.0E-8)
     {
       // If z = 0, then the result is infinity... diverge!
       std::complex<double> r(1.0E300, 0.0);
       exp_e1 = r;
     }
     else if (az <= 10.0 || (rz < 0.0 && az < 20.0))
     {
       // Some interesting region, equal to integrate to infinity, converged
       // cout << "[DB] Type 1" << endl;

       std::complex<double> r(1.0, 0.0);
       exp_e1 = r;
       std::complex<double> cr = r;

       for (size_t k = 1; k <= 150; ++k)
       {
         double dk = double(k);
         cr = -cr * dk * z / ( (dk+1.0)*(dk+1.0) );
         exp_e1 += cr;
         if (abs(cr) < abs(exp_e1)*1.0E-15)
         {
           // cr is converged to zero
           break;
         }
       } // ENDFOR k


       const double el = 0.5772156649015328;
       exp_e1 = -el - log(z) + (z*exp_e1);
     }
     else
     {
       // Rest of the region
       std::complex<double> ct0(0.0, 0.0);
       for (int k = 120; k > 0; --k)
       {
         std::complex<double> dk(double(k), 0.0);
         ct0 = dk / (10.0 + dk / (z + ct0));
       } // ENDFOR k

       exp_e1 = 1.0 / (z + ct0);
       exp_e1 = exp_e1 * exp(-z);
       if (rz < 0.0 && fabs(imag(z)) < 1.0E-10 )
       {
         std::complex<double> u(0.0, 1.0);
         exp_e1 = exp_e1 - (PI * u);
       }
     }

     return exp_e1;
   }

   //----------------------------------------------------------------------------------------------
   /** Override setting parameter by parameter index
     */
   void BackToBackExponentialPV::setParameter(size_t i, const double& value, bool explicitlySet)
   {
     // Non lattice parameter
     ParamFunction::setParameter(i, value, explicitlySet);
     m_hasNewParameterValue = true;

     return;
   }

   //----------------------------------------------------------------------------------------------
   /** Overriding setting parameter by parameter name
     */
   void BackToBackExponentialPV::setParameter(const std::string& name, const double& value, bool explicitlySet)
   {
     ParamFunction::setParameter(name, value, explicitlySet);
     m_hasNewParameterValue = true;

     return;
   }

} // namespace CurveFitting
} // namespace Mantid
