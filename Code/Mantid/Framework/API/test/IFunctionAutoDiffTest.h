#ifndef MANTID_API_IFUNCTIONAUTODIFFTEST_H_
#define MANTID_API_IFUNCTIONAUTODIFFTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAPI/IFunctionAutoDiff.h"
#include "MantidAPI/FunctionDomain1D.h"

#include "MantidKernel/MultiThreaded.h"


#include <iomanip>
#include <chrono>

using Mantid::API::IFunctionAutoDiff;
using namespace Mantid::API;

class IFunctionAutoDiffTest : public CxxTest::TestSuite
{
public:
    // This pair of boilerplate methods prevent the suite being created statically
    // This means the constructor isn't called when running other tests
    static IFunctionAutoDiffTest *createSuite() { return new IFunctionAutoDiffTest(); }
    static void destroySuite( IFunctionAutoDiffTest *suite ) { delete suite; }


    void testFunction()
    {
        AutoGaussian gauss;
        gauss.initialize();
        gauss.setParameter("Area", 1.0);
        gauss.setParameter("Sigma", 1.0);
        gauss.setParameter("Centre", 0.0);

        FunctionDomain1DVector domain(-2.0, 2.0, 101);
        FunctionValues values(domain);

        gauss.function(domain, values);

        double referenceValues[] = {0.0539909665131881,
                                    0.0584409443334515,
                                    0.0631565614351987,
                                    0.0681435661010446,
                                    0.0734068125816569,
                                    0.0789501583008941,
                                    0.0847763613080222,
                                    0.0908869790162829,
                                    0.0972822693314675,
                                    0.1039610953287642,
                                    0.1109208346794555,
                                    0.1181572950595823,
                                    0.1256646367890882,
                                    0.1334353039510023,
                                    0.1414599652248388};

        for(size_t i = 0; i < 15; ++i) {
            TS_ASSERT_DELTA(values[i], referenceValues[i], 1e-16);
        }
    }

    void testDerivative()
    {
        AutoGaussian gauss;
        gauss.initialize();
        gauss.setParameter("Area", 1.0);
        gauss.setParameter("Sigma", 1.0);
        gauss.setParameter("Centre", 0.0);

        FunctionDomain1DVector domain(-2.0, 2.0, 101);
        FunctionValues values(domain);

        gauss.function(domain, values);

        AutoDiffJacobian jacobian;
        jacobian.setSize(3, domain.size());

        gauss.functionDeriv(domain, jacobian);

        double ref_dfdc[] = {  -0.107981933026376,
                               -0.114544250893565,
                               -0.121260597955581,
                               -0.128109904269964,
                               -0.135068535150249,
                               -0.142110284941609,
                               -0.149206395902119,
                               -0.156325603908007,
                               -0.163434212476865,
                               -0.170496196339173,
                               -0.177473335487129,
                               -0.184325380292948};

        double ref_dfds[] = {   0.161972899539564,
                                0.166065787417936,
                                0.169663786639518,
                                0.172703053926487,
                                0.175119292094801,
                                0.176848354594003,
                                0.177826895479707,
                                0.177993059705488,
                                0.177287207629666,
                                0.175652666667480,
                                0.173036502099951,
                                0.169390298197417};

        double ref_dfda[] = {   0.053990966513188,
                                0.058440944333451,
                                0.063156561435199,
                                0.068143566101045,
                                0.073406812581657,
                                0.078950158300894,
                                0.084776361308022,
                                0.090886979016283,
                                0.097282269331467,
                                0.103961095328764,
                                0.110920834679456,
                                0.118157295059582};

        for(size_t i = 0; i < 12; ++i) {
            TS_ASSERT_DELTA(jacobian.get(i, 0), ref_dfda[i], 1e-15);
            TS_ASSERT_DELTA(jacobian.get(i, 1), ref_dfds[i], 1e-15);
            TS_ASSERT_DELTA(jacobian.get(i, 2), ref_dfdc[i], 1e-15);
        }

        AutoGaussian othergauss;
        othergauss.initialize();
        othergauss.setParameter("Area", 1.0);
        othergauss.setParameter("Sigma", 1.0);
        othergauss.setParameter("Centre", 0.0);

        AutoDiffJacobian otherjacobian;
        otherjacobian.setSize(3, domain.size());

        othergauss.functionDeriv(domain, otherjacobian);

        for(size_t i = 0; i < 12; ++i) {
            TS_ASSERT_DELTA(otherjacobian.get(i, 0), ref_dfda[i], 1e-15);
            TS_ASSERT_DELTA(otherjacobian.get(i, 1), ref_dfds[i], 1e-15);
            TS_ASSERT_DELTA(otherjacobian.get(i, 2), ref_dfdc[i], 1e-15);
        }

    }

    void testSpeed()
    {
        double ref_dfda[] = {      0.053990966513188,
                                   0.054860014278305,
                                   0.055739482942755,
                                   0.056629426094767,
                                   0.057529896152821,
                                   0.058440944333451,
                                   0.059362620618918,
                                   0.060294973724766,
                                   0.061238051067277,
                                   0.062191898730834,
                                   0.063156561435199,
                                   0.064132082502720,

                                   0.120985362259572,
                                   0.121469301125014,
                                   0.121953224468382,
                                   0.122437116711568,
                                   0.122920962216024,
                                   0.123404745283521,
                                   0.123888450156928,
                                   0.124372061020983,
                                   0.124855562003082,
                                   0.125338937174073,
                                   0.125822170549059,
                                   0.126305246088203,

                                   0.106482668507451,
                                   0.106671760119866,
                                   0.106860427621539,
                                   0.107048667576722,
                                   0.107236476552733,
                                   0.107423851120054,
                                   0.107610787852443,
                                   0.107797283327027,
                                   0.107983334124413,
                                   0.108168936828789,
                                   0.108354088028027,
                                   0.108538784313786

                            };

        AutoGaussian gauss;
        gauss.initialize();

        auto t1 = std::chrono::high_resolution_clock::now();

        for(size_t j = 0; j < 32; ++j) {
            size_t curr = j % 3;

            gauss.setParameter(0, 1.0);
            gauss.setParameter(1, static_cast<double>(curr + 1));
            gauss.setParameter(2, 0.0);

            PARALLEL_FOR_NO_WSP_CHECK()
            for(size_t i = 0; i < 400; ++i) {
                FunctionDomain1DVector domain(-2.0, 2.0, 501);
                FunctionValues values(domain);
                AutoDiffJacobian otherjacobian;
                otherjacobian.setSize(3, domain.size());

                gauss.function(domain, values);
                gauss.functionDeriv(domain, otherjacobian);

                for(size_t k = 0; k < 12; ++k) {
                    TS_ASSERT_DELTA(otherjacobian.get(k, 0), ref_dfda[curr * 12 + k], 1e-15);
                }
            }
        }

        auto t2 = std::chrono::high_resolution_clock::now();

        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << " ms" << std::endl;
    }
private:
    class AutoGaussian : virtual public IFunctionAutoDiff, virtual public ParamFunction
    {
    public:
        AutoGaussian() { }
        ~AutoGaussian() { }

        std::string name() const { return "AutoGauss"; }

        void functionAutoDiff(std::vector<adept::adouble> &x, std::vector<adept::adouble> &y, const AutoDiffParameters &params) const
        {
            adept::adouble area = params.getParameter(0);
            adept::adouble sigma = params.getParameter(1);
            adept::adouble centre = params.getParameter(2);

            for(size_t i = 0; i < x.size(); ++i) {
                y[i] = area / (sigma * sqrt(2.0 * M_PI)) * exp(-0.5 * pow( (x[i] - centre)/sigma, 2.0) );
            }
        }

    protected:
        void init()
        {
            declareParameter("Area");
            declareParameter("Sigma");
            declareParameter("Centre");
        }

    };


};


#endif /* MANTID_API_IFUNCTIONAUTODIFFTEST_H_ */
