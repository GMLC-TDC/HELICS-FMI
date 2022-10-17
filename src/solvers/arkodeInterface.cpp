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

#include "arkodeInterface.h"

#include "gmlc/utilities/stringOps.h"
#include "gmlc/utilities/vectorOps.hpp"

#include <arkode/arkode.h>
#include <arkode/arkode_erkstep.h>
#include <cstdio>

#ifdef KLU_ENABLE
#    include <sunlinsol/sunlinsol_klu.h>
#endif

#include <cassert>
#include <map>
#include <sunlinsol/sunlinsol_dense.h>
#include <vector>

namespace griddyn {
namespace solvers {
    int arkodeFunc(realtype time, N_Vector state, N_Vector dstate_dt, void* user_data);
    int arkodeJac(realtype time,
                  N_Vector state,
                  N_Vector dstate_dt,
                  SUNMatrix J,
                  void* user_data,
                  N_Vector tmp1,
                  N_Vector tmp2,
                  N_Vector tmp3);

    int arkodeRootFunc(realtype time, N_Vector state, realtype* gout, void* user_data);

    arkodeInterface::arkodeInterface(const std::string& objName): sundialsInterface(objName)
    {
        mode.dynamic = true;
        mode.differential = true;
        mode.algebraic = false;
    }

    arkodeInterface::arkodeInterface(SolvableObject* solveObj, const solverMode& sMode):
        sundialsInterface(solveObj, sMode)
    {
        mode.dynamic = true;
        mode.differential = true;
        mode.algebraic = false;
    }

    arkodeInterface::~arkodeInterface()
    {
        // clear variables for CVode to use
        if (flags[initialized_flag]) {
            ERKStepFree(&solverMem);
        }
    }

    std::unique_ptr<SolverInterface> arkodeInterface::clone(bool fullCopy) const
    {
        std::unique_ptr<SolverInterface> si = std::make_unique<arkodeInterface>();
        arkodeInterface::cloneTo(si.get(), fullCopy);
        return si;
    }

    void arkodeInterface::cloneTo(SolverInterface* si, bool fullCopy) const
    {
        sundialsInterface::cloneTo(si, fullCopy);
        auto ai = dynamic_cast<arkodeInterface*>(si);
        if (ai == nullptr) {
            return;
        }
        ai->maxStep = maxStep;
        ai->minStep = minStep;
        ai->step = step;
    }

    void arkodeInterface::allocate(count_t stateCount, count_t numRoots)
    {
        // load the vectors
        if (stateCount == svsize) {
            return;
        }
        flags.reset(initialized_flag);

        a1.setRowLimit(stateCount);
        a1.setColLimit(stateCount);

        // update the rootCount
        rootCount = numRoots;
        rootsfound.resize(numRoots);

        // allocate the solverMemory
        if (solverMem != nullptr) {
            ERKStepFree(&(solverMem));
        }

        sundialsInterface::allocate(stateCount, numRoots);
    }

    void arkodeInterface::setMaxNonZeros(count_t nonZeroCount)
    {
        maxNNZ = nonZeroCount;
        a1.reserve(nonZeroCount);
        a1.clear();
    }

    void arkodeInterface::set(const std::string& param, const std::string& val)
    {
        if (param.empty()) {
        } else {
            sundialsInterface::set(param, val);
        }
    }

    void arkodeInterface::set(const std::string& param, double val)
    {
        bool checkStepUpdate = false;
        if (param == "step") {
            if ((maxStep < 0) || (maxStep == step)) {
                maxStep = val;
            }
            if ((minStep < 0) || (minStep == step)) {
                minStep = val;
            }
            step = val;
            checkStepUpdate = true;
        } else if (param == "maxstep") {
            maxStep = val;
            checkStepUpdate = true;
        } else if (param == "minstep") {
            minStep = val;
            checkStepUpdate = true;
        } else {
            SolverInterface::set(param, val);
        }
        if (checkStepUpdate) {
            if (flags[initialized_flag]) {
                ERKStepSetMaxStep(solverMem, maxStep);
                ERKStepSetMinStep(solverMem, minStep);
                ERKStepSetInitStep(solverMem, step);
            }
        }
    }

    double arkodeInterface::get(const std::string& param) const
    {
        long int val = -1;
        if ((param == "resevals") || (param == "iterationcount")) {
            //    CVodeGetNumResEvals(solverMem, &val);
        } else if (param == "iccount") {
            val = icCount;
        } else if (param == "jac calls") {
#ifdef KLU_ENABLE
//    CVodeCVodeSlsGetNumJacEvals(solverMem, &val);
#else
            // ARKDlsGetNumJacEvals (solverMem, &val);
#endif
        } else {
            return sundialsInterface::get(param);
        }

        return static_cast<double>(val);
    }

    // output solver stats
    void arkodeInterface::logSolverStats([[maybe_unused]] solver_print_level logLevel,
                                         bool /*iconly*/) const
    {
        if (!flags[initialized_flag]) {
            return;
        }
        // long int nni = 0;
        long int nst, nre, nfi = 0, netf, /*ncfn,*/ nge;
        realtype tolsfac, hlast, hcur;

        int retval = ERKStepGetNumRhsEvals(solverMem, &nre);
        check_flag(&retval, "ARKodeGetNumResEvals", 1);

        // retval = ERKStepGetNumNonlinSolvIters (solverMem, &nni);
        // check_flag (&retval, "ARKodeGetNumNonlinSolvIters", 1);
        // retval = ERKStepGetNumNonlinSolvConvFails (solverMem, &ncfn);
        // check_flag (&retval, "ARKodeGetNumNonlinSolvConvFails", 1);

        retval = ERKStepGetNumSteps(solverMem, &nst);
        check_flag(&retval, "ARKodeGetNumSteps", 1);
        retval = ERKStepGetNumErrTestFails(solverMem, &netf);
        check_flag(&retval, "ARKodeGetNumErrTestFails", 1);

        retval = ERKStepGetNumGEvals(solverMem, &nge);
        check_flag(&retval, "ARKodeGetNumGEvals", 1);
        ERKStepGetCurrentStep(solverMem, &hcur);

        ERKStepGetLastStep(solverMem, &hlast);
        ERKStepGetTolScaleFactor(solverMem, &tolsfac);

        std::string logstr = "Arkode Run Statistics: \n";

        logstr += "Number of steps                    = " + std::to_string(nst) + '\n';
        logstr +=
            "Number of rhs evaluations     = " + std::to_string(nre) + std::to_string(nfi) + '\n';
        // logstr += "Number of Jacobian evaluations     = " + std::to_string (jacCallCount) + '\n';
        // logstr += "Number of nonlinear iterations     = " + std::to_string (nni) + '\n';
        logstr += "Number of error test failures      = " + std::to_string(netf) + '\n';
        //    logstr += "Number of nonlinear conv. failures = " + std::to_string (ncfn) + '\n';
        logstr += "Number of root fn. evaluations     = " + std::to_string(nge) + '\n';

        logstr += "Current step                       = " + std::to_string(hcur) + '\n';
        logstr += "Last step                          = " + std::to_string(hlast) + '\n';
        logstr += "Tolerance scale factor             = " + std::to_string(tolsfac) + '\n';

        if (sobj != nullptr) {
            // sobj->log (sobj, logLevel, logstr);
        } else {
            printf("\n%s", logstr.c_str());
        }
    }

    void arkodeInterface::logErrorWeights([[maybe_unused]] solver_print_level logLevel) const
    {
        N_Vector eweight = NVECTOR_NEW(use_omp, svsize, ctx);
        N_Vector ele = NVECTOR_NEW(use_omp, svsize, ctx);

        realtype* eldata = NVECTOR_DATA(use_omp, ele);
        realtype* ewdata = NVECTOR_DATA(use_omp, eweight);
        int retval = ERKStepGetErrWeights(solverMem, eweight);
        check_flag(&retval, "ARKodeGetErrWeights", 1);
        retval = ERKStepGetEstLocalErrors(solverMem, ele);
        check_flag(&retval, "ARKodeGetEstLocalErrors ", 1);
        std::string logstr = "Error Weight\tEstimated Local Errors\n";
        for (index_t kk = 0; kk < svsize; ++kk) {
            logstr += std::to_string(kk) + ':' + std::to_string(ewdata[kk]) + '\t' +
                std::to_string(eldata[kk]) + '\n';
        }

        if (sobj != nullptr) {
            //   sobj->log (sobj, logLevel, logstr);
        } else {
            printf("\n%s", logstr.c_str());
        }
        NVECTOR_DESTROY(use_omp, eweight);
        NVECTOR_DESTROY(use_omp, ele);
    }

    static const std::map<int, std::string> arkodeRetCodes{
        {ARK_MEM_NULL, "The solver memory argument was NULL"},
        {ARK_ILL_INPUT, "One of the function inputs is illegal"},
        {ARK_NO_MALLOC, "The solver memory was not allocated by a call to CVodeMalloc"},
        {ARK_TOO_MUCH_WORK, "The solver took the maximum internal steps but could not reach tout"},
        {ARK_TOO_MUCH_ACC,
         "The solver could not satisfy the accuracy demanded by the user for some internal step"},
        {ARK_TOO_CLOSE, "t0 and tout are too close and user didn't specify a step size"},
        {ARK_LINIT_FAIL, "The linear solver's initialization function failed"},
        {ARK_LSETUP_FAIL, "The linear solver's setup function failed in an unrecoverable manner"},
        {ARK_LSOLVE_FAIL, "The linear solver's solve function failed in an unrecoverable manner"},
        {ARK_ERR_FAILURE, "The error test occurred too many times"},
        {ARK_MEM_FAIL, "A memory allocation failed"},
        {ARK_CONV_FAILURE, "convergence test failed too many times"},
        {ARK_BAD_T, "The time t is outside the last step taken"},
        {ARK_FIRST_RHSFUNC_ERR,
         "The user - provided derivative function failed recoverably on the first call"},
        {ARK_REPTD_RHSFUNC_ERR,
         "convergence test failed with repeated recoverable errors in the derivative function"},
        {ARK_RTFUNC_FAIL, "The rootfinding function failed in an unrecoverable manner"},
        {ARK_UNREC_RHSFUNC_ERR,
         "The user-provided right hand side function repeatedly returned a recoverable error "
         "flag, but the solver was unable to recover"},
        {ARK_BAD_K, "Bad K"},
        {ARK_BAD_DKY, "Bad DKY"},

    };

    void arkodeInterface::initialize(double time0)
    {
        if (!flags[allocated_flag]) {
            throw(InvalidSolverOperation());
        }
        // auto jsize = sobj->jacobianSize(mode);

        // dynInitializeB CVode - Sundials
        // guessState an initial condition
        sobj->guessCurrentValue(time0, state_data(), deriv_data(), mode);

        solverMem = ERKStepCreate(arkodeFunc, time0, state, ctx);
        check_flag(solverMem, "ARKStepCreate", 0);

        int retval = ERKStepSetUserData(solverMem, this);
        check_flag(&retval, "ARKodeSetUserData", 1);

        //  retval = ERKStepInit (solverMem, arkodeFunc, arkodeFunc, time0, state);
        //  check_flag (&retval, "ARKodeInit", 1);

        if (rootCount > 0) {
            rootsfound.resize(rootCount);
            retval = ERKStepRootInit(solverMem, rootCount, arkodeRootFunc);
            check_flag(&retval, "ARKodeRootInit", 1);
        }

        N_VConst(tolerance, abstols);

        retval = ERKStepSVtolerances(solverMem, tolerance / 100, abstols);
        check_flag(&retval, "ARKodeSVtolerances", 1);

        retval = ERKStepSetMaxNumSteps(solverMem, 1500);
        check_flag(&retval, "ARKodeSetMaxNumSteps", 1);

#ifdef KLU_ENABLE
        if (flags[dense_flag]) {
            J = SUNDenseMatrix(svsize, svsize);
            check_flag(J, "SUNDenseMatrix", 0);
            /* Create KLU solver object */
            LS = SUNDenseLinearSolver(state, J);
            check_flag(LS, "SUNDenseLinearSolver", 0);
        } else {
            /* Create sparse SUNMatrix */
            J = SUNSparseMatrix(svsize, svsize, jsize, CSR_MAT);
            check_flag(J, "SUNSparseMatrix", 0);

            /* Create KLU solver object */
            LS = SUNKLU(state, J);
            check_flag(LS, "SUNKLU", 0);
        }
#else
        J = SUNDenseMatrix(svsize, svsize, ctx);
        check_flag(J, "SUNSparseMatrix", 0);
        /* Create KLU solver object */
        LS = SUNLinSol_Dense(state, J, ctx);
        check_flag(LS, "SUNDenseLinearSolver", 0);
#endif

        //    retval = ERKStepSetLinearSolver (solverMem, LS, J);

        // check_flag (&retval, "IDADlsSetLinearSolver", 1);

        // retval = ERKStepSetJacFn (solverMem, arkodeJac);
        // check_flag (&retval, "IDADlsSetJacFn", 1);

        // retval = ERKStepSetMaxNonlinIters (solverMem, 20);
        // check_flag (&retval, "ARKodeSetMaxNonlinIters", 1);

        retval = ERKStepSetErrHandlerFn(solverMem, sundialsErrorHandlerFunc, this);
        check_flag(&retval, "ARKodeSetErrHandlerFn", 1);

        if (maxStep > 0.0) {
            retval = ERKStepSetMaxStep(solverMem, maxStep);
            check_flag(&retval, "ARKodeSetMaxStep", 1);
        }
        if (minStep > 0.0) {
            retval = ERKStepSetMinStep(solverMem, minStep);
            check_flag(&retval, "ARKodeSetMinStep", 1);
        }
        if (step > 0.0) {
            retval = ERKStepSetInitStep(solverMem, step);
            check_flag(&retval, "ARKodeSetInitStep", 1);
        }
        setConstraints();
        flags.set(initialized_flag);
    }

    void arkodeInterface::sparseReInit(sparse_reinit_modes sparseReinitMode)
    {
        KLUReInit(sparseReinitMode);
    }

    void arkodeInterface::setRootFinding(count_t numRoots)
    {
        if (numRoots != static_cast<count_t>(rootsfound.size())) {
            rootsfound.resize(numRoots);
        }
        rootCount = numRoots;
        int retval = ERKStepRootInit(solverMem, numRoots, arkodeRootFunc);
        check_flag(&retval, "ARKodeRootInit", 1);
    }

    void arkodeInterface::getCurrentData()
    {
        /*
        int retval = CVodeGetConsistentIC(solverMem, state, deriv);
        if (check_flag(&retval, "CVodeGetConsistentIC", 1))
        {
        return(retval);
        }
        */
    }

    int arkodeInterface::solve(double tStop, double& tReturn, step_mode stepMode)
    {
        // assert (rootCount == sobj->rootSize (mode));
        ++solverCallCount;
        icCount = 0;
        double tret;
        int retval = ERKStepEvolve(solverMem,
                                   tStop,
                                   state,
                                   &tret,
                                   (stepMode == step_mode::normal) ? ARK_NORMAL : ARK_ONE_STEP);
        tReturn = tret;
        check_flag(&retval, "ARKodeSolve", 1, false);

        if (retval == ARK_ROOT_RETURN) {
            retval = SOLVER_ROOT_FOUND;
        }
        return retval;
    }

    void arkodeInterface::getRoots()
    {
        int ret = ERKStepGetRootInfo(solverMem, rootsfound.data());
        check_flag(&ret, "ARKodeGetRootInfo", 1);
    }

    void arkodeInterface::loadMaskElements()
    {
        std::vector<double> mStates(svsize, 0.0);
        //    sobj->getVoltageStates (mStates.data (), mode);
        //   sobj->getAngleStates (mStates.data (), mode);
        maskElements = gmlc::utilities::vecFindgt<double, index_t>(mStates, 0.5);
        tempState.resize(svsize);
        double* lstate = NV_DATA_S(state);
        for (auto& v : maskElements) {
            tempState[v] = lstate[v];
        }
    }

    // CVode C Functions
    int arkodeFunc(realtype time, N_Vector state, N_Vector dstate_dt, void* user_data)
    {
        auto sd = reinterpret_cast<arkodeInterface*>(user_data);
        sd->funcCallCount++;
        if (sd->mode.pairedOffsetIndex != kNullLocation) {
            int ret = sd->sobj->dynAlgebraicSolve(time,
                                                  NVECTOR_DATA(sd->use_omp, state),
                                                  NVECTOR_DATA(sd->use_omp, dstate_dt),
                                                  sd->mode);
            if (ret < FUNCTION_EXECUTION_SUCCESS) {
                return ret;
            }
        }
        int ret = sd->sobj->derivativeFunction(time,
                                               NVECTOR_DATA(sd->use_omp, state),
                                               NVECTOR_DATA(sd->use_omp, dstate_dt),
                                               sd->mode);

        if (sd->flags[fileCapture_flag]) {
            if (!sd->stateFile.empty()) {
                //      writeVector (time, STATE_INFORMATION, sd->funcCallCount,
                //      sd->mode.offsetIndex, sd->svsize,
                //                  NVECTOR_DATA (sd->use_omp, state), sd->stateFile,
                //                  (sd->funcCallCount != 1));
                //      writeVector (time, DERIVATIVE_INFORMATION, sd->funcCallCount,
                //      sd->mode.offsetIndex, sd->svsize,
                //                  NVECTOR_DATA (sd->use_omp, dstate_dt), sd->stateFile);
            }
        }
        return ret;
    }

    int arkodeRootFunc(realtype time, N_Vector state, realtype* gout, void* user_data)
    {
        auto sd = reinterpret_cast<arkodeInterface*>(user_data);
        sd->sobj->rootFindingFunction(
            time, NVECTOR_DATA(sd->use_omp, state), sd->deriv_data(), gout, sd->mode);

        return FUNCTION_EXECUTION_SUCCESS;
    }

    int arkodeJac(realtype time,
                  N_Vector state,
                  N_Vector dstate_dt,
                  SUNMatrix J,
                  void* user_data,
                  N_Vector tmp1,
                  N_Vector tmp2,
                  N_Vector /*tmp3*/)
    {
        return sundialsJac(time, 0.0, state, dstate_dt, J, user_data, tmp1, tmp2);
    }

}  // namespace solvers
}  // namespace griddyn
