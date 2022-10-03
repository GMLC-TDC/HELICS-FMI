# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(solver_headers
    solverInterface.h
    sundialsInterface.h
    sundialsMatrixData.h
    basicOdeSolver.h
    solverMode.hpp
    cvodeInterface.h
    arkodeInterface.h
    solver_definitions.hpp
    SolvableObject.hpp
)

set(solver_sources
    ${solver_headers}
    solverInterface.cpp
    sundialsMatrixData.cpp
    sundialsMatrixDataDense.cpp
    sundialsMatrixDataSparseRow.cpp
    sundialsMatrixDataSparseColumn.cpp
    sundialsInterface.cpp
    basicOdeSolver.cpp
    cvodeInterface.cpp
    arkodeInterface.cpp
    solverMode.cpp
)

add_library(solvers STATIC ${solver_sources} ${solver_headers})

target_link_libraries(solvers PUBLIC utilities sundials_cvode sundials_arkode)
if(ENABLE_KLU)
    target_link_libraries(solvers PUBLIC SuiteSparse::klu)
endif(ENABLE_KLU)

get_target_property(OUT sundials_cvode LINK_LIBRARIES)
message(STATUS ${OUT})