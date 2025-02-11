# -------------------------------------------------------------------
# Copyright (c) 2013-2020, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMIPP_LICENSE for details.
# -------------------------------------------------------------------

cmake_minimum_required( VERSION 3.8 )


project( fmipp )


# include the sundials library and choose use an integrator from CVode
option( INCLUDE_SUNDIALS "Use SUNDIALS integrator suite." OFF )
if ( INCLUDE_SUNDIALS )
   message( "ATTENTION: Sundials will be included!" )

   set( SUNDIALS_INCLUDEDIR "" CACHE PATH "Optional: Sundials include directory." )
   include_directories( ${SUNDIALS_INCLUDEDIR} )

   set( SUNDIALS_LIBRARYDIR "" CACHE PATH "Optional: Sundials library directory." )
   link_directories( ${SUNDIALS_LIBRARYDIR} )

   add_definitions( -DUSE_SUNDIALS )
endif ()


# set the name of the FMU-binaries-subdirectory according to the current OS
if ( WIN32 )
   if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set( FMU_BIN_DIR "win64" )
   else ()
      set( FMU_BIN_DIR "win32" )
   endif ()
   set( FMU_BIN_EXT ".dll" )
elseif ( APPLE )
   if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set( FMU_BIN_DIR "darwin64" )
   else ()
      set( FMU_BIN_DIR "darwin32" )
   endif ()
   set( FMU_BIN_EXT ".dylib" )
elseif ( CYGWIN )
   if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set( FMU_BIN_DIR "cygwin64" )
   else ()
      set( FMU_BIN_DIR "cygwin32" )
   endif ()
   set( FMU_BIN_EXT ".dll" )
elseif ( UNIX )
   if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set( FMU_BIN_DIR "linux64" )
   else ()
      set( FMU_BIN_DIR "linux32" )
   endif ()
   set( FMU_BIN_EXT ".so" )
endif ()

add_definitions( -DFMU_BIN_DIR="${FMU_BIN_DIR}" )
add_definitions( -DFMU_BIN_EXT="${FMU_BIN_EXT}" )

# OS-specific compiler settings
if ( WIN32 ) # windows-specific
   set( PATH_SEPARATOR ";" )
   set( CMAKE_SHARED_LIBRARY_PREFIX "" )
   set( CMAKE_SHARED_MODULE_PREFIX "" )
   add_definitions( -DWIN32 )
   if ( MINGW )
      add_definitions( -DMINGW -Wall -D_hypot=hypot ) # -O1
      if ( CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" )
         set( CMAKE_CXX_FLAGS "-std=c++0x" )
      else ()
         set( CMAKE_CXX_FLAGS "-std=gnu++11" )
      endif ()
   endif ()
   if ( MSVC )
      # disable auto-linking and use cmake's dependency handling
      add_definitions( -DBOOST_ALL_NO_LIB /W3 )
   endif ()
else () # linux-specific
   set( PATH_SEPARATOR ":" )
   add_definitions( -Wall ) # -O1
      if ( CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" )
         set( CMAKE_CXX_FLAGS "-std=c++0x" )
      else ()
         set( CMAKE_CXX_FLAGS "-std=c++11" )
      endif ()
endif ()


# Boost dependencies here
find_package( Boost 1.71.0 REQUIRED )
if ( Boost_FOUND )
   include_directories( ${Boost_INCLUDE_DIRS} )
   set( BOOST_LIBRARYDIR ${Boost_LIBRARY_DIRS} )

   option( BOOST_STATIC_LINKING "Turn on/off static linking for Boost" OFF )
   if ( BOOST_STATIC_LINKING )
      set( Boost_USE_STATIC_LIBS ON ) # ON when using static linking.
      if ( WIN32 )
        set( Boost_LIB_PREFIX "lib" )
      endif ()
      message( "ATTENTION: Use static linking for Boost libraries!" )
   else ()
      set( Boost_USE_STATIC_LIBS OFF )
   endif ()

   set( Boost_USE_MULTITHREADED ON )
   set( Boost_USE_STATIC_RUNTIME OFF )
endif ()


# common include directories
include_directories( ${fmipp_SOURCE_DIR} )
include_directories( ${fmipp_SOURCE_DIR}/common )
include_directories( ${fmipp_SOURCE_DIR}/common/fmi_v1.0 )


# FMI++ import library
add_subdirectory( import )
