find_package(Python3 COMPONENTS Interpreter)
find_path (IPP_ROOT_DIR
        "include/ipp.h"
    HINTS
        ${Python3_SITELIB}/../../..
        ${VCTR_IPP_ROOT}
        $ENV{IPPROOT}
        $ENV{HOME}/intel/ipp
        $ENV{HOME}/intel/oneapi/ipp/latest
        $ENV{HOME}/.local/
        /usr/local/
        /opt/intel/oneapi/ipp/latest
        /opt/intel/compilers_and_libraries/linux/ipp
        /opt/intel/compilers_and_libraries/mac/ipp
        "C:\\Python310\\"
        "C:\\Python311\\"
        "C:\\IntelSWTools\\compilers_and_libraries\\windows\\ipp"
        "C:\\Program Files (x86)\\Intel\\oneAPI\\ipp\\latest"
        "C:\\Program Files (x86)\\IntelSWTools\\compilers_and_libraries\\windows\\ipp"
    REQUIRED)

find_library (IPPCORE 
    NAMES 
        ippcore.lib 
        ippcoremt.lib
        libippcore.a
    PATHS
        ${IPP_ROOT_DIR}/lib
        ${IPP_ROOT_DIR}/Lib
        ${IPP_ROOT_DIR}/intel64/
        ${IPP_ROOT_DIR}/Library/lib
        /usr/local/lib
    REQUIRED)

find_library (IPPS
    NAMES 
        ipps.lib 
        ippsmt.lib
        libipps.a
    PATHS
        ${IPP_ROOT_DIR}/lib
        ${IPP_ROOT_DIR}/Lib
        ${IPP_ROOT_DIR}/intel64/
        ${IPP_ROOT_DIR}/Library/lib
        /usr/local/lib
    REQUIRED)

find_library (IPPVM
    NAMES 
        ippvm.lib 
        ippvmmt.lib
        libippvm.a
    PATHS
        ${IPP_ROOT_DIR}/lib
        ${IPP_ROOT_DIR}/Lib
        ${IPP_ROOT_DIR}/intel64/
        ${IPP_ROOT_DIR}/Library/lib
        /usr/local/lib
    REQUIRED)

add_library (ipp INTERFACE)
target_include_directories (ipp INTERFACE ${IPP_ROOT_DIR}/include)
target_link_libraries (ipp INTERFACE ${IPPS} ${IPPVM} ${IPPCORE})
target_compile_definitions (ipp INTERFACE VCTR_USE_IPP=1)
message (STATUS "Linking against IPP")