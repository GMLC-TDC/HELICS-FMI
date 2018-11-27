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
#include "solver_definitions.hpp"
#include "solverMode.hpp"
#include "utilities/matrixData.hpp"
namespace griddyn
{
class SolvableObject
{
public:
	/** @brief get the number of non-zeros in the most recent Jacobian calculation
	@param[in] sMode the solverMode to get the number of non-zeros for
	@return the number of non-zero elements in the Jacobian
	*/
	virtual solver_index_type jacobianSize(const solverMode &sMode) const = 0;

	/** @brief guess the current value for the states
		@param[in] time  the simulation time of the evaluation
		@param[in] state  the state information to evaluation
		@param[in] dstate_dt  the time derivative of the state
		@param[in] sMode the solverMode to solve for
		*/
	virtual void guessCurrentValue(double time,
		double state[],
		double dstate_dt[],
		const solverMode &sMode) = 0;

		/** @brief compute the network residuals
		computes a set of function for the power system such $r(\hat{x},\hat{x'})=f(x,x)- f(\hat{x},\hat{x}')$
		so that r approaches 0 as the $x$ == $\hat{x}
		@param[in] time  the simulation time of the evaluation
		@param[in] state  the state information to evaluation
		@param[in] dstate_dt  the time derivative of the state
		@param[out] resid the storage location for the residual function
		@param[in] sMode the solverMode to solve for
		@return integer indicating success (0) or failure (non-zero)
		*/
		virtual int residualFunction(double time,
			const double state[],
			const double dstate_dt[],
			double resid[],
			const solverMode &sMode) noexcept = 0;

		/** @brief compute the derivatives for all differential states
		@param[in] time  the simulation time of the evaluation
		@param[in] state  the state information to evaluation
		@param[out] dstate_dt  the time derivative of the state
		@param[in] sMode the solverMode to solve for
		@return integer indicating success (0) or failure (non-zero)
		*/
		virtual int
		derivativeFunction(double time, const double state[], double dstate_dt[], const solverMode &sMode) noexcept = 0;

		/** @brief compute an update to all algebraic states
		compute $x=f(\hat{x})$
		@param[in] time  the simulation time of the evaluation
		@param[in] state  the state information to evaluation
		@param[out] update  the updated state information
		@param[in] sMode the solverMode to solve for
		@param[in] alpha a multiplication factor for updates that are expected to be iterative
		@return integer indicating success (0) or failure (non-zero)
		*/
		virtual int algUpdateFunction(double time,
			const double state[],
			double update[],
			const solverMode &sMode,
			double alpha) noexcept = 0;

		/** @brief compute the Jacobian of the residuals
		computes $\frac{\partial r}{\partial x}$ for all components of the residual
		@param[in] time  the simulation time of the evaluation
		@param[in] state  the state information to evaluation
		@param[in] dstate_dt  the time derivative of the state
		@param[out] md the matrixData object to store the Jacobian information into
		@param[in] cj the constant of integration for use in Jacobian elements using derivatives
		@param[in] sMode the solverMode to solve for
		@return integer indicating success (0) or failure (non-zero)
		*/
		virtual int jacobianFunction(double time,
			const double state[],
			const double dstate_dt[],
			matrixData<double> &md,
			double cj,
			const solverMode &sMode) noexcept = 0;

		/** @brief compute any root values
		computes the roots for any root finding functions used in the system
		@param[in] time  the simulation time of the evaluation
		@param[in] state  the state information to evaluation
		@param[in] dstate_dt  the time derivative of the state
		@param[out] roots the storage location for the roots
		@param[in] sMode the solverMode to solve for
		@return integer indicating success (0) or failure (non-zero)
		*/
		virtual int rootFindingFunction(double time,
			const double state[],
			const double dstate_dt[],
			double roots[],
			const solverMode &sMode) noexcept = 0;


	/** @brief solve for the algebraic components of a system for use with the ode solvers
	@param[in] time  the simulation time of the evaluation
	@param[in] diffState  the current derivative information
	@param[in] deriv the current derivative information
	@param[in] sMode the solverMode to solve related to the differential state information
	@return integer indicating success (0) or failure (non-zero)
	*/
	virtual int dynAlgebraicSolve(double time,
		const double diffState[],
		const double deriv[],
		const solverMode &sMode) noexcept = 0;
};

} //namespace griddyn
