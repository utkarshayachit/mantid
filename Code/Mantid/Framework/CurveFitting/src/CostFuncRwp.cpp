//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidCurveFitting/CostFuncRwp.h"
#include "MantidCurveFitting/Jacobian.h"
#include "MantidCurveFitting/SeqDomain.h"
#include "MantidAPI/IConstraint.h"
#include "MantidAPI/CompositeDomain.h"
#include "MantidAPI/FunctionValues.h"

#include <iomanip>

namespace Mantid
{
namespace CurveFitting
{

DECLARE_COSTFUNCTION(CostFuncRwp,Rwp)

//----------------------------------------------------------------------------------------------
/** Constructor
 */
CostFuncRwp::CostFuncRwp() : CostFuncLeastSquares()
{
  m_includePenalty = false;
  m_value = 0.;
  m_pushed = false;
  m_factor = 1.;
}

//----------------------------------------------------------------------------------------------
/** Get weight of data point i(1/sigma)
  * @param values :: function values
  * @param i :: index of the data point
  * @param sqrtW :: pre-calculated sqrt(sum(y*w)^2)
  */
double CostFuncRwp::getWeight(const API::FunctionValues_sptr& values, size_t i, double sqrtW) const
{
  double fitweight = values->getFitWeight(i);

  return (fitweight/sqrtW);
}

//----------------------------------------------------------------------------------------------
/** Get square root of normalization weight (W)
  * @param values :: function values to calculated weight for
  */
double CostFuncRwp::calSqrtW(const API::FunctionValues_sptr &values) const
{
  double weight = 0.0;
  size_t ny = values->size();

  // FIXME : This might give a wrong answer in case of multiple-domain
  // TODO : Test it later with multi-domain in another ticket.
  for (size_t i = 0; i < ny; ++i)
  {
    double obsval = values->getFitData(i);
    double inv_sigma = values->getFitWeight(i);
    weight += obsval * obsval * inv_sigma * inv_sigma;
  }

  double rvalue = sqrt(weight);

  return rvalue;
}

/** Override add value
 * Add a contribution to the cost function value from the fitting function evaluated on a particular domain.
 * @param domain :: A domain
 * @param values :: Values
 */
void CostFuncRwp::addVal(API::FunctionDomain_sptr domain, API::FunctionValues_sptr values)const
{
  m_function->function(*domain,*values);
  size_t ny = values->size();

  double retVal = 0.0;

  double sqrtw = calSqrtW(values);

  for (size_t i = 0; i < ny; i++)
  {
    // double val = ( values->getCalculated(i) - values->getFitData(i) ) * values->getFitWeight(i);
    double val = ( values->getCalculated(i) - values->getFitData(i) ) * getWeight(values, i, sqrtw);
    retVal += val * val;
  }

  PARALLEL_ATOMIC
  m_value += m_factor * retVal;

  return;
}

/** Override
 * Update the cost function, derivatives and hessian by adding values calculated
 * on a domain.
 * @param function :: Function to use to calculate the value and the derivatives
 * @param domain :: The domain.
 * @param values :: The fit function values
 * @param evalFunction :: Flag to evaluate the function
 * @param evalDeriv :: Flag to evaluate the derivatives
 * @param evalHessian :: Flag to evaluate the Hessian
 */
void CostFuncRwp::addValDerivHessian(
  API::IFunction_sptr function,
  API::FunctionDomain_sptr domain,
  API::FunctionValues_sptr values,
  bool evalFunction , bool evalDeriv, bool evalHessian) const
{
  UNUSED_ARG(evalDeriv);
  size_t np = function->nParams();  // number of parameters
  size_t ny = domain->size(); // number of data points
  Jacobian jacobian(ny,np);
  if (evalFunction)
  {
    function->function(*domain,*values);
  }
  function->functionDeriv(*domain,jacobian);

  size_t iActiveP = 0;
  double fVal = 0.0;
  /*
  if (debug)
  {
    std::cerr << "Jacobian:\n";
    for(size_t i = 0; i < ny; ++i)
    {
      for(size_t ip = 0; ip < np; ++ip)
      {
        if ( !m_function->isActive(ip) ) continue;
        std::cerr << jacobian.get(i,ip) << ' ';
      }
      std::cerr << std::endl;
    }
  }
  */
  double sqrtw = calSqrtW(values);
  for(size_t ip = 0; ip < np; ++ip)
  {
    if ( !function->isActive(ip) ) continue;
    double d = 0.0;
    for(size_t i = 0; i < ny; ++i)
    {
      double calc = values->getCalculated(i);
      double obs = values->getFitData(i);
      // double w = values->getFitWeight(i);
      double w =  getWeight(values, i, sqrtw);
      double y = ( calc - obs ) * w;
      d += y * jacobian.get(i,ip) * w;
      if (iActiveP == 0 && evalFunction)
      {
        fVal += y * y;
      }
    }
    PARALLEL_CRITICAL(der_set)
    {
      double der = m_der.get(iActiveP);
      m_der.set(iActiveP, der + d);
    }
    //std::cerr << "der " << ip << ' ' << der[iActiveP] << std::endl;
    ++iActiveP;
  }

  if (evalFunction)
  {
    PARALLEL_ATOMIC
    m_value += 0.5 * fVal;
  }

  if (!evalHessian) return;

  //size_t na = m_der.size(); // number of active parameters

  size_t i1 = 0; // active parameter index
  for(size_t i = 0; i < np; ++i) // over parameters
  {
    if ( !function->isActive(i) ) continue;
    size_t i2 = 0; // active parameter index
    for(size_t j = 0; j <= i; ++j) // over ~ half of parameters
    {
      if ( !function->isActive(j) ) continue;
      double d = 0.0;
      for(size_t k = 0; k < ny; ++k) // over fitting data
      {
        // double w = values->getFitWeight(k);
        double w = getWeight(values, k, sqrtw);
        d += jacobian.get(k,i) * jacobian.get(k,j) * w * w;
      }
      PARALLEL_CRITICAL(hessian_set)
      {
        double h = m_hessian.get(i1,i2);
        m_hessian.set(i1,i2, h + d);
        //std::cerr << "hess " << i1 << ' ' << i2 << std::endl;
        if (i1 != i2)
        {
          m_hessian.set(i2,i1,h + d);
        }
      }
      ++i2;
    }
    ++i1;
  }
}


} // namespace CurveFitting
} // namespace Mantid
