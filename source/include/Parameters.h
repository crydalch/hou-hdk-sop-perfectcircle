/*
	Creates circles from selected edges.

	IMPORTANT! ------------------------------------------
	-----------------------------------------------------

	Author: 	SNOWFLAKE
	Email:		snowflake@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2017 SNOWFLAKE
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#ifndef ____parameters_h____
#define ____parameters_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/SwitcherPRM.h>
#include <Macros/GroupMenuPRM.h>
#include <Macros/FloatPRM.h>
#include <Macros/TogglePRM.h>

// this
#include "SOP_PerfectCircle.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator GET_SOP_Namespace()::SOP_PerfectCircle

/* -----------------------------------------------------------------
USING                                                              |
----------------------------------------------------------------- */

// YOUR CODE GOES HERE...

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	namespace UI
	{
		__DECLARE__Filter_Section_PRM(3)
		DECLARE_Default_EdgeGroup_Input_0_PRM(input0)
		DECLARE_Toggle_with_Separator_OFF_PRM("useunsharededges", "Use Unshared Edges", "useunsharededgesseparator", 0, "Use unshared edges instead of edge group.", useUnsharedEdges)

		__DECLARE_Main_Section_PRM(2)
		static auto		radiusModeChoiceMenuParm_Name = PRM_Name("radiusmode", "Radius Mode");
		static auto		radiusModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 3);
		static PRM_Name radiusModeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Default"),
			PRM_Name("1", "Closest Point"),
			PRM_Name("2", "Farthest Point"),
			PRM_Name("3", "Custom"),
			PRM_Name(0)
		};
		static auto		radiusModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, radiusModeChoiceMenuParm_Choices);
		auto			radiusModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &radiusModeChoiceMenuParm_Name, 0, &radiusModeChoiceMenuParm_ChoiceList, &radiusModeChoiceMenuParm_Range, &SOP_Operator::CallbackSetRadiusMode, 0, 1, "Specify radius mode.");
		
		DECLARE_Custom_Float_0R_to_MaxU_PRM("radiusvalue", "Value", 1, 0.5, 0, "2 * r = 1 unit", radiusValue)

		__DECLARE_Additional_Section_PRM(7)
		DECLARE_Toggle_with_Separator_OFF_PRM("setmorph", "Morph", "setmorphseparator", &SOP_Operator::CallbackSetMorph, "Blend between original and modified position.", setMorph)
		DECLARE_Custom_Float_MinR_to_MaxU_PRM("morphvalue", "Value", 0, 100, 100, 0, "Percents", morphValue)
		DECLARE_DescriptionPRM(SOP_Operator)
	}
		
DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator

#endif // !____parameters_h____