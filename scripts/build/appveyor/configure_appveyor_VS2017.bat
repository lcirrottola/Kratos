cmake .. -G "Visual Studio 15 2017 Win64"                                       ^
-DCMAKE_CXX_FLAGS="/D EXCLUDE_EMBEDDED_PYTHON_DEBUG" 	                        ^
-DCMAKE_C_FLAGS="/D EXCLUDE_EMBEDDED_PYTHON_DEBUG" 		                        ^
-DBOOST_ROOT="C:\Libraries\boost_1_65_1"                                        ^
-DPYTHON_EXECUTABLE="C:\Python36-x64\python.exe"		 	                    ^
-DCMAKE_BUILD_TYPE="Debug"  							                        ^
-DDEM_APPLICATION=ON                                                            ^
-DEXTERNAL_SOLVERS_APPLICATION=OFF                                              ^
-DFLUID_DYNAMICS_APPLICATION=ON                                                 ^
-DSTRUCTURAL_MECHANICS_APPLICATION=ON                                           ^
-DCONTACT_STRUCTURAL_MECHANICS_APPLICATION=ON                                   ^
-DSWIMMING_DEM_APPLICATION=ON                                                   ^
-DMESH_MOVING_APPLICATION=ON                                                    ^
-DSOLID_MECHANICS_APPLICATION=ON                                                ^
-DCONSTITUTIVE_MODELS_APPLICATION=ON                                            ^
-DDELAUNAY_MESHING_APPLICATION=ON                                               ^
-DCONTACT_MECHANICS_APPLICATION=ON                                              ^
-DPFEM_APPLICATION=ON                                                           ^
-DPFEM_SOLID_MECHANICS_APPLICATION=ON                                           ^
-DPFEM_FLUID_DYNAMICS_APPLICATION=ON                                            ^
-DMETIS_APPLICATION=OFF                                                         ^
-DPARMETIS_ROOT_DIR="UNSET"                                                     ^
-DTRILINOS_APPLICATION=OFF                                                      ^
-DTRILINOS_ROOT="UNSET"                                                         ^
-DINSTALL_EMBEDDED_PYTHON=ON                                                    ^
-DINCLUDE_FEAST=OFF                                                             ^
-DUSE_COTIRE=ON