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

#include "solverMode.hpp"

namespace griddyn
{

	solverMode::solverMode(solver_index_type index) :offsetIndex(index)
	{
		if (index == local_mode)  // predefined local
		{
			local = true;
			dynamic = true;
			differential = true;
			algebraic = true;
		}
		else if (index == power_flow)  // predefined pflow
		{
			algebraic = true;
			differential = false;
			dynamic = false;
		}
		else if (index == dae)  // predefined dae
		{
			dynamic = true;
			differential = true;
			algebraic = true;
		}
		else if (index == dynamic_algebraic)  // predefined dynAlg
		{
			algebraic = true;
			differential = false;
			dynamic = true;
		}
		else if (index == dynamic_differential)  // predefined dynDiff
		{
			algebraic = false;
			differential = true;
			dynamic = true;
		}
	}

} // namespace griddyn