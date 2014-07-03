.. algorithm::

.. summary::

.. alias::

.. properties::

Description
-----------

Subtract background from an I(Q) distribution.

The DataDistribution and Background properties can either be the name of
a workspace or a file path. If a file path is provided, it will be
loaded and assumed to be in units of Q.

The output workspace will be equal to:

| `` Output = DataDistribution - ScaleFactor * Background + Constant``

The Dq values are propagated from the DataDistribution workspace to the
output workspace as-is.

If the OutputDirectory property is filled, the output workspace will be
written to disk. Two files will be produced, a 4 column ASCII file and a
CanSAS XML file.

Usage
-----

**Example - Subtract Background From Distribution with Constant and Scale Factor :**

.. testcode:: Ex

   # Create a dummy distribution and background.
   dataX = [0,1,2,3,4,5]
   distribution = CreateWorkspace(DataX=dataX, DataY=[1,2,3,4,5])
   background = CreateWorkspace(DataX=dataX, DataY=[3,5,7,9,11])

   # Do the subtraction and print the result.
   result = SANSSubtract(distribution, background, Constant=1, ScaleFactor=2)
   print "The result of the subtraction was:", result.readY(0)

Output:

.. testoutput:: Ex

   The result of the subtraction was: [ -4.  -7. -10. -13. -16.]

.. categories::
