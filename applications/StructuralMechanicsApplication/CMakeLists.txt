set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("**** configuring KratosStructuralMechanicsApplication ****")

################### PYBIND11
include(pybind11Tools)

include_directories( ${CMAKE_SOURCE_DIR}/kratos )

## StructuralMechanics Core sources
file(GLOB_RECURSE KRATOS_STRUCTURAL_MECHANICS_APPLICATION_CORE
    ${CMAKE_CURRENT_SOURCE_DIR}/structural_mechanics_application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/structural_mechanics_application_variables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_conditions/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_constitutive/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_io/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_processes/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_response_functions/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_strategies/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/custom_utilities/*.cpp
)

list(REMOVE_ITEM KRATOS_STRUCTURAL_MECHANICS_APPLICATION_CORE ${CMAKE_CURRENT_SOURCE_DIR}/custom_utilities/constitutive_law_utilities.cpp)
list(INSERT KRATOS_STRUCTURAL_MECHANICS_APPLICATION_CORE 0 ${CMAKE_CURRENT_SOURCE_DIR}/custom_utilities/constitutive_law_utilities.cpp)

## StructuralMechanics testing sources
if(${KRATOS_BUILD_TESTING} MATCHES ON)
    file(GLOB_RECURSE KRATOS_STRUCTURAL_MECHANICS_APPLICATION_TESTING_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
endif(${KRATOS_BUILD_TESTING} MATCHES ON)

## StructuralMechanics python interface sources
file(GLOB_RECURSE KRATOS_STRUCTURAL_MECHANICS_APPLICATION_PYTHON_INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/custom_python/*.cpp)

add_library(KratosStructuralMechanicsCore SHARED ${KRATOS_STRUCTURAL_MECHANICS_APPLICATION_CORE} ${KRATOS_STRUCTURAL_MECHANICS_APPLICATION_TESTING_SOURCES})
target_link_libraries(KratosStructuralMechanicsCore PUBLIC KratosCore)
set_target_properties(KratosStructuralMechanicsCore PROPERTIES COMPILE_DEFINITIONS "STRUCTURAL_MECHANICS_APPLICATION=EXPORT,API")

###############################################################
## define library Kratos which defines the basic python interface
pybind11_add_module(KratosStructuralMechanicsApplication MODULE THIN_LTO ${KRATOS_STRUCTURAL_MECHANICS_APPLICATION_PYTHON_INTERFACE})
target_link_libraries(KratosStructuralMechanicsApplication PUBLIC KratosStructuralMechanicsCore)
set_target_properties(KratosStructuralMechanicsApplication PROPERTIES PREFIX "")

# changing the .dll suffix to .pyd
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set_target_properties(KratosStructuralMechanicsApplication PROPERTIES SUFFIX .pyd)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# changing the .dylib suffix to .so
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(KratosStructuralMechanicsApplication PROPERTIES SUFFIX .so)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${INSTALL_PYTHON_FILES} MATCHES ON)
  get_filename_component (CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python_scripts DESTINATION applications/${CURRENT_DIR_NAME} FILES_MATCHING PATTERN "*.py")
endif(${INSTALL_PYTHON_FILES} MATCHES ON)

# Kratos Testing. Install everything except sources to ensure that reference and configuration files are copied.
if(${INSTALL_TESTING_FILES} MATCHES ON )
    get_filename_component (CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests DESTINATION applications/${CURRENT_DIR_NAME}
        PATTERN "*.git" EXCLUDE
        PATTERN "*.c" EXCLUDE
        PATTERN "*.h" EXCLUDE
        PATTERN "*.cpp" EXCLUDE
        PATTERN "*.hpp" EXCLUDE
  )
endif(${INSTALL_TESTING_FILES} MATCHES ON)

if(USE_COTIRE MATCHES ON)
    cotire(KratosStructuralMechanicsCore)
    cotire(KratosStructuralMechanicsApplication)
endif(USE_COTIRE MATCHES ON)

install(TARGETS KratosStructuralMechanicsCore DESTINATION libs )
install(TARGETS KratosStructuralMechanicsApplication DESTINATION libs )

# Add to the KratosMultiphisics Python module
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/StructuralMechanicsApplication.py" DESTINATION "KratosMultiphysics/StructuralMechanicsApplication" RENAME "__init__.py")
