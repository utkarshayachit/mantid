#ifndef MANTID_API_IFUNCTIONAUTODIFFTEST_H_
#define MANTID_API_IFUNCTIONAUTODIFFTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAPI/IFunction1DAutoDiff.h"
#include "MantidAPI/FunctionDomain1D.h"

#include "MantidKernel/MultiThreaded.h"
#include "MantidAPI/IFunction1D.h"

using Mantid::API::IFunction1DAutoDiff;
using namespace Mantid::API;

class IFunction1DAutoDiffTest : public CxxTest::TestSuite
{
public:
    // This pair of boilerplate methods prevent the suite being created statically
    // This means the constructor isn't called when running other tests
    static IFunction1DAutoDiffTest *createSuite() { return new IFunction1DAutoDiffTest(); }
    static void destroySuite( IFunction1DAutoDiffTest *suite ) { delete suite; }


    void testFunction()
    {
        AutoGaussian gauss;
        gauss.initialize();
        gauss.setParameter("Height", 1.0);
        gauss.setParameter("Sigma", 1.0);
        gauss.setParameter("Centre", 0.0);

        FunctionDomain1DVector domain(-2.0, 2.0, 101);
        FunctionValues values(domain);

        gauss.function(domain, values);

        double referenceValues[] = {   0.135335283236613,
                                       0.146489723462366,
                                       0.158310022621939,
                                       0.170810589523065,
                                       0.184003591967720,
                                       0.197898699083615,
                                       0.212502824275022,
                                       0.227819871398009,
                                       0.243850486926525,
                                       0.260591821012689,
                                       0.278037300453194,
                                       0.296176416650267};

        for(size_t i = 0; i < 12; ++i) {
            TS_ASSERT_DELTA(values[i], referenceValues[i], 1e-15);
        }
    }

    void testDerivative()
    {
        AutoGaussian gauss;
        gauss.initialize();
        gauss.setParameter("Height", 1.0);
        gauss.setParameter("Sigma", 1.0);
        gauss.setParameter("Centre", 0.0);

        FunctionDomain1DVector domain(-2.0, 2.0, 101);
        FunctionValues values(domain);

        gauss.function(domain, values);

        AutoDiffJacobianAdapter jacobian;
        jacobian.setSize(3, domain.size());

        gauss.functionDeriv(domain, jacobian);

        double ref_dfdc[] = {    -0.270670566473225,
                                 -0.287119857986237,
                                 -0.303955243434122,
                                 -0.321123908303362,
                                 -0.338566609220604,
                                 -0.356217658350506,
                                 -0.374004970724039,
                                 -0.391850178804576,
                                 -0.409668818036561,
                                 -0.427370586460810,
                                 -0.444859680725111,
                                 -0.462035209974416};

        double ref_dfds[] = {     0.541341132946451,
                                  0.562754921653024,
                                  0.583594067393515,
                                  0.603712947610321,
                                  0.622962560965912,
                                  0.641191785030912,
                                  0.658248748474308,
                                  0.673982307543870,
                                  0.688243614301423,
                                  0.700887761795729,
                                  0.711775489160177,
                                  0.720774927560089};

        double ref_dfdh[] = {      0.135335283236613,
                                   0.146489723462366,
                                   0.158310022621939,
                                   0.170810589523065,
                                   0.184003591967720,
                                   0.197898699083615,
                                   0.212502824275022,
                                   0.227819871398009,
                                   0.243850486926525,
                                   0.260591821012689,
                                   0.278037300453194,
                                   0.296176416650267};

        for(size_t i = 0; i < 12; ++i) {
            TS_ASSERT_DELTA(jacobian.get(i, 0), ref_dfdh[i], 1e-15);
            TS_ASSERT_DELTA(jacobian.get(i, 1), ref_dfds[i], 1e-15);
            TS_ASSERT_DELTA(jacobian.get(i, 2), ref_dfdc[i], 1e-15);
        }

        AutoGaussian othergauss;
        othergauss.initialize();
        othergauss.setParameter("Height", 1.0);
        othergauss.setParameter("Sigma", 1.0);
        othergauss.setParameter("Centre", 0.0);

        AutoDiffJacobianAdapter otherjacobian;
        otherjacobian.setSize(3, domain.size());

        othergauss.functionDeriv(domain, otherjacobian);

        for(size_t i = 0; i < 12; ++i) {
            TS_ASSERT_DELTA(otherjacobian.get(i, 0), ref_dfdh[i], 1e-15);
            TS_ASSERT_DELTA(otherjacobian.get(i, 1), ref_dfds[i], 1e-15);
            TS_ASSERT_DELTA(otherjacobian.get(i, 2), ref_dfdc[i], 1e-15);
        }

    }

    void testSpeed()
    {
        /*
         * reference values from matlab symbolic toolbox
         * with a domain from -2 to 2 with 501 steps.
         * for sigma = 1, 2, 3
         */
        double ref_dfda[] = {            0.135335283236613,
                                         0.137513662936659,
                                         0.139718163957622,
                                         0.141948920625269,
                                         0.144206064333246,
                                         0.146489723462366,
                                         0.148800023299574,
                                         0.151137085956631,
                                         0.153501030288533,
                                         0.155891971811695,
                                         0.158310022621939,
                                         0.160755291312287,

                                         0.606530659712633,
                                         0.608956769399255,
                                         0.611382801269737,
                                         0.613808677227026,
                                         0.616234318871067,
                                         0.618659647502622,
                                         0.621084584127140,
                                         0.623509049458657,
                                         0.625932963923738,
                                         0.628356247665460,
                                         0.630778820547428,
                                         0.633200602157834,

                                         0.800737402916808,
                                         0.802159350063333,
                                         0.803578107945926,
                                         0.804993650728161,
                                         0.806405952596652,
                                         0.807814987761839,
                                         0.809220730458753,
                                         0.810623154947802,
                                         0.812022235515541,
                                         0.813417946475454,
                                         0.814810262168729,
                                         0.816199156965039};

        AutoGaussian gauss;
        gauss.initialize();

        for(size_t j = 0; j < 32; ++j) {
            size_t curr = j % 3;
            double sigma = static_cast<double>(curr + 1);

            gauss.setParameter(0, 1.0);
            gauss.setParameter(1, sigma);
            gauss.setParameter(2, 0.0);

            //PARALLEL_FOR_NO_WSP_CHECK()
            for(size_t i = 0; i < 40; ++i) {
                FunctionDomain1DVector domain(-2.0, 2.0, 501);
                FunctionValues values(domain);
                AutoDiffJacobianAdapter otherjacobian;
                otherjacobian.setSize(3, domain.size());

                gauss.function(domain, values);
                gauss.functionDeriv(domain, otherjacobian);

                for(size_t k = 0; k < 12; ++k) {
                    TS_ASSERT_DELTA(otherjacobian.get(k, 0), ref_dfda[curr * 12 + k], 1e-15);
                }
            }
        }
    }
private:
    class AutoGaussian : virtual public IFunction1DAutoDiff, virtual public ParamFunction
    {
    public:
        AutoGaussian() { }
        ~AutoGaussian() { }

        std::string name() const { return "AutoGauss"; }

        void function1DAutoDiff(const FunctionDomain1D &domain, std::vector<adept::adouble> &y, const AutoDiffParameterAdapter &params) const
        {
            adept::adouble height = params.getParameter(0);
            adept::adouble sigma = params.getParameter(1);
            adept::adouble centre = params.getParameter(2);

            for(size_t i = 0; i < y.size(); ++i) {
                //y[i] = area / (sigma * sqrt(2.0 * M_PI)) * exp(-0.5 * pow( (x[i] - centre)/sigma, 2.0) );
                y[i] = height * exp(-0.5 * pow( (domain[i] - centre)/sigma, 2.0) );
            }
        }

    protected:
        void init()
        {
            declareParameter("Height");
            declareParameter("Sigma");
            declareParameter("Centre");
        }

    };
};


#endif /* MANTID_API_IFUNCTIONAUTODIFFTEST_H_ */
