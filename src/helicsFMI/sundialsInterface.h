/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#pragma once

#include "solverInterface.h"
// SUNDIALS libraries
#include "nvector/nvector_serial.h"

#define NVECTOR_DESTROY(vec) N_VDestroy_Serial (vec)
#define NVECTOR_NEW(size) N_VNew_Serial (size)
#define NVECTOR_DATA(vec) NV_DATA_S (vec)

#include <sundials/sundials_linearsolver.h>
#include <sundials/sundials_types.h>

#define ONE RCONST (1.0)
#define ZERO RCONST (0.0)

#define MEASURE_TIMINGS 0

#define _unused(x) ((void)(x))

namespace helics_fmi
{
namespace solvers
{
void sundialsErrorHandlerFunc (int error_code,
                               const char *module,
                               const char *function,
                               char *msg,
                               void *user_data);

/** brief abstract base class for SUNDIALS based SolverInterface objects doesn't really do anything on its own
just provides common functionality to SUNDIALS SolverInterface objects
*/
class sundialsInterface : public OdeSolverInterface
{
  protected:
    bool use_omp = false;  //!< helper variable to handle omp functionality
    N_Vector state = nullptr;  //!< state vector
    N_Vector dstate_dt = nullptr;  //!< dstate_dt information
    N_Vector abstols = nullptr;  //!< tolerance vector
    N_Vector consData = nullptr;  //!< constraint type Vector
    N_Vector scale = nullptr;  //!< scaling vector
    void *solverMem = nullptr;  //!< the memory used by a specific solver internally
    FILE *m_sundialsInfoFile = nullptr;  //!< direct file reference for input to the solver itself
    SUNLinearSolver LS = nullptr;  //!< the link to the linear solver to use
  public:
    sundialsInterface ();
    /** @brief constructor loading the SolverInterface structure*
    @param[in] gds  the gridDynSimulation to link with
    @param[in] sMode the solverMode for the solver
    */
    explicit sundialsInterface (FmiModelExchangeFederate *fed);
    /** @brief destructor
     */
    virtual ~sundialsInterface ();

    virtual std::unique_ptr<OdeSolverInterface > clone (bool fullCopy = false) const override;

    virtual void cloneTo (OdeSolverInterface  *si, bool fullCopy = false) const override;
    virtual double *state_data () noexcept override;
    virtual double *deriv_data () noexcept override;
    virtual const double *state_data () const noexcept override;
    virtual const double *deriv_data () const noexcept override;
    virtual void allocate (int stateCount, int numRoots) override;
    virtual double get (const std::string &param) const override;

    /** @brief get the dedicated memory space of the solver
    @return a void pointer to the memory location of the solver specific memory
    */
    void *getSolverMem () const { return solverMem; }

};

}  // namespace solvers
}  // namespace helics_fmi
