#ifndef MANTID_API_IFUNCTIONAUTODIFFTEST_H_
#define MANTID_API_IFUNCTIONAUTODIFFTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAPI/IFunction1DAutoDiff.h"
#include "MantidAPI/FunctionDomain1D.h"

#include "MantidKernel/MultiThreaded.h"
#include "MantidAPI/IFunction1D.h"
#include "MantidAPI/ParamFunction.h"

using Mantid::API::IFunction1DAutoDiff;
using namespace Mantid::API;

class IFunction1DAutoDiffTest : public CxxTest::TestSuite
{
public:
    // This pair of boilerplate methods prevent the suite being created statically
    // This means the constructor isn't called when running other tests
    static IFunction1DAutoDiffTest *createSuite() { return new IFunction1DAutoDiffTest(); }
    static void destroySuite( IFunction1DAutoDiffTest *suite ) { delete suite; }

    void testAutoDiffJacobianAdapterConstruction()
    {
        TS_ASSERT_THROWS_NOTHING(AutoDiffJacobianAdapter adapter(0, 0));
        TS_ASSERT_THROWS_NOTHING(AutoDiffJacobianAdapter adapter(0, 10));
        TS_ASSERT_THROWS_NOTHING(AutoDiffJacobianAdapter adapter(10, 0));
        TS_ASSERT_THROWS_NOTHING(AutoDiffJacobianAdapter adapter(10, 10));
    }

    void testAutoDiffJacobianAdapterGetSet()
    {
        /* These checks also verify that the protected methods
         * getRaw, index and safeIndex work as expected.
         */
        AutoDiffJacobianAdapter adapter(20, 3);

        for(size_t y = 0; y < 20; ++y) {
            for(size_t p = 0; p < 3; ++p) {
                double value = static_cast<double>(y * p);
                TS_ASSERT_THROWS_NOTHING(adapter.set(y, p, value));
                TS_ASSERT_THROWS_NOTHING(adapter.get(y, p));

                TS_ASSERT_EQUALS(adapter.get(y, p), value);
            }
        }

        TS_ASSERT_THROWS(adapter.set(20, 3, 30.0), std::out_of_range);
        TS_ASSERT_THROWS(adapter.set(10, 4, 30.0), std::out_of_range);

        TS_ASSERT_THROWS(adapter.get(20, 3), std::out_of_range);
        TS_ASSERT_THROWS(adapter.get(10, 4), std::out_of_range);
    }

    void testAutoDiffJacobianAdapterRawValues()
    {
        AutoDiffJacobianAdapter writeAdapter(3, 1);

        double *rawJacobianWrite = writeAdapter.rawValues();
        for(size_t i = 0; i < 3; ++i) {
            *(rawJacobianWrite + i) = static_cast<double>(i + 1);
        }

        TS_ASSERT_EQUALS(writeAdapter.get(0, 0), 1.0);
        TS_ASSERT_EQUALS(writeAdapter.get(1, 0), 2.0);
        TS_ASSERT_EQUALS(writeAdapter.get(2, 0), 3.0);

        AutoDiffJacobianAdapter readAdapter(3, 1);
        readAdapter.set(0, 0, 1.0);
        readAdapter.set(1, 0, 2.0);
        readAdapter.set(2, 0, 3.0);

        double *rawJacobianRead = readAdapter.rawValues();
        for(size_t i = 0; i < 3; ++i) {
            TS_ASSERT_EQUALS(*(rawJacobianRead + i), static_cast<double>(i + 1));
        }
    }

    void testAutoDiffParameterAdapterConstruction()
    {
        TS_ASSERT_THROWS(AutoDiffParameterAdapter adapter(0), std::invalid_argument);

        AutoGaussian gauss;
        TS_ASSERT_THROWS_NOTHING(AutoDiffParameterAdapter adapter(&gauss));
    }

    void testAutoDiffParameterAdapterParameters()
    {
        AutoGaussian uninitializedGauss;

        AutoDiffParameterAdapter emptyAdapter(&uninitializedGauss);
        TS_ASSERT_EQUALS(emptyAdapter.size(), 0);
        TS_ASSERT_THROWS(emptyAdapter.getParameter(0), std::out_of_range);
        TS_ASSERT_THROWS(emptyAdapter.getParameter(1), std::out_of_range);
        TS_ASSERT_THROWS(emptyAdapter.getParameter("Height"), std::out_of_range);

        AutoGaussian initializedGauss;
        initializedGauss.initialize();
        initializedGauss.setParameter("Height", 3.0);
        initializedGauss.setParameter("Centre", 1.0);
        initializedGauss.setParameter("Sigma", 7.0);

        /* For this we need an adept::Stack, otherwise calling adept::adouble::value()
         * causes a segmentation fault*/
        adept::Stack stack;
        stack.new_recording();

        AutoDiffParameterAdapter adapter(&initializedGauss);
        TS_ASSERT_EQUALS(adapter.size(), 3); // check that it's actually something other than 0.
        TS_ASSERT_EQUALS(adapter.size(), initializedGauss.nParams());

        for(size_t i = 0; i < adapter.size(); ++i) {
            TS_ASSERT_EQUALS(adapter.getParameter(0).value(), initializedGauss.getParameter(0));
        }

        std::vector<std::string> parameterNames = initializedGauss.getParameterNames();
        for(size_t i = 0; i < parameterNames.size(); ++i) {
            TS_ASSERT_EQUALS(adapter.getParameter(parameterNames[i]), initializedGauss.getParameter(parameterNames[i]));
        }

        TS_ASSERT_EQUALS(adapter[0].value(), 3.0);
        TS_ASSERT_EQUALS(adapter[1].value(), 7.0);
        TS_ASSERT_EQUALS(adapter[2].value(), 1.0);

        /* Now the unhappy cases */
        TS_ASSERT_THROWS(adapter.getParameter(3), std::out_of_range);
        TS_ASSERT_THROWS(adapter.getParameter("BlaBla"), std::out_of_range);

    }

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

        double referenceValues[] = {0.135335283236613,
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

        AutoDiffJacobianAdapter jacobian(domain.size(), 3);

        gauss.functionDeriv(domain, jacobian);

        // dy/d(Centre)
        double ref_dfdc[] = {-0.270670566473225,
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

        // dy/d(Sigma)
        double ref_dfds[] = {0.541341132946451,
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

        // dy/d(Height)
        double ref_dfdh[] = {0.135335283236613,
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

        AutoDiffJacobianAdapter otherjacobian(domain.size(), 3);

        othergauss.functionDeriv(domain, otherjacobian);

        for(size_t i = 0; i < 12; ++i) {
            TS_ASSERT_DELTA(otherjacobian.get(i, 0), ref_dfdh[i], 1e-15);
            TS_ASSERT_DELTA(otherjacobian.get(i, 1), ref_dfds[i], 1e-15);
            TS_ASSERT_DELTA(otherjacobian.get(i, 2), ref_dfdc[i], 1e-15);
        }

    }

    void testSpeed()
    {
        /* This unit test should be very fast. If it is slow
         * for some reason, a performance problem has been
         * introduced somehow.
         *
         * Reference values for dy/d(Height), obtained
         * from matlab symbolic toolbox with a domain
         * from -2 to 2 with 501 steps, for sigma = {1, 2, 3}
         */
        double ref_dfda[] = {0.135335283236613, // sigma = 1
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

                             0.606530659712633, // sigma = 2
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

                             0.800737402916808, // sigma = 3
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

            for(size_t i = 0; i < 40; ++i) {
                FunctionDomain1DVector domain(-2.0, 2.0, 501);
                FunctionValues values(domain);

                /* Here AutoDiffJacobianAdapter is used for this purpose,
                 * but in general it should not be used outside of
                 * IFunction1DAutoDiff
                 */
                AutoDiffJacobianAdapter otherjacobian(domain.size(), 3);

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
