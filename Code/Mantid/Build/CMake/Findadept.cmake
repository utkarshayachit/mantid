# Try to find the adept library (header file, library for linking)
#
# See 
# http://www.met.rdg.ac.uk/clouds/adept/
#
# Once run this will define: 
# 
# adept_FOUND       = adept library was found
#
# adept_LIBRARIES   = full path to the libraries
#
# adept_INCLUDE_DIR      = where to find headers 
#
# Michael Wedel 07/2014
#
# http://psi.ch/mss

FIND_PATH(adept_INCLUDE_DIR
    NAMES adept.h
    DOC "Include directory where adept.h is located"
    )
  
FIND_LIBRARY(adept_LIBRARIES
    NAMES adept
    DOC "adept dynamic library file"
    )
    
mark_as_advanced ( adept_INCLUDE_DIR adept_LIBRARIES )
    
IF(adept_LIBRARIES AND adept_INCLUDE_DIR)
    SET(adept_FOUND 1)
ENDIF()