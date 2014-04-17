#ifndef MANTID_ALGORITHMS_CHOPEVENTFILTERS_H_
#define MANTID_ALGORITHMS_CHOPEVENTFILTERS_H_

#include <cxxtest/TestSuite.h>

#include "MantidAlgorithms/ChopEventFilters.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidDataObjects/Workspace2D.h"

using Mantid::Algorithms::ChopEventFilters;

using namespace Mantid;
using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::DataObjects;

using namespace std;

class FitPeakTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static FitPeakTest *createSuite()
  {
    API::FrameworkManager::Instance();
    return new FitPeakTest();
  }
  static void destroySuite( FitPeakTest *suite ) { delete suite; }

  //----------------------------------------------------------------------------------------------
  /** Test on init and setup
    */
  void test_Init()
  {
    // Generate input workspace
    MatrixWorkspace_sptr dataws = gen_4866P5Data();
    AnalysisDataService::Instance().addOrReplace("PG3_4866Peak5", dataws);

    // Generate peak and background parameters
    vector<string> peakparnames, bkgdparnames;
    vector<double> peakparvalues, bkgdparvalues;

    gen_BkgdParameters(bkgdparnames, bkgdparvalues);
    gen_PeakParameters(peakparnames, peakparvalues);

    // Initialize FitPeak
    ChopEventFilters fitpeak;

    fitpeak.initialize();
    TS_ASSERT(fitpeak.isInitialized());

    // Set properties
    TS_ASSERT_THROWS_NOTHING(fitpeak.setPropertyValue("InputWorkspace", "PG3_4866Peak5"));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setPropertyValue("OutputWorkspace", "FittedPeak"));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setPropertyValue("ParameterTableWorkspace", "Fitted_Peak5_Parameters"));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("WorkspaceIndex", 0));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("PeakFunctionType", "Gaussian"));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("PeakParameterNames", peakparnames));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("PeakParameterValues", peakparvalues));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("BackgroundType", "Linear"));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("BackgroundParameterNames", bkgdparnames));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("BackgroundParameterValues", bkgdparvalues));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setPropertyValue("FitWindow", "10.0, 20.0"));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setPropertyValue("PeakRange", "11.0, 12.0"));
    TS_ASSERT_THROWS_NOTHING(fitpeak.setProperty("FitBackgroundFirst", true));

    // Clean
    AnalysisDataService::Instance().remove("PG3_4866Peak5");
    AnalysisDataService::Instance().remove("Peak5_Parameters");

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Generate a workspace contains PG3_4866 5-th peak
    */
  MatrixWorkspace_sptr gen_PG3DiamondData()
  {
    vector<double> vecx, vecy, vece;

    vecx.push_back(2.050678);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.051498);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.052319);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.053140);    vecy.push_back(2.000000 ); vece.push_back(1.41421E+00);
    vecx.push_back(2.053961);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.054783);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.055605);    vecy.push_back(2.000000 ); vece.push_back(1.41421E+00);
    vecx.push_back(2.056427);    vecy.push_back(2.000000 ); vece.push_back(1.41421E+00);
    vecx.push_back(2.057250);    vecy.push_back(3.000000 ); vece.push_back(1.73205E+00);
    vecx.push_back(2.058072);    vecy.push_back(4.000000 ); vece.push_back(2.00000E+00);
    vecx.push_back(2.058896);    vecy.push_back(5.000000 ); vece.push_back(2.23607E+00);
    vecx.push_back(2.059719);    vecy.push_back(16.000000); vece.push_back(4.00000E+00);
    vecx.push_back(2.060543);    vecy.push_back(20.000000); vece.push_back(4.47214E+00);
    vecx.push_back(2.061367);    vecy.push_back(31.000000); vece.push_back(5.56776E+00);
    vecx.push_back(2.062192);    vecy.push_back(26.000000); vece.push_back(5.09902E+00);
    vecx.push_back(2.063017);    vecy.push_back(28.000000); vece.push_back(5.29150E+00);
    vecx.push_back(2.063842);    vecy.push_back(29.000000); vece.push_back(5.38516E+00);
    vecx.push_back(2.064668);    vecy.push_back(41.000000); vece.push_back(6.40312E+00);
    vecx.push_back(2.065493);    vecy.push_back(40.000000); vece.push_back(6.32456E+00);
    vecx.push_back(2.066320);    vecy.push_back(38.000000); vece.push_back(6.16441E+00);
    vecx.push_back(2.067146);    vecy.push_back(40.000000); vece.push_back(6.32456E+00);
    vecx.push_back(2.067973);    vecy.push_back(34.000000); vece.push_back(5.83095E+00);
    vecx.push_back(2.068800);    vecy.push_back(35.000000); vece.push_back(5.91608E+00);
    vecx.push_back(2.069628);    vecy.push_back(18.000000); vece.push_back(4.24264E+00);
    vecx.push_back(2.070456);    vecy.push_back(21.000000); vece.push_back(4.58258E+00);
    vecx.push_back(2.071284);    vecy.push_back(9.000000 ); vece.push_back(3.00000E+00);
    vecx.push_back(2.072112);    vecy.push_back(6.000000 ); vece.push_back(2.44949E+00);
    vecx.push_back(2.072941);    vecy.push_back(6.000000 ); vece.push_back(2.44949E+00);
    vecx.push_back(2.073770);    vecy.push_back(11.000000); vece.push_back(3.31662E+00);
    vecx.push_back(2.074600);    vecy.push_back(10.000000); vece.push_back(3.16228E+00);
    vecx.push_back(2.075430);    vecy.push_back(4.000000 ); vece.push_back(2.00000E+00);
    vecx.push_back(2.076260);    vecy.push_back(7.000000 ); vece.push_back(2.64575E+00);
    vecx.push_back(2.077090);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.077921);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.078752);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.079584);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.080416);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.081248);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.082080);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.082913);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.083746);    vecy.push_back(2.000000 ); vece.push_back(1.41421E+00);
    vecx.push_back(2.084580);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.085414);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.086248);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.087082);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.087917);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.088752);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.089588);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.090424);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.091260);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.092096);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.092933);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(2.093770);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);

    size_t NVectors = 1;
    size_t sizex = vecx.size();
    size_t sizey = vecy.size();
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(
            WorkspaceFactory::Instance().create("Workspace2D", NVectors, sizex, sizey));

    MantidVec& vecX = ws->dataX(0);
    MantidVec& vecY = ws->dataY(0);
    MantidVec& vecE = ws->dataE(0);

    for (size_t i = 0; i < sizex; ++i)
        vecX[i] = vecx[i];
    for (size_t i = 0; i < sizey; ++i)
    {
        vecY[i] = vecy[i];
        vecE[i] = vece[i];
    }

    return ws;
  }

};


#endif /* MANTID_ALGORITHMS_CHOPEVENTFILTERS_H_ */
