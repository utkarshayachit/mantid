.. algorithm::

.. summary::

.. alias::

.. properties::

Description
-----------

Threshold an MDHistoWorkspace to overwrite values below or above the
defined threshold.

Usage
-----

**Example - Threshold a small workspace:**

.. testcode:: ExThresholdMD

   # Create input workspace
   CreateMDWorkspace(Dimensions=3, Extents='-10,10,-10,10,-10,10', Names='A,B,C', Units='U,U,U', OutputWorkspace='demo')
   FakeMDEventData(InputWorkspace='demo', PeakParams='32,0,0,0,1')
   threshold_input = BinMD(InputWorkspace='demo', AlignedDim0='A,-2,2,4', AlignedDim1='B,-2,2,4', AlignedDim2='C,-2,2,4')

   # Run the algorithm to set all values greater than 4 to zero
   threshold_output = ThresholdMD(InputWorkspace='threshold_input', Condition='Greater Than', ReferenceValue=4)

   # Print selection before and after
   print "selected bins before threshold greater than 4",threshold_input.getSignalArray()[1,1]
   print "same bins after threshold greater than 4",threshold_output.getSignalArray()[1,1]

Output:

.. testoutput:: ExThresholdMD

   selected bins before threshold greater than 4 [ 0.  4.  5.  0.]
   same bins after threshold greater than 4 [ 0.  4.  0.  0.]

.. categories::
