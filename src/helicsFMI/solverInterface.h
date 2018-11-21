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

#include "helics/core/helics-time.hpp"
#include <exception>
#include <memory>
#include <vector>
#include <bitset>

namespace helics_fmi
{
enum class solver_print_level
{
    s_debug_print = 2,
    s_error_log = 1,
    s_error_trap = 0,
};

class FmiModelExchangeFederate;

/** error class for throwing solver exceptions*/
class solverException : public std::exception
{
  protected:
    int errorCode;  //<!* the actual solver Error Code
  public:
    explicit solverException (int ecode = 0) : errorCode (ecode){};
    virtual const char *what () const noexcept override
    {
        return (std::string ("solver Exception:error code=") + std::to_string (errorCode)).c_str ();
    }
    /** return the full name of the object that threw the exception*/
    int code () const noexcept { return errorCode; }
};

/** error class for throwing an invalid solver operation exception from a solver
 */
class InvalidSolverOperation : public solverException
{
  protected:
  public:
    explicit InvalidSolverOperation (int ecode = 0) : solverException (ecode){};
    virtual const char *what () const noexcept override
    {
        return (std::string ("invalid solver operation:error code=") + std::to_string (errorCode)).c_str ();
    }
};

// solver return codes from the solve and initIC functions
#define SOLVER_ROOT_FOUND 2
#define SOLVER_INVALID_STATE_ERROR (-36)
#define SOLVER_INITIAL_SETUP_ERROR (-38)
#define SOLVER_CONVERGENCE_ERROR (-12)

enum solver_flags : int
{
    dense_flag = 0,  //!< if the solver should use a dense or sparse version
    constantJacobian_flag = 1,  //!< if the solver should just keep a constant Jacobian
    useMask_flag = 2,  //!< if the solver should use a mask to filter out specific states
    parallel_flag = 3,  //!< if the solver should use a parallel version
    locked_flag = 4,  //!< if the solverMode is locked from further updates
    use_omp_flag = 5,  //!< flag indicating whether to use omp data constructs
    allocated_flag = 6,  //!< if the solver has been allocated
    initialized_flag = 7,  //!< flag indicating if these vectors have been initialized
    fileCapture_flag = 8,
    directLogging_flag = 9,  //!< flag telling the SolverInterface to capture a log directly from the solver
    use_newton_flag = 11,
    use_bdf_flag = 12,
    block_mode_only = 13,  //!< flag indicating that the solver only supports block mode
    extra_solver_flag1 = 16,
    extra_solver_flag2 = 17,
    extra_solver_flag3 = 18,
    extra_solver_flag4 = 19,
    extra_solver_flag5 = 20,
    extra_solver_flag6 = 21,
    extra_solver_flag7 = 22,
    extra_solver_flag8 = 23,
    extra_solver_flag9 = 24,
    extra_solver_flag10 = 25,
    extra_solver_flag11 = 26,
    extra_solver_flag12 = 27,
    print_residuals = 28,
};
/** @brief class defining the data related to a specific solver
 the SolverInterface class is the base class for solvers for the GridDyn power systems program
a particular SolverInterface class will contain the interface and calls necessary to implement a particular solver
methodology
*/
class OdeSolverInterface 
{
  public:
    /** @brief enumeration of solver call modes*/
    enum class step_mode
    {
        normal,  //!< normal operation
        single_step,  //!< single step operation
        block,  //!< the solver runs in a block mode all at once
    };
    /** @brief enumeration of initiaL condition call modes*/
    enum class ic_modes
    {
        fixed_masked_and_deriv,  //!< fixed_algebraic and differential state derivatives
        fixed_diff,  //!< differential states are fixed
    };

    std::vector<int> rootsfound;  //!< mask vector for which roots were found
  protected:
    std::string lastErrorString;  //!< string containing the last error

    // solver outputs

    std::vector<index_t> maskElements;  //!< vector of constant states in any problem
    std::string solverLogFile;  //!< file name and location of log file reference
    solver_print_level printLevel = solver_print_level::s_error_trap;  //!< print_level for solver
    int solverPrintLevel = 1;  //!< print level for internal solver logging
    count_t rootCount = 0;  //!< the number of root finding functions
    count_t solverCallCount = 0;  //!< the number of times the solver has been called
    count_t funcCallCount = 0;  //!< the number of times the function evaluation has been called
    count_t rootCallCount = 0;
    count_t max_iterations = 10000;  //!< the maximum number of iterations in the solver loop
    double tolerance = 1e-8;  //!< the default solver tolerance
    helics::Time solveTime = helics::Time::minVal();  //!< storage for the time the solver is called
    std::string jacFile;  //!< the file to write the Jacobian to
    std::string stateFile;  //!< the file to write the state and residual to
	FmiModelExchangeFederate *m_gds = nullptr;  //!< pointer the gridDynSimulation object used
    count_t svsize = 0;  //!< the state size
    count_t nnz = 0;  //!< the actual number of non-zeros in a Jacobian
    std::bitset<32> flags;  //!< flags for the solver
    int lastErrorCode = 0;  //!< the last error Code
  public:
    /** @brief default constructor
     * @param[in] objName  the name of the solver
     */
    OdeSolverInterface();

    /** @brief alternate constructor
    @param[in] gds  gridDynSimulation to link with
    @param[in] sMode the solverMode associated with the solver
    */
	explicit OdeSolverInterface(FmiModelExchangeFederate *mex);

    /** @brief make a copy of the solver interface
    @param[in] fullCopy set to true to initialize and copy over all data to the new object
    @return a unique ptr to the clones SolverInterface
    */
    virtual std::unique_ptr<OdeSolverInterface> clone (bool fullCopy = false) const;

    /** @brief make a copy of the solver interface
    @param[in] si a ptr to an existing interface that data should be copied to
    @param[in] fullCopy set to true to initialize and copy over all data to the new object
    */
    virtual void cloneTo (OdeSolverInterface *si, bool fullCopy = false) const;
    /** @brief get a pointer to the state data
    @return a pointer to a double array with the state data
    */
    virtual double *state_data () noexcept;

    /** @brief get a pointer to the state time derivative information
    @return a pointer to a double array with the state time derivative information
    */
    virtual double *deriv_data () noexcept;

    /** @brief get a pointer to the const state data
    @return a pointer to a const double array with the state data
    */
    virtual const double *state_data () const noexcept;

    /** @brief get a pointer to the const state time derivative information
    @return a pointer to a const double array with the state time derivative information
    */
    virtual const double *deriv_data () const noexcept;

    /** @brief allocate the memory for the solver
    @param[in] size  the size of the state vector
    @param[in] numRoots  the number of root functions in the solution
    @return the function status
    */
    virtual void allocate (count_t size, count_t numRoots = 0);

    /** @brief initialize the solver to time t0
    @param[in] t0  the time for the initialization
    @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
    */
    virtual void initialize (helics::Time t0);

    /** @brief load the constraints*/
    virtual void setConstraints ();

    /** @brief get the current solution
     usually called after a call to CalcIC to get the calculated conditions
    @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
    */
    virtual void getCurrentData ();
    /** @brief get the locations of any found roots
    @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
    */
    virtual void getRoots ();
    /** @brief update the number of roots to find
    @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
    */
    virtual void setRootFinding (index_t numRoots);

    /** @brief get a parameter from the solver
  @param[in] param  a string with the desired name of the parameter or result
  @return the value of the requested parameter
  */
    virtual double get (const std::string &param) const;
    /** @brief set a string parameter in the solver
    @param[in] param  a string with the desired name of the parameter
    @param[in] val the value of the property to set
    */
    virtual void set (const std::string &param, const std::string &val);

    /** @brief set a numerical parameter on a solver
  @param[in] param  a string with the desired name of the parameter
  @param[in] val the value of the property to set
  */
    virtual void set (const std::string &param, double val);

    /** @brief set a flag parameter on a solver
    @param[in] flag  a string with the name of the flag to set
    @param[in] val the value of the property to set
    */
    virtual void setFlag (const std::string &flag, bool val = true);
    /** @brief get a flag parameter from a solver
    @param[in] flag  a string with the name of the flag to set
    */
    virtual bool getFlag (const std::string &flag) const;
    /** get the last time the solver was called*/
   helics::Time getSolverTime () const { return solveTime; }
    /** @brief perform the solver calculations
  @param[in] tStop  the requested return time   not that useful for algebraic solvers
  @param[out]  tReturn  the actual return time
  @param[in] stepMode  the step mode
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
    virtual int solve (helics::Time tStop, helics::Time &tReturn, step_mode stepMode = step_mode::normal);
   
    /** @brief check if the SolverInterface has been initialized
    @return true if initialized false if not
    */
    bool isInitialized () const { return flags[initialized_flag]; }

    /** @brief helper function to log specific solver stats
    @param[in] logLevel  the level of logging to display
    */
    virtual void logSolverStats (solver_print_level logLevel) const;
    /** @brief helper function to log error weight information
    @param[in] logLevel  the level of logging to display
    */
    virtual void logErrorWeights (solver_print_level logLevel) const;

    /** @brief get the state size
    @return the state size
    */
    count_t size () const { return svsize; }

    /** @brief print out all the state values
    @param[in] getNames use the actual state names vs some coding
    */
    void printStates (bool getNames = false);
    /** @brief input the simulation data to attach to
    @param[in] gds the gridDynSimulationObject to attach to
    @param[in] sMode the solverMode associated with the solver
    */
    void setSimulationData (FmiModelExchangeFederate *gds);

    void logMessage (int errorCode, const std::string &message);

    int getLastError () const { return lastErrorCode; }
    const std::string &getLastErrorString () const { return lastErrorString; }

  protected:
    /** @brief check the output of actual solver calls for proper results
    @param[in] flagvalue a return code <0 usually indicates an error
    @param[in] funcname  the name of the function that we are checking
    @param[in] opt  0 for allocation 1 for other functions
    @param[in] printError  boolean flag indicating whether to print a message on error or not
    */
    virtual void check_flag (void *flagvalue, const std::string &funcname, int opt, bool printError = true) const;
};

/** @brief make a solver from a string
@param[in] type the type of SolverInterface to create
@return a unique_ptr to a SolverInterface object
*/
std::unique_ptr<OdeSolverInterface > makeSolver (const std::string &type, FmiModelExchangeFederate *me);

}  // namespace helics_fmi
