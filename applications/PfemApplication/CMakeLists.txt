set(CMAKE_INCLUDE_CURRENT_DIR ON)

message("**** configuring KratosPfemApplication ****")

################### PYBIND11
include(pybind11Tools)

include_directories( ${CMAKE_SOURCE_DIR}/kratos )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../DelaunayMeshingApplication )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../SolidMechanicsApplication )

## Set a default value for triangle and tetgen in case the user does not set it
## to avoid problems with the define linkage block. By default we will compile the lib

if(NOT (${DELAUNAY_MESHING_APPLICATION} MATCHES "ON"))
    message(SEND_ERROR "The Delaunay Meshing Application must be compiled before this one. Set DELAUNAY_MESHING_APPLICATION=ON in the configure file.")
endif(NOT (${DELAUNAY_MESHING_APPLICATION} MATCHES "ON"))

if(NOT (${SOLID_MECHANICS_APPLICATION} MATCHES "ON"))
    message(SEND_ERROR "The Solid Mechanics Application must be compiled before this one. Set SOLID_MECHANICS_APPLICATION=ON in the configure file.")
endif(NOT (${SOLID_MECHANICS_APPLICATION} MATCHES "ON"))

if(NOT DEFINED ${INCLUDE_TRIANGLE})
  set(INCLUDE_TRIANGLE ON)
  set(TRIANGLE_INSTALL_DIR ${CMAKE_SOURCE_DIR}/external_libraries/triangle/)
  include_directories( ${TRIANGLE_INSTALL_DIR} )
endif(NOT DEFINED ${INCLUDE_TRIANGLE})

if(NOT DEFINED ${INCLUDE_TETGEN})
  set(INCLUDE_TETGEN ON)
  set(TETGEN_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../DelaunayMeshingApplication/external_modules/tetgen/)
  include_directories( ${TETGEN_INSTALL_DIR} )
  link_directories( ${TETGEN_INSTALL_DIR} )
endif(NOT DEFINED ${INCLUDE_TETGEN})

## Generate variables with the sources
set( KRATOS_PFEM_APPLICATION_CORE
	${CMAKE_CURRENT_SOURCE_DIR}/pfem_application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pfem_application_variables.cpp

	##elements
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/fluid_elements/fluid_element.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_elements/fluid_elements/updated_lagrangian_segregated_fluid_element.cpp

)

## Generate variables with the sources
set( KRATOS_PFEM_APPLICATION_PYTHON_INTERFACE
        # Custom python
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/pfem_python_application.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/custom_python/add_custom_processes_to_python.cpp
)

add_library(KratosPfemCore SHARED ${KRATOS_PFEM_APPLICATION_CORE})
target_link_libraries(KratosPfemCore PUBLIC KratosCore KratosDelaunayMeshingCore KratosSolidMechanicsCore)
set_target_properties(KratosPfemCore PROPERTIES COMPILE_DEFINITIONS "PFEM_APPLICATION=EXPORT,API")

###############################################################
## define library Kratos which defines the basic python interface
pybind11_add_module(KratosPfemApplication MODULE THIN_LTO ${KRATOS_PFEM_APPLICATION_PYTHON_INTERFACE})
target_link_libraries(KratosPfemApplication PRIVATE KratosPfemCore)
set_target_properties(KratosPfemApplication PROPERTIES PREFIX "")

# get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
# message("TestApplication subdir inc_dirs = ${inc_dirs}")

# changing the .dll suffix to .pyd
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set_target_properties(KratosPfemApplication PROPERTIES SUFFIX .pyd)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# changing the .dylib suffix to .so
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties(KratosPfemApplication PROPERTIES SUFFIX .so)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${INSTALL_PYTHON_FILES} MATCHES ON)
  get_filename_component (CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python_scripts DESTINATION applications/${CURRENT_DIR_NAME}  FILES_MATCHING PATTERN "*.py"  PATTERN ".svn" EXCLUDE)
endif(${INSTALL_PYTHON_FILES} MATCHES ON)

# get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
# message("PfemApplication subdir inc_dirs = ${inc_dirs}")

if(USE_COTIRE MATCHES ON)
    cotire(KratosPfemCore)
    cotire(KratosPfemApplication)
endif(USE_COTIRE MATCHES ON)

install(TARGETS KratosPfemCore DESTINATION libs )
install(TARGETS KratosPfemApplication DESTINATION libs )

# Add to the KratosMultiphisics Python module
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/PfemApplication.py" DESTINATION "KratosMultiphysics/PfemApplication" RENAME "__init__.py")
