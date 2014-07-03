.. algorithm::

.. summary::

.. alias::

.. properties::

Description
-----------

Calculates the probability of a neutron being transmitted through the
sample using detected counts from two monitors, one in front and one
behind the sample. A data workspace can be corrected for transmission by
`dividing <http://www.mantidproject.org/Divide>`_ by the output of this algorithm.

Because the detection efficiency of the monitors can be different the
transmission calculation is done using two runs, one run with the sample
(represented by :math:`S` below) and a direct run without
it (:math:`D`). The fraction transmitted through the sample :math:`f` is calculated from this formula:

.. math::p = \frac{S_T}{D_T}\frac{D_I}{S_I}

where :math:`S_I` is the number of counts from the monitor in front of
the sample (the incident beam monitor), :math:`S_T` is the transmission
monitor after the sample, etc.

The resulting fraction as a function of wavelength is created as the
OutputUnfittedData workspace. However, because of statistical variations
it is recommended to use the OutputWorkspace, which is the evaluation of
a fit to those transmission fractions. The unfitted data is not affected
by the RebinParams or Fitmethod properties but these can be used to
refine the fitted data. The RebinParams method is useful when the range
of wavelengths passed to CalculateTransmission is different from that of
the data to be corrected.

ChildAlgorithms used
####################

Uses the algorithm `Fit <algm-Fit>`_ to fit to the calculated
transmission fraction.

Usage
-----

.. include:: ../usagedata-note.txt

**Example - Calculate Trasnmission Using a Polynomial Fit:**

.. testcode:: Ex

   # Take different parts of the same data to simulate sample and direct
   # can workspaces.
   dummy_data = LoadRaw("LOQ48097", SpectrumMin=1, SpectrumMax=2)
   dummy_data = ConvertUnits(dummy_data, "Wavelength")
   sample = Rebin(dummy_data, "7.5, 0.1, 9")
   direct = Rebin(dummy_data, "6, 0.1, 7.5")

   # (After the rebin, need to manually set some common X binning.)
   for ws_index in [0, 1]:
       for bin in range(sample.blocksize() + 1):
           sample.dataX(ws_index)[bin] = direct.readX(ws_index)[bin]

   # Calculate and print result.
   poly = CalculateTransmission(sample, direct, 
                                IncidentBeamMonitor=1,
                                TransmissionMonitor=2,
                                RebinParams="7.5, 0.1, 9",
                                FitMethod="Polynomial",
                                PolynomialOrder=3)
   print "The Y values of the resulting polynomial fit are:"
   print poly.readX(0)

Output:

.. testoutput:: Ex

   The Y values of the resulting polynomial fit are:
   [ 7.5  7.6  7.7  7.8  7.9  8.   8.1  8.2  8.3  8.4  8.5  8.6  8.7  8.8  8.9
     9. ]

.. categories::
