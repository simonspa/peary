# Retrieve the version string from git describe
FUNCTION(get_version PROJECT_VERSION)
    # Check if this is a source tarball build
    IF(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.git)
        SET(SOURCE_PACKAGE 1)
    ENDIF(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.git)

    # Set package version
    IF(NOT SOURCE_PACKAGE)
        SET(TAG_FOUND FALSE)

        # Get the version from last git tag plus number of additional commits:
        FIND_PACKAGE(Git QUIET)
        IF(GIT_FOUND)
            EXEC_PROGRAM(git ${CMAKE_CURRENT_SOURCE_DIR} ARGS describe --tags HEAD OUTPUT_VARIABLE ${PROJECT_VERSION})
            STRING(REGEX REPLACE "^release-" "" ${PROJECT_VERSION} ${${PROJECT_VERSION}})
            STRING(REGEX REPLACE "([v0-9.]+)-([0-9]+)-([A-Za-z0-9]+)" "\\1+\\2^\\3" ${PROJECT_VERSION} ${${PROJECT_VERSION}})
            EXEC_PROGRAM(git ARGS status --porcelain ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE PROJECT_STATUS)
            IF(PROJECT_STATUS STREQUAL "")
                MESSAGE(STATUS "Git project directory is clean.")
            ELSE(PROJECT_STATUS STREQUAL "")
                MESSAGE(STATUS "Git project directory is dirty:\n ${PROJECT_STATUS}.")
            ENDIF(PROJECT_STATUS STREQUAL "")

            # Check if commit flag has been set by the CI:
            IF(DEFINED ENV{CI_COMMIT_TAG})
                MESSAGE(STATUS "Found CI tag flag, building tagged version")
                SET(TAG_FOUND TRUE)
            ENDIF()
        ELSE(GIT_FOUND)
            MESSAGE(STATUS "Git repository present, but could not find git executable.")
        ENDIF(GIT_FOUND)
    ELSE(NOT SOURCE_PACKAGE)
        # If we don't have git we can not really do anything
        MESSAGE(STATUS "Source tarball build - no repository present.")
        SET(TAG_FOUND TRUE)
    ENDIF(NOT SOURCE_PACKAGE)

    # Set the project version in the parent scope
    SET(TAG_FOUND ${TAG_FOUND} PARENT_SCOPE)

    # Set the project version in the parent scope
    SET(${PROJECT_VERSION} ${${PROJECT_VERSION}} PARENT_SCOPE)
ENDFUNCTION()
