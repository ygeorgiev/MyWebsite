cmake_minimum_required(VERSION 2.6)

# Start
project(hwm)

# Setup toolchain
SET (CMAKE_C_FLAGS                "-Wall -std=c99")
SET (CMAKE_C_FLAGS_DEBUG          "-g")
SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
SET (CMAKE_CXX_FLAGS                "-Wall")
SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Install-dirs
set(INSTALLBIN bin)
set(INSTALLLIB lib)
set(INSTALLARC lib)

# Generate config.h
configure_file("config.h.in" "config.h")
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

# Continue with the building
#This hack is because for some reason GTK2_LIBS does not include gio
set(GIO_HACK "-lgio-2.0 -lgobject-2.0")

include(FindGTK2)

include_directories(${GTK2_INCLUDE_DIRS} ${LIBNOTIFY_INCLUDE_DIRS})

# Add targets
add_executable(mysite main.c)
target_link_libraries(mysite ${GTK2_GLIB_LIBRARY} ${GIO_HACK})
install(TARGETS mysite BINARY DESTINATION bin)





# Add support for "make dist"
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | gzip > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.gz
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add support for "make check"
add_custom_target(check COMMAND ctest "-v")
# Test Suite
#add_executable(<testprog> EXCLUDE_FROM_ALL ...)
#add_test(<testprog> <testprog>)
#add_dependencies(check <testprog>)
