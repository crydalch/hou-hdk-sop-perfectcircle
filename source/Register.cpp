/*
	Creates circles from selected edges.

	IMPORTANT! ------------------------------------------
	-----------------------------------------------------

	Author: 	SWANN
	Email:		sebastianswann@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2017 SWANN
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <UT/UT_DSOVersion.h>
#include <OP/OP_OperatorTable.h>
#include <BM/BM_ResourceManager.h>

// hou-hdk-common
#include <Macros/GroupMenuPRM.h>

// this
#include "SOP_PerfectCircle.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_PerfectCircle
#define SOP_SmallName		"modeling::perfectcircle::1.1"
#define SOP_BigName			"Perfect Circle"
#define SOP_TabMenuPath		"Toolkit/Modeling"

#define MSS_Selector		GET_SOP_Namespace()::MSS_PerfectCircleSelector
#define MSS_SmallName		"modeling::perfectcircleselector::1.0"
#define MSS_BigName			"Perfect Circle (selector)"
#define MSS_Prompt			"Select edges to create circle. Press <enter> to accept."
#define SOP_GroupPRM		CONST_EdgeGroupInput0_Name

/* -----------------------------------------------------------------
OPERATOR                                                           |
----------------------------------------------------------------- */

void 
newSopOperator(OP_OperatorTable* table)
{
	auto sop = new OP_Operator
	(
		SOP_SmallName,
		SOP_BigName,
		SOP_Operator::CreateMe,
		SOP_Operator::parametersList,
		1,								// min inputs 
		1,								// max inputs
		nullptr,
		0,								// type of node OP_FLAG_GENERATOR (BE CAREFUL WITH THIS LITTLE FUCKER)
		nullptr,
		1,								// outputs count
		SOP_TabMenuPath
	);
	
	auto success = table->addOperator(sop);	
	//table->addOpHidden(sop->getName());	
}

/* -----------------------------------------------------------------
SELECTOR                                                           |
----------------------------------------------------------------- */

void
newSelector(BM_ResourceManager* manager)
{
	// find operator
	const auto sopOperator = OP_Network::getOperatorTable(SOP_TABLE_NAME)->getOperator(SOP_SmallName);
	if (!sopOperator)
	{
		UT_ASSERT(!"Could not find required operator!");
		return;
	}

	// create selector
	auto sopSelector = new PI_SelectorTemplate
	(
		MSS_SmallName,
		MSS_BigName,
		SOP_TABLE_NAME
	);
	
	if (sopOperator)
	{
		// setup selector
#if _WIN32		
		sopSelector->constructor(static_cast<void*>(&MSS_Selector::CreateMe));
#else
		sopSelector->constructor((void*)&MSS_Selector::CreateMe);
#endif

		sopSelector->data(OP3DtheEdgeSelTypes);
		
		auto success = manager->registerSelector(sopSelector);
		if (!success) return;

		// bind selector		
		success = manager->bindSelector
		(
			sopOperator,
			MSS_SmallName,
			MSS_BigName,
			MSS_Prompt,
			SOP_GroupPRM,				// Parameter to write group to.
			0,							// Input number to wire up.
			1,							// 1 means this input is required.
			"0x000000ff",				// Prim/point mask selection.
			0,
			nullptr,
			0,
			nullptr,
			false
		);
	}
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_GroupPRM
#undef MSS_Prompt
#undef MSS_BigName
#undef MSS_SmallName
#undef MSS_Selector

#undef SOP_TabMenuPath
#undef SOP_BigName
#undef SOP_SmallName
#undef SOP_Operator