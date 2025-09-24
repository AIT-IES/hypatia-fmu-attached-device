# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os, subprocess

REQUIRED_BOOST_LIBS = ['filesystem']

def required_boost_libs( conf ):
    for lib in REQUIRED_BOOST_LIBS:
        if lib not in conf.env[ 'REQUIRED_BOOST_LIBS' ]:
            conf.env[ 'REQUIRED_BOOST_LIBS' ] += REQUIRED_BOOST_LIBS

def configure( conf ):
    # Check if Boost is abailable.
    if not conf.env['BOOST_VERSION']:
        abort_configuration( conf, 'A Boost installation is required for this module.' )
        return

    # Retrieve path to FMI++ source directory.
    fmipp_include_path = os.path.join( conf.path.abspath(), 'fmipp' )

    # Path to shared libraries.
    fmipp_lib_path = os.path.join( conf.path.abspath(), 'lib' )

    # Create path if necessary.
    if not os.path.isdir( fmipp_lib_path ): os.mkdir( fmipp_lib_path )

    # Check which C and C++ compiler waf uses.
    conf_c_compiler = conf.env[ 'CC' ]
    if ( 1 != len( conf_c_compiler ) ):
        # There seems to be a problem with the configuration ...
        abort_configuration( conf, 'Unable to retrieve C compiler from configuration.' )
        return
    c_compiler = conf_c_compiler[0]

    conf_cxx_compiler = conf.env[ 'CXX' ]
    if ( 1 != len( conf_cxx_compiler ) ):
        # There seems to be a problem with the configuration ...
        abort_configuration( conf, 'Unable to retrieve C++ compiler from configuration.' )
        return
    cxx_compiler = conf_cxx_compiler[0]

    # Issue debug message.
    conf.msg( "Checking for FMI++ location", ( "%s (given)" % fmipp_include_path ) )
    conf.msg( "Checking for FMI++ library path", fmipp_lib_path )
    conf.msg( "Checking for C compiler", c_compiler )
    conf.msg( "Checking for C++ compiler", cxx_compiler )

    # Define command for compiling shared libraries from FMI++ code.
    compile_fmipp_cmd = 'cmake %s -DCMAKE_C_COMPILER=%s -DCMAKE_CXX_COMPILER=%s && make' % ( fmipp_include_path, c_compiler, cxx_compiler )

    # Compile shared libraries from FMI++ code.
    exit_code = subprocess.call( compile_fmipp_cmd, shell=True, cwd=fmipp_lib_path )
    if ( 0 != exit_code ):
        # Compilation failed, abort configuration.
        abort_configuration( conf, 'Failed to build shared libraries.' )
        return

    # Define compiler and linker flags specific to FMI++.
    conf.env.append_value( 'CXXFLAGS', [ '-I' + fmipp_include_path ] )
    #conf.env.append_value( 'CXXDEFINES', [ '-D...' ] )
    
    # Add specific linker flags.
    conf.env.append_value( 'LINKFLAGS', [
        '-Wl,--no-as-needed', # Needed on some Linux variants.
        '-lfmippim', # Add shared library implementing the back-end.
        '-lboost_filesystem'
        ] )

    # Add directory containing the shared library implementing the back-end to the list of module paths.
    conf.env.append_value( 'NS3_MODULE_PATH', [ os.path.join( fmipp_lib_path, 'import' ) ] )
    
    # Add all module paths to the linker flags.
    for p in conf.env[ 'NS3_MODULE_PATH' ]:
        conf.env.append_value( 'LINKFLAGS', [ '-L' + p, '-Wl,-rpath,' + p ] )
    
    conf.report_optional_feature( 'fmu-attached-device', 'FMU-Attached Device', True, '' )


def build(bld):
    module = bld.create_ns3_module('fmu-attached-device', ['core', 'basic-sim'])
    module.source = [
        'model/device-client.cc',
        'model/fmu-attached-device.cc',
        'model/fmu-shared-device.cc',
        'model/payload.cc',
        'model/processing-time.cc',
        'helper/device-client-factory.cc',
        'helper/device-client-helper.cc',
        'helper/factory-util.cc',
        'helper/fmu-attached-device-factory.cc',
        'helper/fmu-shared-device-factory.cc',
        ]

    # module_test = bld.create_ns3_module_test_library('fmu-attached-device')
    # module_test.source = [
    #     'test/fmipp-test-suite.cc',
    #     ]

    headers = bld(features='ns3header')
    headers.module = 'fmu-attached-device'
    headers.source = [
        'model/device-client.h',
        'model/fmu-attached-device.h',
        'model/fmu-shared-device.h',
        'model/payload.h',
        'model/processing-time.h',
        'helper/device-client-factory.h',
        'helper/device-client-helper.h',
        # 'helper/factory-util.h',
        'helper/fmu-attached-device-factory.h',
        'helper/fmu-shared-device-factory.h',
        'helper/fmu-device-helper.h',
        'helper/fmu-util.h',
        ]

    # if bld.env.ENABLE_EXAMPLES:
    #    bld.recurse('examples')

    ## bld.ns3_python_bindings()


def abort_configuration( conf, message ):
    conf.msg( 'Configuring FMI++ Library', message )
    conf.report_optional_feature( 'fmu-attached-device', 'FMU Attached Devices', False, message )
    # Add this module to the list of modules that won't be built if they are enabled.
    conf.env[ 'MODULES_NOT_BUILT' ].append( 'fmu-attached-device' )
