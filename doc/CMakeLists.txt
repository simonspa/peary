#############################################
# Doxygen target to generate API reference  #
#############################################

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/reference/Doxyfile.in ${CMAKE_BINARY_DIR}/doc/reference/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(reference
    ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/reference/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc/reference
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
ENDIF(DOXYGEN_FOUND)
