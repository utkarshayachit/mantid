##
## mantid.geometry tests
##
set ( TEST_PY_FILES
  BoundingBoxTest.py
  IComponentTest.py
  InstrumentTest.py
  ObjectTest.py
  OrientedLatticeTest.py
  RectangularDetectorTest.py
  ReferenceFrameTest.py
  UnitCellTest.py
  PointGroupTest.py
  SpaceGroupTest.py
  SymmetryOperationTest.py
)

check_tests_valid ( ${CMAKE_CURRENT_SOURCE_DIR} ${TEST_PY_FILES} )

# Prefix for test=PythonInterfaceGeometry
pyunittest_add_test ( ${CMAKE_CURRENT_SOURCE_DIR} PythonInterfaceGeometry ${TEST_PY_FILES} )
