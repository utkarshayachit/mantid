.. algorithm::

.. summary::

.. alias::

.. properties::

Description
-----------

LoadReflTBl is loads ascii files in Reflectometry TBL format into a
tableworkspace. Format accepted is strict to only allow 17 columns of
data.

The ascii format contains up to runs per row. The in-memory table workspace puts each run on its own row, with a 'StitchGroup' id used to determine which runs should be 
stitched together. The columns in the table workspace schema are **Run(s),ThetaIn,TransRun(s),Qmin,Qmax,dq/q,Scale,StitchGroup** in that order.

Usage
-----

**Example - Simple loading**

.. testcode:: LoadReflTBL1

   table = Load(Filename="INTER_NR.tbl")

   first_row =  table.row(0)
   second_row =  table.row(1)

   print "Rows:", table.rowCount()
   print "Columns:", table.columnCount()
   print "Column names:", table.getColumnNames()
   print "First run:", first_row['Run(s)']
   print "Second run:", second_row['Run(s)']

Output
------

.. testoutput:: LoadReflTBL1

   Rows: 2
   Columns: 8
   Column names: ['Run(s)','ThetaIn','TransRun(s)','Qmin','Qmax','dq/q','Scale','StitchGroup']
   First run: 13460
   Second run: 13462


.. categories:: LoadReflTBL1
