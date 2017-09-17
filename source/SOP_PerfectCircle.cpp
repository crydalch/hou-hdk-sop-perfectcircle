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
#include <UT/UT_Interrupt.h>
#include <OP/OP_AutoLockInputs.h>
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_Error.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>
#include <GU/GU_EdgeUtils.h>
#include <sys/SYS_Math.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Utility/ParameterAccessing.h>
#include <Utility/EdgeGroupAccessing.h>

// this
#include "Parameters.h"
#include "RadiusModeOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_PerfectCircle
#define SOP_SmallName			"modeling::perfectcircle::1.0"
#define SOP_Input_Name_0		"Geometry"
#define SOP_Icon_Name			"nodeway_short_dark_WB.png"
#define SOP_Base_Operator		SOP_Node
#define MSS_Selector			GET_SOP_Namespace()::MSS_PerfectCircleSelector

// very important
#define SOP_GroupFieldIndex_0	1

#define UI						GET_SOP_Namespace()::UI
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM
#define GRP_ACCESS				GET_Base_Namespace()::Utility::Groups

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,
	UI::input0EdgeGroup_Parameter,
	UI::useUnsharedEdgesToggle_Parameter,
	UI::useUnsharedEdgesSeparator_Parameter,
	UI::edgeIslandErrorModeChoiceMenu_Parameter,

	UI::mainSectionSwitcher_Parameter,
	UI::radiusModeChoiceMenu_Parameter,
	UI::radiusValueFloat_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	UI::setMorphToggle_Parameter,
	UI::setMorphSeparator_Parameter,
	UI::morphValueFloat_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	// is input connected?
	const exint is0Connected = getInput(0) == nullptr ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */

	visibilityState = is0Connected ? 1 : 0; // TODO: do I still need this?

	/* ---------------------------- Set States --------------------------------------- */

	PRM_ACCESS::Get::IntPRM(this, activeState, UI::radiusModeChoiceMenu_Parameter, currentTime);
	changed |= setVisibleState(UI::radiusValueFloat_Parameter.getToken(), activeState == 3);

	PRM_ACCESS::Get::IntPRM(this, activeState, UI::useUnsharedEdgesToggle_Parameter, currentTime);
	changed |= enableParm(UI::input0EdgeGroup_Parameter.getToken(), !activeState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::setMorphToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::morphValueFloat_Parameter.getToken(), visibilityState);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

int
SOP_Operator::CallbackSetRadiusMode(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	// TODO: figure out why restoreFactoryDefaults() doesn't work
	auto defVal = UI::radiusValueFloat_Parameter.getFactoryDefaults()->getFloat();
	PRM_ACCESS::Set::FloatPRM(me, defVal, UI::radiusValueFloat_Parameter, time);

	return 1;
}

int
SOP_Operator::CallbackSetMorph(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	// TODO: figure out why restoreFactoryDefaults() doesn't work
	auto defVal = UI::morphValueFloat_Parameter.getFactoryDefaults()->getFloat();
	PRM_ACCESS::Set::FloatPRM(me, defVal, UI::morphValueFloat_Parameter, time);

	return 1;
}

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_PerfectCircle() { }

SOP_Operator::SOP_PerfectCircle(OP_Network* network, const char* name, OP_Operator* op) 
: SOP_Base_Operator(network, name, op), 
_edgeGroupInput0(nullptr), 
_edgeUnsharedGroup(nullptr)
{ op->setIconName(SOP_Icon_Name); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const 
{ return SOP_Input_Name_0; }

OP_ERROR
SOP_Operator::cookInputGroups(OP_Context& context, int alone)
{
	const auto currentTime = CHgetEvalTime();

	bool useUnsharedEdgesState;
	PRM_ACCESS::Get::IntPRM(this, useUnsharedEdgesState, UI::useUnsharedEdgesToggle_Parameter, currentTime);
	if (useUnsharedEdgesState) return error();

	return cookInputEdgeGroups(context, this->_edgeGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, 0); 
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

OP_ERROR
SOP_Operator::MakePerfectCircleFromEachEdgeIsland(GA_EdgeIslandBundle& edgeislands, UT_AutoInterrupt progress, fpreal time)
{	
	UT_Map<GA_Offset, UT_Vector3>	originalPositions;
	UT_Map<GA_Offset, UT_Vector3>	edits;
	UT_Set<GA_Edge>					edges;	
		
	exint							radiusModeState;	
	bool							setMorphState;
	fpreal							radiusValueState;
	fpreal							morphValueState;
	exint							edgeIslandErrorLevelState;

	PRM_ACCESS::Get::IntPRM(this, radiusModeState, UI::radiusModeChoiceMenu_Parameter, time);
	PRM_ACCESS::Get::FloatPRM(this, radiusValueState, UI::radiusValueFloat_Parameter, time);	

	PRM_ACCESS::Get::IntPRM(this, setMorphState, UI::setMorphToggle_Parameter, time);
	PRM_ACCESS::Get::FloatPRM(this, morphValueState, UI::morphValueFloat_Parameter, time);				
	morphValueState = setMorphState ? 0.01 * morphValueState : 1.0;				// from percentage
	
	PRM_ACCESS::Get::IntPRM(this, edgeIslandErrorLevelState, UI::edgeIslandErrorModeChoiceMenu_Parameter, time);

#define PROGRESS_ESCAPE(node, message, passedprogress) if (passedprogress.wasInterrupted()) { node->addError(SOP_ErrorCodes::SOP_MESSAGE, message); return error(); }
	for (auto island : edgeislands)
	{
		PROGRESS_ESCAPE(this, "Operation interrupted", progress)

#ifdef DEBUG_ISLANDS
		island.Report();
#endif // DEBUG_ISLANDS

		// ignore invalid ones
		auto message = "Edge islands with endpoints detected.";
		if (!island.IsValid())
		{						
			switch (edgeIslandErrorLevelState)
			{
				default: /* do nothing */ continue;
				case static_cast<exint>(HOU_NODE_ERROR_LEVEL::Warning) : { addWarning(SOP_ErrorCodes::SOP_MESSAGE, message); } continue;
				case static_cast<exint>(HOU_NODE_ERROR_LEVEL::Error) : { addError(SOP_ErrorCodes::SOP_MESSAGE, message); } return error();				
			}
		}

		if (island.IsValid() && island.HasMultiOffsets())
		{
			message = "Edge islands with multipoints detected.";
			switch (edgeIslandErrorLevelState)
			{
				default: /* do nothing */ continue;
				case static_cast<exint>(HOU_NODE_ERROR_LEVEL::Warning) : { addWarning(SOP_ErrorCodes::SOP_MESSAGE, message); } continue;
				case static_cast<exint>(HOU_NODE_ERROR_LEVEL::Error) : { addError(SOP_ErrorCodes::SOP_MESSAGE, message); } return error();				
			}
		}

		// clear everything to be sure that nothing is cached
		originalPositions.clear();
		edits.clear();
		edges.clear();
		
		// store original positions		
		auto it = island.Begin();
		for (it; !it.atEnd(); it.advance())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			originalPositions[*it] = this->gdp->getPos3(*it);
		}
		
		// calculate circle
		for (auto edge : island.GetEdges())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			edges.insert(edge);
		}

		GUcircleEdges(edits, *gdp, edges);		
		for (auto edit : edits)
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)			
			this->gdp->setPos3(edit.first, edit.second);
		}		
		
		// modify radius
		if (radiusModeState != static_cast<exint>(RadiusModeOption::DEFAULT))
		{
			// find center
			auto circleCenter = UT_Vector3(0.0, 0.0, 0.0);
			it = island.Begin();
			for (it; !it.atEnd(); it.advance())
			{
				PROGRESS_ESCAPE(this, "Operation interrupted", progress)
				circleCenter += this->gdp->getPos3(*it);
			}
			circleCenter = circleCenter / island.Entries();

			// find closest and farthest position
			const auto posIt = originalPositions.begin();
			auto closestPosition = UT_Vector3(originalPositions[posIt->first]);
			auto farthestPosition = UT_Vector3(originalPositions[posIt->first]);
					
			for (const auto position : originalPositions)
			{
				PROGRESS_ESCAPE(this, "Operation interrupted", progress)

				const auto currentDistance = (circleCenter - position.second).length();
				const auto closestDistance = (circleCenter - closestPosition).length();
				const auto farthestDistance = (circleCenter - farthestPosition).length();

				closestPosition = currentDistance <= closestDistance ? position.second : closestPosition;
				farthestPosition = currentDistance >= farthestDistance ? position.second : farthestPosition;
			}

			// set radius
			it = island.Begin();
			for (it; !it.atEnd(); it.advance())
			{
				PROGRESS_ESCAPE(this, "Operation interrupted", progress)

				// calculate projection
				// TODO: probably one of the UT_Vector3 built in methods can handle this
				auto direction = this->gdp->getPos3(*it) - circleCenter;
				direction.normalize();

				// move points to new position
				this->gdp->setPos3(*it, circleCenter);

				auto newPosition = this->gdp->getPos3(*it);
				switch (radiusModeState)
				{
					case static_cast<exint>(RadiusModeOption::CLOSEST_POINT): { newPosition += (direction * (closestPosition - circleCenter).length()); } break;
					case static_cast<exint>(RadiusModeOption::FARTHEST_POINT): { newPosition += (direction * (farthestPosition - circleCenter).length()); } break;
					case static_cast<exint>(RadiusModeOption::CUSTOM): { newPosition += (direction * radiusValueState); } break;
				}

				this->gdp->setPos3(*it, newPosition);
			}			
		}

		// morph
		if (!setMorphState) continue;

		it = island.Begin();
		for (it; !it.atEnd(); it.advance())
		{
			PROGRESS_ESCAPE(this, "Operation interrupted", progress)
			const auto newPosition = SYSlerp(originalPositions[*it], this->gdp->getPos3(*it), morphValueState);
			this->gdp->setPos3(*it, newPosition);
		}
	}
#undef PROGRESS_ESCAPE
	
	return error();
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{	
	DEFAULTS_CookMySop()

	if (duplicateSource(0, context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{
		// do we use group or process whole geometry?
		bool useUnsharedEdgesState;;
		PRM_ACCESS::Get::IntPRM(this, useUnsharedEdgesState, UI::useUnsharedEdgesToggle_Parameter, currentTime);
		if (useUnsharedEdgesState)
		{
			auto gop = GroupCreator(this->gdp);			
			this->_edgeUnsharedGroup = static_cast<GA_EdgeGroup*>(gop.createGroup(GA_GROUP_EDGE));

			if (this->_edgeUnsharedGroup) GUfindUnsharedEdges(*this->gdp, *this->_edgeUnsharedGroup);
			if (!this->_edgeUnsharedGroup || this->_edgeUnsharedGroup->isEmpty())
			{
				clearSelection();
				addWarning(SOP_ErrorCodes::SOP_ERR_BADGROUP);
				return error();
			}
			
			select(*this->_edgeUnsharedGroup);
		}
		else
		{	
			if (!this->_edgeGroupInput0 || this->_edgeGroupInput0->isEmpty())
			{
				clearSelection();
				addWarning(SOP_ErrorCodes::SOP_ERR_BADGROUP);
				return error();
			}
		}

		// edge selection can contain multiple separate edge islands
		auto edgeData = GA_EdgesData();
		edgeData.Clear();

		auto success = GRP_ACCESS::Edge::Break::PerPoint(this, (useUnsharedEdgesState? this->_edgeUnsharedGroup : this->_edgeGroupInput0), edgeData, progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// once we have endpoints, we can go thru our edge group and split it on edge islands	
		auto edgeIslands = GA_EdgeIslandBundle();
		edgeIslands.clear();

		success = GRP_ACCESS::Edge::Break::PerIsland(this, edgeData, edgeIslands, EdgeIslandType::CLOSED, progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// finally, we can go thru each edge island and calculate and apply circles
		return MakePerfectCircleFromEachEdgeIsland(edgeIslands, progress, currentTime);
	}		

	return error();
}

/* -----------------------------------------------------------------
SELECTOR IMPLEMENTATION                                            |
----------------------------------------------------------------- */

MSS_Selector::~MSS_PerfectCircleSelector() { }

MSS_Selector::MSS_PerfectCircleSelector(OP3D_View& viewer, PI_SelectorTemplate& templ) 
: MSS_ReusableSelector(viewer, templ, SOP_SmallName, CONST_EdgeGroupInput0_Name, nullptr, true)
{ this->setAllowUseExistingSelection(false); }

BM_InputSelector*
MSS_Selector::CreateMe(BM_View& viewer, PI_SelectorTemplate& templ)
{ return new MSS_Selector(reinterpret_cast<OP3D_View&>(viewer), templ); }

const char*
MSS_Selector::className() const
{ return "MSS_PerfectCircleSelector"; }

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef PRM_ACCESS
#undef UI

#undef SOP_GroupFieldIndex_0

#undef MSS_Selector
#undef SOP_Base_Operator
#undef SOP_Icon_Name
#undef SOP_Input_Name_0
#undef SOP_SmallName
#undef SOP_Operator