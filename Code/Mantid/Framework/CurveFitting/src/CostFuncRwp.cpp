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


} // namespace CurveFitting
} // namespace Mantid
