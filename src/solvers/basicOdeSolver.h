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

#include <vector>
#include <memory>
#include <string>

namespace griddyn {
namespace solvers {
    /** @brief class implementing a Gauss Seidel solver for algebraic variables in a power system
     */
    class basicOdeSolver: public SolverInterface {
      private:
        std::vector<double> state;  //!< state data/
        std::vector<double> deriv;  //!< temp state data location 1
        std::vector<double> state2;  //!< temp state data location 2
        std::vector<double> type;  //!< type data
        double deltaT = 0.005;  //!< the default time step
      public:
        /** @brief default constructor*/
        explicit basicOdeSolver(const std::string& objName = "basicOde");
        /** alternate constructor to feed to SolverInterface
        @param[in] sobj  the solvableObject to link to
        @param[in] sMode the solverMode to solve with
        */
        basicOdeSolver(SolvableObject* sobj, const solverMode& sMode);

        virtual std::unique_ptr<SolverInterface> clone(bool fullCopy = false) const override;

        virtual void cloneTo(SolverInterface* si, bool fullCopy = false) const override;
        double* state_data() noexcept override;
        double* deriv_data() noexcept override;
        double* type_data() noexcept override;

        const double* state_data() const noexcept override;
        const double* deriv_data() const noexcept override;
        const double* type_data() const noexcept override;
        virtual void allocate(solver_index_type stateCount,
                              solver_index_type numRoots = 0) override;
        virtual void initialize(double t0) override;

        virtual double get(const std::string& param) const override;
        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param, double val) override;

        virtual int
            solve(double tStop, double& tReturn, step_mode stepMode = step_mode::normal) override;
    };

}  // namespace solvers
}  // namespace griddyn
