.. algorithm::

.. summary::

.. alias::

.. properties::

Description
-----------

This algorithm performs a **deep copy** of all of the information in the
workspace. It maintains events if the input is an
:ref:`EventWorkspace <EventWorkspace>`. It will call CloneMDWorkspace for a
`MDEventWorkspace <http://www.mantidproject.org/MDEventWorkspace>`_ or a
:ref:`MDHistoWorkspace <MDHistoWorkspace>`. It can also clone a
`PeaksWorkspace <http://www.mantidproject.org/PeaksWorkspace>`_.

Assignment of Workspaces Explained
=============================

For value types in python, performing *a = b* results in *a* becoming a unique copy of *b*. The assignment (operator =) results in a full deep copy of *b*. For example:

.. code-block:: python

   a = int(1)
   b = a
   
   print a, b
   
   a = float(2)
   
   print a, b
   # prints 2.0, 1. Note that b is independent of a.

However, this is not a general rule in python. Many objects, particularly those which are expensive to copy, both in terms of time and memory, are not copied unless explicitly asked. 
Mantid workspaces are examples of this. **If you attempt to assign from a mantid workspace in python, you will get a reference (alias) to the original.** If your desired behavior 
is **to get a complete copy of the original workspace, use CloneWorkspace instead**


Usage
-----

The following illustrate some usage examples in python.

**Example - Creating and then clone a workspace**

.. testcode:: CloneAlgorithmExample1
   
   # Create a workspace
   ws = CreateWorkspace(DataX=[1,2,3], DataY=[1,0])
   # Clone it
   clone = CloneWorkspace(ws)

   print 'Number of histograms in original ', ws.getNumberHistograms()
   print 'Number of histograms in clone ', clone.getNumberHistograms()
   print 'Number of bins in original ', ws.blocksize()
   print 'Number of bins in clone ', clone.blocksize()

   # Delete the original
   DeleteWorkspace(ws)

   if mtd.doesExist(clone.name()):
       print 'The clone was not deleted when the original was deleted'
   else: 
       print 'The clone was deleted when the original was deleted'

Output:
   
.. testoutput:: CloneAlgorithmExample1

   Number of histograms in original  1
   Number of histograms in clone  1
   Number of bins in original  2
   Number of bins in clone  2
   The clone was not deleted when the original was deleted

**Example - Clone shorthand**

.. testcode:: CloneAlgorithmExample2

   # Create a workspace
   ws = CreateWorkspace(DataX=[1,2,3], DataY=[1,0])
   # Clone it
   clone = ws.clone()


**Example - Clone vs Alias**

.. testcode:: CloneAlgorithmExample3

   def does_exist(workspace_name):
       return mtd.doesExist(workspace_name)

   # Create a workspace
   ws = CreateWorkspace(DataX=[1,2,3], DataY=[1,0])
   # Alias it
   alias = ws
   # Clone it
   clone = ws.clone()
   
   # Cache names to test against later
   ws_name = ws.name()
   clone_name = clone.name()
   alias_name = alias.name()
   
   # Delete the original
   DeleteWorkspace(ws)

   print "Does the original still exist: ",  str(does_exist(ws_name)) 
   print "Does the clone still exist: ",  str(does_exist(clone_name))
   print "Does the alias still exist: ",  str(does_exist(alias_name))

Output:
   
.. testoutput:: CloneAlgorithmExample3

   Does the original still exist:  False
   Does the clone still exist:  True
   Does the alias still exist:  False
   
.. categories::