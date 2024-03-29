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
#include "sundialsInterface.h"

#include <memory>
#include <string>
#include <vector>

namespace griddyn {
namespace solvers {
    /** @brief SolverInterface interfacing to the SUNDIALS cvode solver
     */
    class cvodeInterface: public sundialsInterface {
      public:
        solver_index_type icCount = 0;  //!< total number of initial condition calls

      private:
        matrixDataSparse<double> a1;  //!< array structure for holding the Jacobian information
        std::vector<double> tempState;  //!< temporary holding location for a state vector
        double maxStep = -1.0;  //!< the maximum step size permitted
        double minStep = -1.0;  //!< the minimum step size permitted
        double step = 0.0;  //!< the requested step size

      public:
        /** @brief constructor*/
        explicit cvodeInterface(const std::string& objName = "cvode");
        /** @brief alternate constructor
        @param[in] sobj  the solvable object to connect to
        @param[in] sMode the solverMode to solve For
        */
        cvodeInterface(SolvableObject* sobj, const solverMode& sMode);
        /** @brief destructor*/
        virtual ~cvodeInterface();

        virtual std::unique_ptr<SolverInterface> clone(bool fullCopy = false) const override;

        virtual void cloneTo(SolverInterface* si, bool fullCopy = false) const override;
        virtual void allocate(solver_index_type stateCount,
                              solver_index_type numRoots = 0) override;
        virtual void initialize(double time0) override;
        virtual void setMaxNonZeros(solver_index_type nonZeroCount) override;
        virtual void sparseReInit(sparse_reinit_modes reInitMode) override;
        virtual void getCurrentData() override;
        virtual int
            solve(double tStop, double& tReturn, step_mode stepMode = step_mode::normal) override;
        virtual void getRoots() override;
        virtual void setRootFinding(solver_index_type numRoots) override;

        virtual void logSolverStats(solver_print_level logLevel,
                                    bool iconly = false) const override;
        virtual void logErrorWeights(solver_print_level logLevel) const override;
        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param, double val) override;
        virtual double get(const std::string& param) const override;
        // declare friend some helper functions
        friend int cvodeFunc(realtype time, N_Vector state, N_Vector dstate_dt, void* user_data);

        friend int cvodeJac(realtype time,
                            N_Vector state,
                            N_Vector dstate_dt,
                            SUNMatrix J,
                            void* user_data,
                            N_Vector tmp1,
                            N_Vector tmp2,
                            N_Vector tmp3);

        friend int cvodeRootFunc(realtype time, N_Vector state, realtype* gout, void* user_data);

      protected:
        void loadMaskElements();
    };

}  // namespace solvers
}  // namespace griddyn
