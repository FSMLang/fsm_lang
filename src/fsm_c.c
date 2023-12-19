/**
  fsm_c.c

	Creates C code to implement FSM


	FSMLang (fsm) - A Finite State Machine description language.
	Copyright (C) 2002  Steven Stanton

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Steven Stanton
	ringwinner@users.sourceforge.net

	For the latest on FSMLang: http://fsmlang.sourceforge.net

	And, finally, your possession of this source code implies nothing.

  Long Description:

  Creation: 	sstanton		Jan-21-2002
*/

#include "fsm_c.h"
#include "fsm_c_common.h"
#include "fsm_unused.h"
#include "ancestry.h"

#if defined (CYGWIN) || defined (LINUX)
	#include <stdio.h>
	#include <ctype.h>
	#include <unistd.h>
#endif
#if defined (LINUX) || defined (VS) || defined (CYGWIN)
	#include <time.h>
#endif
#include <string.h>
#include <stdlib.h>

#if defined (CYGWIN) || defined (LINUX)
	#include "y.tab.h"
#elif defined (VS)
	#include "parser.h"
#endif


/*
  Our interface to the outside world
*/
static void writeCMachine(pFSMOutputGenerator, pMACHINE_INFO);
static void writeCSubMachine(pFSMOutputGenerator, pMACHINE_INFO);
static void writeCMachineFN(pFSMOutputGenerator, pMACHINE_INFO);

FSMCOutputGenerator CMachineWriter = {
	{
		initCMachine,
		writeCMachine,
		closeCMachine
	},
	NULL
};

/* list iteration callbacks */

static bool declare_action_enum_member(pLIST_ELEMENT pelem, void *data)
{
	pITERATOR_CALLBACK_HELPER pich = ((pITERATOR_CALLBACK_HELPER)data);
	pID_INFO pid_info              = ((pID_INFO)pelem->mbr);

	if (pid_info->name && strlen(pid_info->name))
	{

		fprintf(pich->pcmd->hFile
				, "%s"
				, pich->ih.first ? (pich->ih.first = false, "  ") : ", "
			   );
		printAncestry(pich->ih.pmi, pich->pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pich->pcmd->hFile
				, "_%s_e\n"
				, pid_info->name
			   );

	}

	return false;
}

static bool declare_action_array_member(pLIST_ELEMENT pelem, void *data)
{
	pITERATOR_CALLBACK_HELPER pich = ((pITERATOR_CALLBACK_HELPER)data);
	pID_INFO pid_info              = ((pID_INFO)pelem->mbr);

	if (pid_info->name && strlen(pid_info->name))
	{

		fprintf(pich->pcmd->cFile
				, "\t%s%s_%s\n"
				, pich->ih.first ? (pich->ih.first = false, "  ") : ", "
				, machineName(pich->pcmd)
				, pid_info->name
			   );

	}

	return false;
}

static bool declare_transition_fn_enum_member(pLIST_ELEMENT pelem, void *data)
{
	pITERATOR_CALLBACK_HELPER pich = ((pITERATOR_CALLBACK_HELPER)data);
	pID_INFO pid_info              = ((pID_INFO)pelem->mbr);

	fprintf(pich->pcmd->hFile
			, "%s"
			, pich->ih.first ? (pich->ih.first = false, "  ") : ", "
		   );
	printAncestry(pich->ih.pmi, pich->pcmd->hFile, "_", alc_lower, ai_include_self);
	fprintf(pich->pcmd->hFile
			, "_%s_e\n"
			, pid_info->name
		   );

	return false;
}

static bool declare_transition_enum_member(pLIST_ELEMENT pelem, void *data)
{
	pITERATOR_CALLBACK_HELPER pich = ((pITERATOR_CALLBACK_HELPER)data);
	pID_INFO pid_info              = ((pID_INFO)pelem->mbr);

	fprintf(pich->pcmd->hFile
			, "%s"
			, pich->ih.first ? (pich->ih.first = false, "  ") : ", "
		   );
	printAncestry(pich->ih.pmi, pich->pcmd->hFile, "_", alc_lower, ai_include_self);
	fprintf(pich->pcmd->hFile
			, "_transitionTo%s_e\n"
			, pid_info->name
		   );

	return false;
}

static bool define_transition_fn_array_member(pLIST_ELEMENT pelem, void *data)
{
	pITERATOR_CALLBACK_HELPER pich = ((pITERATOR_CALLBACK_HELPER)data);
	pID_INFO pid_info              = ((pID_INFO)pelem->mbr);

	fprintf(pich->pcmd->cFile
			, "\t%s%s_%s\n"
			, pich->ih.first ? (pich->ih.first = false, "  ") : ", "
			, fqMachineName(pich->pcmd)
			, pid_info->name
		   );

	return false;
}

static bool define_transition_array_member(pLIST_ELEMENT pelem, void *data)
{
	pITERATOR_CALLBACK_HELPER pich = ((pITERATOR_CALLBACK_HELPER)data);
	pID_INFO pid_info              = ((pID_INFO)pelem->mbr);

	fprintf(pich->pcmd->cFile
			, "\t%s%s_transitionTo%s\n"
			, pich->ih.first ? (pich->ih.first = false, "  ") : ", "
			, fqMachineName(pich->pcmd)
			, pid_info->name
		   );

	return false;
}

/*
  Our "real" internals.
*/
static int  writeCMachineInternal(pCMachineData, pMACHINE_INFO);
static int  writeCSubMachineInternal(pCMachineData, pMACHINE_INFO);
static void writeOriginalFSM(pCMachineData, pMACHINE_INFO);
static void writeOriginalSubFSM(pCMachineData, pMACHINE_INFO);
static void writeOriginalFSMLoop(pCMachineData, pMACHINE_INFO);
static void writeOriginalSubFSMLoop(pCMachineData, pMACHINE_INFO);
static void writeOriginalFSMLoopInnards(pCMachineData, pMACHINE_INFO, char *);
static void writeOriginalSubFSMLoopInnards(pCMachineData, pMACHINE_INFO, char *);
static void writeNoTransition(pCMachineData, pMACHINE_INFO);
static void writeReentrantFSM(pCMachineData, pMACHINE_INFO);
static void writeActionsReturnStateFSM(pCMachineData, pMACHINE_INFO);
static void declareCMachineActionArray(pCMachineData, pMACHINE_INFO);
static void defineCMachineActionFnArray(pCMachineData, pMACHINE_INFO);
static void defineCMachineTransitionFnArray(pCMachineData, pMACHINE_INFO);
static void declareCMachineActionFnEnum(pCMachineData, pMACHINE_INFO);
static void declareCMachineTransitionFnEnum(pCMachineData, pMACHINE_INFO);
static void declareCMachineStruct(pCMachineData, pMACHINE_INFO);
static void defineActionArray(pCMachineData, pMACHINE_INFO);
static void defineCMachineFSM(pCMachineData, pMACHINE_INFO);
static void defineCSubMachineFSM(pCMachineData, pMACHINE_INFO);


static int writeCSubMachineInternal(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	if (!pmi || !pcmd) return 1;

	/* do this now, since some header stuff puts content into the source file.*/
	addNativeImplementationIfThereIsAny(pmi, pcmd->cFile);

	subMachineHeaderStart(pcmd, pmi, "action");

	/* we need our count of events */
	fprintf(pcmd->hFile, "typedef enum { ");
	printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->hFile
			, "_numEvents = %u} "
			, pmi->event_list->count
		   );
	printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->hFile, "_EVENTS;\n");

	declareCMachineActionArray(pcmd, pmi);

	declareCMachineStruct(pcmd, pmi);

	commonHeaderEnd(pcmd, pmi, true);

	/*
	  Source File
	*/

	defineActionArray(pcmd, pmi);

	defineSubMachineIF(pcmd, pmi);

	possiblyDefineSubMachineSharedEventStructures(pcmd, pmi);

	defineSubMachineArray(pcmd, pmi);

	if (generate_instance)
	{
		generateInstance(pcmd, pmi, "action");
	}

	defineCSubMachineFSM(pcmd, pmi);

	defineStateEntryAndExitManagers(pcmd, pmi);

	if (generate_weak_fns)
	{
		/* write weak stubs for our action functions */
		defineWeakActionFunctionStubs(pcmd, pmi);

		/* ... and for the noAction case */
		defineWeakNoActionFunctionStubs(pcmd, pmi);

		/* write weak stubs for any data translators 
		TODO: sub machine data translators are different
		defineWeakDataTranslatorStubs(pcmd, pmi);
		*/

		/* write weak state entry and exit functions */
		defineWeakStateEntryAndExitFunctionStubs(pcmd, pmi);
	}
	else if (force_generation_of_event_passing_actions)
	{
		defineEventPassingActions(pcmd, pmi);
	}

	/* write our transition functions, if needed */
	if (pmi->transition_fn_list->count)
	{
		writeStateTransitions(pcmd, pmi);
		writeNoTransition(pcmd, pmi);
	}

	writeDebugInfo(pcmd, pmi);

	return 0;

}

static int writeCMachineInternal(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	if (!pmi || !pcmd) return 1;

	/* do this now, since some header stuff puts content into the source file.*/
	addNativeImplementationIfThereIsAny(pmi, pcmd->cFile);

	commonHeaderStart(pcmd, pmi, "action");

	declareCMachineActionArray(pcmd, pmi);

	declareCMachineStruct(pcmd, pmi);

	commonHeaderEnd(pcmd, pmi, true);

	/*
	  Source File
	*/

	defineActionArray(pcmd, pmi);

	defineSubMachineArray(pcmd, pmi);

	if (generate_instance)
	{
		generateInstance(pcmd, pmi, "action");
	}

	if (generate_run_function)
	{
		generateRunFunction(pcmd, pmi);
	}

	defineCMachineFSM(pcmd, pmi);

	/* write our sub-machine lookup, if needed */
	if (pmi->machine_list)
	{
		defineSubMachineFinder(pcmd, pmi);
	}

	defineStateEntryAndExitManagers(pcmd, pmi);

	if (pmi->data_block_count)
	{
		defineEventDataManager(pcmd, pmi);
	}

	if (generate_weak_fns)
	{
		/* write weak stubs for our action functions */
		defineWeakActionFunctionStubs(pcmd, pmi);

		/* ... and for the noAction case */
		defineWeakNoActionFunctionStubs(pcmd, pmi);

		/* ... don't forget any entry and exit functions */
		defineWeakStateEntryAndExitFunctionStubs(pcmd, pmi);

		/* write weak stubs for needed data translators */
		if (pmi->data_block_count)
		{
			defineWeakDataTranslatorStubs(pcmd, pmi);
		}

	}
	else if (force_generation_of_event_passing_actions)
	{
		defineEventPassingActions(pcmd, pmi);
	}

	/* write our transition functions, if needed */
	if (pmi->transition_fn_list->count)
	{
		writeStateTransitions(pcmd, pmi);
		writeNoTransition(pcmd, pmi);
	}

	writeDebugInfo(pcmd, pmi);

	return 0;

}

static void writeCMachineFN(pFSMOutputGenerator pfsmog, pMACHINE_INFO pmi)
{

	pFSMCOutputGenerator pfsmcog = (pFSMCOutputGenerator)pfsmog;

	printf("%s ", pfsmcog->pcmd->cName);

	if (pmi->machine_list)
	{
		write_machines(pmi->machine_list, generateCMachineWriter, pfsmog);
	}

}
static void writeCSubMachine(pFSMOutputGenerator pfsmog, pMACHINE_INFO pmi)
{

	pFSMCOutputGenerator pfsmcog = (pFSMCOutputGenerator)pfsmog;

	pfsmcog->pcmd->pmi = pmi;

	writeCSubMachineInternal(pfsmcog->pcmd, pmi);

	if (pmi->machine_list)
	{
		write_machines(pmi->machine_list, generateCMachineWriter, pfsmog);
	}

}

static void writeCMachine(pFSMOutputGenerator pfsmog, pMACHINE_INFO pmi)
{

	pFSMCOutputGenerator pfsmcog = (pFSMCOutputGenerator)pfsmog;

	pfsmcog->pcmd->pmi = pmi;

	writeCMachineInternal(pfsmcog->pcmd, pmi);

	if (pmi->machine_list)
	{
		write_machines(pmi->machine_list, generateCMachineWriter, pfsmog);
	}

}


/**
  This function writes the original FSM
*/
static void writeOriginalFSM(pCMachineData pcmd, pMACHINE_INFO pmi)
{
#ifdef FSMLANG_DEVELOP
	fprintf(pcmd->cFile, "/* %s */\n", __func__);
#endif

	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		fprintf(pcmd->cFile
				, "\t%s new_e;\n"
				, eventType(pcmd)
				);

		fprintf(pcmd->cFile
				, "\t%s e = event%s;\n"
				, eventType(pcmd)
				, ultimateAncestor(pmi)->data_block_count ? "->event" : ""
				);
	}

	if (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		fprintf(pcmd->cFile
				, "\t%s new_s;\n\n"
				, stateType(pcmd)
				);
	}

	writeOriginalFSMLoop(pcmd, pmi);

	fprintf(pcmd->cFile, "\n\n}\n\n");
}

/**
  This function writes the original FSM
*/
static void writeOriginalSubFSM(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	char *parent_cp = hungarianToUnderbarCaps(pmi->parent->name->name);

#ifdef FSMLANG_DEVELOP
	fprintf(pcmd->cFile
			, "/* writeOriginalSubFSM */\n"
		   );
#endif

	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		fprintf(pcmd->cFile, "\t");
		streamHungarianToUnderbarCaps(pcmd->cFile, ultimateAncestor(pmi)->name->name);
		fprintf(pcmd->cFile
				, "_EVENT%s new_e;\n\n"
				, ultimateAncestor(pmi)->data_block_count ? "_ENUM" : ""
			   );

		fprintf(pcmd->cFile, "\t");
		streamHungarianToUnderbarCaps(pcmd->cFile, ultimateAncestor(pmi)->name->name);
		fprintf(pcmd->cFile
				, "_EVENT%s e = event;\n\n"
				, ultimateAncestor(pmi)->data_block_count ? "_ENUM" : ""
			   );

	}

	if (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		fprintf(pcmd->cFile, "\t");
		printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->cFile, "_STATE new_s;\n\n");
	}

	writeOriginalSubFSMLoop(pcmd, pmi);

	fprintf(pcmd->cFile
			, "\n\treturn e == THIS(noEvent) ? PARENT(noEvent) : e;"
		   );

	fprintf(pcmd->cFile, "\n\n}\n\n");
}

/**
 *This function writes an FSM suitable for shared use in ISRs  
 *and non-ISR settings.  
*/
static void writeReentrantFSM(pCMachineData pcmd, pMACHINE_INFO pmi)
{
#ifdef FSMLANG_DEVELOP
	fprintf(pcmd->cFile
			, "/* writeReentrantFSM */\n"
		   );
#endif
	fprintf(pcmd->cFile, "\t");
	streamHungarianToUnderbarCaps(pcmd->cFile, ultimateAncestor(pmi)->name->name);
	fprintf(pcmd->cFile
			, "_EVENT%s new_e;\n\n"
			, pmi->data_block_count ? "_ENUM"  : ""
		   );

	fprintf(pcmd->cFile, "\t");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile
			, "_EVENT%s e = event;\n"
			, pmi->data_block_count ? "_ENUM"  : ""
		   );

	fprintf(pcmd->cFile, "#ifdef FSM_START_CRITICAL\n");
	fprintf(pcmd->cFile, "\tFSM_START_CRITICAL;\n");
	fprintf(pcmd->cFile, "#endif\n\n");

	writeOriginalFSMLoop(pcmd, pmi);

	fprintf(pcmd->cFile, "\n\n#ifdef FSM_END_CRITICAL\n");
	fprintf(pcmd->cFile, "\tFSM_END_CRITICAL;\n");
	fprintf(pcmd->cFile, "#endif\n\n");

	fprintf(pcmd->cFile, "}\n\n");

}

/**
  This function writes the ActionsReturnState FSM
*/
static void writeActionsReturnStateFSM(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	fprintf(pcmd->cFile, "\t");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, "_STATE s;\n");

	fprintf(pcmd->cFile, "\n\tDBG_PRINTF(\"event: %%s; start state: %%s\"\n\t\t,");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, "_EVENT_NAMES[event]\n\t\t,");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, "_STATE_NAMES[pfsm->state]\n\t\t);\n");

	if (pmi->data_block_count)
	{
		fprintf(pcmd->cFile
				, "\ttranslateEventData(&pfsm->data, event);\n\n"
			   );
	}

	fprintf(pcmd->cFile
			, "\n\ts = (*(*pfsm->actionArray)[event][pfsm->state])(pfsm);\n\n"
		   );

	fprintf(pcmd->cFile, "\tif (s != ");
	printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->cFile, "_noTransition)\n\t{\n");

	if (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		fprintf(pcmd->cFile
				, "\t\tif (s != pfsm->state)\n\t\t{\n"
			   );

		if (pmi->machineTransition)
		{
			fprintf(pcmd->cFile, "\t\t\t");
			printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
			fprintf(pcmd->cFile
					, "_%s(pfsm,s);\n"
					, pmi->machineTransition->name
				   );
		}

		if (pmi->states_with_exit_fns_count)
		{
			fprintf(pcmd->cFile
					, "\t\trunAppropriateExitFunction(%spfsm->state);\n"
					, pmi->data ? "&pfsm->data, " : ""
				   );
		}

		if (pmi->states_with_entry_fns_count)
		{
			fprintf(pcmd->cFile
					, "\t\trunAppropriateEntryFunction(%ss);\n"
					, pmi->data ? "&pfsm->data, " : ""
				   );
		}

		fprintf(pcmd->cFile
				, "\t\t}\n"
			   );

	}

	fprintf(pcmd->cFile
			, "\t\tpfsm->state = s;\n\t}\n\n"
		   );

	fprintf(pcmd->cFile, "\n\tDBG_PRINTF(\"end state: %%s\"\n\t\t,");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, "_STATE_NAMES[pfsm->state]\n\t\t);\n");

	fprintf(pcmd->cFile, "}\n");

}

static void writeNoTransition(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	fprintf(pcmd->cFile, "\n");
	printNameWithAncestry("STATE ", pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	printNameWithAncestry("noTransitionFn(p", pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, " pfsm");
	if (pmi->modFlags & mfActionsReturnStates)
	{
		fprintf(pcmd->cFile, ")\n{\n");

		fprintf(pcmd->cFile
				, "\t%s(\""
				, core_logging_only ? "NON_CORE_DEBUG_PRINTF" : "DBG_PRINTF"
				);
		printNameWithAncestry("noTransitionFn", pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->cFile, "\");\n\t(void) pfsm;\n\treturn ");
		printNameWithAncestry("noTransition", pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	}
	else
	{
		fprintf(pcmd->cFile, ", ");
		streamHungarianToUnderbarCaps(pcmd->cFile, ultimateAncestor(pmi)->name->name);
		fprintf(pcmd->cFile
				, "_EVENT%s e)\n{\n"
				, ultimateAncestor(pmi)->data_block_count ? "_ENUM"  : ""
				);
		fprintf(pcmd->cFile, "\t(void) e;\n");
		fprintf(pcmd->cFile
				, "\t%s(\""
				, core_logging_only ? "NON_CORE_DEBUG_PRINTF" : "DBG_PRINTF"
				);
		printNameWithAncestry("noTransitionFn", pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->cFile, "\");\n\t(void) pfsm;\n\treturn pfsm->state");
	}
	fprintf(pcmd->cFile, ";\n}\n\n");
}

static void writeOriginalFSMLoopInnards(pCMachineData pcmd, pMACHINE_INFO pmi, char *tabstr)
{
	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		if (compact_action_array)
		{
			fprintf(pcmd->cFile
					, "%s\tnew_e = (*"
					, tabstr
				   );
			printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
			fprintf(pcmd->cFile, "_action_fns[(*pfsm->actionArray)[e][pfsm->state].action])(pfsm);\n\n");
		}
		else
		{
			fprintf(pcmd->cFile
					, "%s\tnew_e = ((* (*pfsm->actionArray)[e][pfsm->state].action)(pfsm));\n\n"
					, tabstr
				   );
		}
	}
	else
	{
		if (compact_action_array)
		{
			fprintf(pcmd->cFile
					, "%s(*"
					, tabstr
				   );
			printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
			fprintf(pcmd->cFile, "_action_fns[(*pfsm->actionArray)[event][pfsm->state].action])(pfsm);\n\n");
		}
		else
		{
			fprintf(pcmd->cFile
					, "%s((* (*pfsm->actionArray)[event][pfsm->state].action)(pfsm));\n\n"
					, tabstr
				   );
		}
	}

	if (!pmi->transition_fn_list->count)
	{
		fprintf(pcmd->cFile
				, "%s\t%s = (*pfsm->actionArray)[%s][pfsm->state].transition;\n\n"
				, tabstr
				, (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
				? "new_s" : "pfsm->state"
				, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
			   );
	}
	else
	{
		fprintf(pcmd->cFile
				, "%s\t%s = ("
				, tabstr
				, (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
				? "new_s" : "pfsm->state"
			   );
		if (compact_action_array)
		{
			fprintf(pcmd->cFile, "*");
			printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
			fprintf(pcmd->cFile
					, "_transition_fns[(*pfsm->actionArray)[%s][pfsm->state].transition])(pfsm,%s);\n\n"
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
				   );
		}
		else
		{
			fprintf(pcmd->cFile
					, "(* (*pfsm->actionArray)[%s][pfsm->state].transition)(pfsm,%s));\n\n"
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
				   );
		}
	}

	if (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		fprintf(pcmd->cFile
				, "\n%s\tif (pfsm->state != new_s)\n\t\t{\n"
				, tabstr
			   );

		if (pmi->machineTransition)
		{
			fprintf(pcmd->cFile
					, "%s\t\t"
					, tabstr
				   );
			printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
			fprintf(pcmd->cFile
					, "_%s(pfsm,new_s);\n"
					, pmi->machineTransition->name
				   );
		}

		if (pmi->states_with_exit_fns_count)
		{
			fprintf(pcmd->cFile
					, "\t\trunAppropriateExitFunction(%spfsm->state);\n"
					, pmi->data ? "&pfsm->data, " : ""
				   );
		}

		if (pmi->states_with_entry_fns_count)
		{
			fprintf(pcmd->cFile
					, "\t\trunAppropriateEntryFunction(%snew_s);\n"
					, pmi->data ? "&pfsm->data, " : ""
				   );
		}

		fprintf(pcmd->cFile
				, "%s\t\tpfsm->state = new_s;\n\n"
				, tabstr
			   );

		fprintf(pcmd->cFile
				, "%s\t}\n\n"
				, tabstr
			   );
	}

	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		fprintf(pcmd->cFile
				, "%s\te = new_e;\n\n"
				, tabstr
			   );

		fprintf(pcmd->cFile
				, "%s} "
				, tabstr
			   );
	}
}

static void writeOriginalSubFSMLoopInnards(pCMachineData pcmd, pMACHINE_INFO pmi, char *tabstr)
{
	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		if (compact_action_array)
		{
			fprintf(pcmd->cFile
					, "%s\tnew_e = (*%s_action_fns[(*pfsm->actionArray)[e - THIS(%s)][pfsm->state].action])(pfsm);\n\n"
					, tabstr
					, pmi->name->name
					, eventNameByIndex(pmi, 0)
				   );
		}
		else
		{
			fprintf(pcmd->cFile
					, "%s\tnew_e = ((* (*pfsm->actionArray)[e - THIS(%s)][pfsm->state].action)(pfsm));\n\n"
					, tabstr
					, eventNameByIndex(pmi, 0)
				   );
		}
	}
	else
	{
		if (compact_action_array)
		{
			fprintf(pcmd->cFile
					, "%s(*%s_action_fns[(*pfsm->actionArray)[event - %s_%s][pfsm->state].action])(pfsm);\n\n"
					, tabstr
					, pmi->name->name
					, pmi->name->name
					, eventNameByIndex(pmi, 0)
				   );
		}
		else
		{
			fprintf(pcmd->cFile
					, "%s((* (*pfsm->actionArray)[event - %s_%s][pfsm->state].action)(pfsm));\n\n"
					, tabstr
					, pmi->name->name
					, eventNameByIndex(pmi, 0)
				   );
		}
	}

	if (!pmi->transition_fn_list->count)
	{
		fprintf(pcmd->cFile
				, "%s\t%s = (*pfsm->actionArray)[%s - THIS(%s)][pfsm->state].transition;\n\n"
				, tabstr
				, (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
				? "new_s" : "pfsm->state"
				, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
				, eventNameByIndex(pmi, 0)
			   );
	}
	else
	{
		if (compact_action_array)
		{
			fprintf(pcmd->cFile
					, "%s\t%s = (*%s_transition_fns[(*pfsm->actionArray)[%s - THIS(%s)][pfsm->state].transition])(pfsm,%s);\n\n"
					, tabstr
					, (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
					? "new_s" : "pfsm->state"
					, pmi->name->name
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
					, eventNameByIndex(pmi, 0)
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
				   );
		}
		else
		{
			fprintf(pcmd->cFile
					, "%s\t%s = ((* (*pfsm->actionArray)[%s - THIS(%s)][pfsm->state].transition)(pfsm,%s));\n\n"
					, tabstr
					, (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
					? "new_s" : "pfsm->state"
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
					, eventNameByIndex(pmi, 0)
					, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
				   );
		}
	}

	if (pmi->machineTransition || pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		fprintf(pcmd->cFile
				, "\n%s\tif (pfsm->state != new_s)\n\t\t{\n"
				, tabstr
			   );

		if (pmi->machineTransition)
		{
			fprintf(pcmd->cFile
					, "%s\t\t%s_%s(pfsm,new_s);\n"
					, tabstr
					, pmi->name->name
					, pmi->machineTransition->name
				   );
		}

		if (pmi->states_with_exit_fns_count)
		{
			fprintf(pcmd->cFile
					, "\t\trunAppropriateExitFunction(%spfsm->state);\n"
					, pmi->data ? "&pfsm->data, " : ""
				   );
		}

		if (pmi->states_with_entry_fns_count)
		{
			fprintf(pcmd->cFile
					, "\t\trunAppropriateEntryFunction(%snew_s);\n"
					, pmi->data ? "&pfsm->data, " : ""
				   );
		}

		fprintf(pcmd->cFile
				, "%s\t\tpfsm->state = new_s;\n\n"
				, tabstr
			   );

		fprintf(pcmd->cFile
				, "%s\t}\n\n"
				, tabstr
			   );
	}

	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		fprintf(pcmd->cFile
				, "%s\te = new_e;\n\n"
				, tabstr
			   );

		fprintf(pcmd->cFile
				, "%s} "
				, tabstr
			   );
	}
}

static void writeOriginalFSMLoop(pCMachineData pcmd, pMACHINE_INFO pmi)
{
#ifdef FSMLANG_DEVELOP
	fprintf(pcmd->cFile
			, "/* writeOriginalFSMLoop */\n"
		   );
#endif
	char *tabstr = "\t";

	if (pmi->data_block_count)
	{
		fprintf(pcmd->cFile
				, "\ttranslateEventData(&pfsm->data, event);\n\n"
			   );
	}

	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		fprintf(pcmd->cFile, "\twhile (e != ");
		printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->cFile, "_noEvent) {\n\n");
	}

	printFSMMachineDebugBlock(pcmd, pmi);

	fprintf(pcmd->cFile
			, "\t/* This is read-only data to facilitate error reporting in action functions */\n"
		   );

	fprintf(pcmd->cFile
			, "\tpfsm->event = %s;\n\n"
			, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
		   );

	if (pmi->machine_list)
	{
		fprintf(pcmd->cFile
				, "\t\tif (e < %s_noEvent)\n\t\t{\n\n"
				, pmi->name->name
			   );
		tabstr = "\t\t";
	}

	writeOriginalFSMLoopInnards(pcmd, pmi, tabstr);

	if (pmi->machine_list)
	{
		fprintf(pcmd->cFile
				, "\n\t\telse\n\t\t{\n"
			   );

		if (pmi->submachine_inhibitor_count)
		{
			fprintf(pcmd->cFile
					, "\t\t\tif (doNotInhibitSubMachines(pfsm->state))\n\t"
				   );
		}
		fprintf(pcmd->cFile
				, "\t\t\te = findAndRunSubMachine(pfsm, e);\n\t\t}\n\n\t}"
			   );
	}

}

static void writeOriginalSubFSMLoop(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	char *tabstr = "\t";

	if (!(pmi->modFlags & mfActionsReturnVoid))
	{
		fprintf(pcmd->cFile
				, "\twhile (\n\t\t(e != THIS(noEvent))\n\t\t&& (e >= THIS(%s))\n\t)\n\t{\n\n"
				, eventNameByIndex(pmi, 0)
			   );
	}

	fprintf(pcmd->cFile, "#ifdef ");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, "_DEBUG\n");
	fprintf(pcmd->cFile
			, "if (EVENT_IS_NOT_EXCLUDED_FROM_LOG(%s))\n{\n"
			, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
		   );

	printFSMSubMachineDebugBlock(pcmd, pmi);

	fprintf(pcmd->cFile
			, "\t/* This is read-only data to facilitate error reporting in action functions */\n"
		   );

	fprintf(pcmd->cFile
			, "\tpfsm->event = %s;\n\n"
			, (pmi->modFlags & mfActionsReturnVoid) ? "event" : "e"
		   );

	if (pmi->machine_list)
	{
		fprintf(pcmd->cFile
				, "\t\tif (e < THIS(noEvent))\n\t\t{\n\n"
			   );
		tabstr = "\t\t";
	}

	writeOriginalSubFSMLoopInnards(pcmd, pmi, tabstr);

	if (pmi->machine_list)
	{
		fprintf(pcmd->cFile
				, "\n\t\telse\n\t\t{\n"
			   );

		fprintf(pcmd->cFile
				, "\t\t\tnew_e = ((* (*pfsm->subMachineArray)[pfsm->active_sub_machine]->subFSM)(e));\n\n\t\t}\n\n\t}"
			   );
	}

}

static void declareCMachineActionArray(pCMachineData pcmd, pMACHINE_INFO pmi)
{

	/*
	  Actions which return events or void have state transitions
		stored in a struct with the function pointer.
  
	  Actions which return states do not.
	*/
	if (pmi->modFlags & mfActionsReturnStates)
	{

		/* publish the array */
		fprintf(pcmd->hFile, "extern const ");
		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile, "_ACTION_FN ");
		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile, "_ACTION_ARRAY[");
		printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->hFile, "_numEvents][");
		printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->hFile, "_numStates];\n\n");

	}
	else
	{

		/* build the structure for the action array */

		/* 
		   if compacting, we will use an array for the action functions,
		   and the transition functions, should they exist
		 */
		if (compact_action_array)
		{
			declareCMachineActionFnEnum(pcmd, pmi);
			if (pmi->transition_fn_list->count)
			{
				declareCMachineTransitionFnEnum(pcmd, pmi);
			}
		}

		/* now do the action/transition array */
		fprintf(pcmd->hFile, "typedef struct _");
		printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->hFile, "_action_trans_struct_ {\n\t");

		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile
				, "_ACTION_FN%s\taction;\n\t"
				, compact_action_array ? "_E" : ""
			   );

		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile
				, "_%s%s\ttransition;\n} "
				, pmi->transition_fn_list->count ? "TRANSITION_FN" : "STATE"
				, (pmi->transition_fn_list->count && compact_action_array) ? "_E" : ""
			   );
		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile, "_ACTION_TRANS, *p");
		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile, "_ACTION_TRANS;\n\n");

		/* publish the array */
		fprintf(pcmd->hFile, "extern const ");
		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile, "_ACTION_TRANS ");
		printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->hFile, "_action_array[");
		printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->hFile, "_numEvents][");
		printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->hFile, "_numStates];\n\n");

	}

}

static void declareCMachineActionFnEnum(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	ITERATOR_CALLBACK_HELPER ich = { 0 };

	ich.ih.first = true;
	ich.pcmd  = pcmd;
	ich.ih.pmi   = pmi;

	/* enum */
	fprintf(pcmd->hFile
			, "typedef enum\n{\n"
		   );

	iterate_list(pmi->action_list, declare_action_enum_member, &ich);

	/* declare the dummy, or no op action */
	fprintf(pcmd->hFile, ", ");
	printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->hFile, "_noAction_e\n");

	fprintf(pcmd->hFile, "} __attribute__((__packed__)) ");
	printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->hFile, "_ACTION_FN_E;\n\n");

}

static void defineCMachineActionFnArray(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	ITERATOR_CALLBACK_HELPER ich = { 0 };

	ich.ih.first = true;
	ich.pcmd  = pcmd;
	ich.ih.pmi   = pmi;


	/* open the array */
	fprintf(pcmd->cFile
			, "const %s_ACTION_FN %s_action_fns[] =\n{\n"
			, fsmType(pcmd)
			, fqMachineName(pcmd)
			);

	/* fill the array */
	iterate_list(pmi->action_list,  declare_action_array_member, &ich);

	/* declare the dummy, or no op action and close the array */
	fprintf(pcmd->cFile
			, "\t,  %s_noAction\n};\n\n"
			, fqMachineName(pcmd)
			);

}

static void declareCMachineTransitionFnEnum(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	ITERATOR_CALLBACK_HELPER ich = { 0 };

	ich.ih.first = true;
	ich.pcmd  = pcmd;
	ich.ih.pmi   = pmi;

	/* enum */
	fprintf(pcmd->hFile
			, "typedef enum\n{\n"
		   );

	iterate_list(pmi->transition_fn_list
				 , declare_transition_fn_enum_member
				 , &ich
				 );
	iterate_list(pmi->transition_list
				 , declare_transition_enum_member
				 , &ich
				 );

	fprintf(pcmd->hFile
			, ", %s_noTransition_e\n"
			, machineName(pcmd)
			);

	fprintf(pcmd->hFile, "} __attribute__ (( __packed__ )) ");
	fprintf(pcmd->hFile
			, "%s_TRANSITION_FN_E;\n\n"
			, fsmType(pcmd)
			);

}

static void defineCMachineTransitionFnArray(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	ITERATOR_CALLBACK_HELPER ich = { 0 };

	ich.ih.first = true;
	ich.pcmd  = pcmd;
	ich.ih.pmi   = pmi;


	/* open the array */
	fprintf(pcmd->cFile
			, "const %s_TRANSITION_FN %s_transition_fns[] =\n{\n"
			, fsmType(pcmd)
			, fqMachineName(pcmd)
			);

	/* fill the array */
	iterate_list(pmi->transition_fn_list
				 , define_transition_fn_array_member
				 , &ich
				);
	iterate_list(pmi->transition_list
				 , define_transition_array_member
				 , &ich
				);

	fprintf(pcmd->cFile
			, "\t, %s_noTransitionFn\n"
			, fqMachineName(pcmd)
			);

	fprintf(pcmd->cFile
			, "};\n\n"
		   );

}


static void declareCMachineStruct(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	/* put the machine structure definition into the header */
	fprintf(pcmd->hFile, "struct _");
	printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->hFile, "_struct_ {\n");

	if (pmi->data)
	{
		fprintf(pcmd->hFile, "\t");
		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile, "_DATA\t\t\t\t\tdata;\n");
	}

	fprintf(pcmd->hFile, "\t");
	printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->hFile, "_STATE\t\t\t\t\tstate;\n");

	fprintf(pcmd->hFile, "\t");
	streamHungarianToUnderbarCaps(pcmd->hFile, ultimateAncestor(pmi)->name->name);
	fprintf(pcmd->hFile
			, "_EVENT%s\t\t\t\t\tevent;\n"
			, ultimateAncestor(pmi)->data_block_count ? "_ENUM"  : ""
		   );

	fprintf(pcmd->hFile, "\t");
	printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->hFile, "_ACTION_%s const\t(*actionArray)["
			, (pmi->modFlags & mfActionsReturnStates) ? "FN" : "TRANS"
		   );
	printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->hFile, "_numEvents][");
	printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->hFile, "_numStates];\n");

	if (pmi->machine_list)
	{
		fprintf(pcmd->hFile, "\tp");
		printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
		fprintf(pcmd->hFile, "_SUB_FSM_IF\t(*subMachineArray)[");
		printAncestry(pmi, pcmd->hFile, "_", alc_lower, ai_include_self);
		fprintf(pcmd->hFile, "_numSubMachines];\n");
	}

	fprintf(pcmd->hFile, "\t");
	printAncestry(pmi, pcmd->hFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->hFile, "_FSM\t\t\t\t\t\tfsm;\n};\n\n");

}

static void defineActionArray(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	/* 
	   if compacting, we will use an array for the action functions,
	   and the transition functions, should they exist
	 */
	if (compact_action_array)
	{
		defineCMachineActionFnArray(pcmd, pmi);
		if (pmi->transition_fn_list->count)
		{
			defineCMachineTransitionFnArray(pcmd, pmi);
		}
	}

	fprintf(pcmd->cFile, "const ");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, "_ACTION_%s "
			, (pmi->modFlags & mfActionsReturnStates) ? "FN" : "TRANS"
		   );
	printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->cFile, "_action_array[");
	printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->cFile, "_numEvents][");
	printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->cFile, "_numStates] =\n{\n");

	for (unsigned i = 0; i < pmi->event_list->count; i++)
	{

		fprintf(pcmd->cFile, "\t{\n");

		fprintf(pcmd->cFile, "\t\t/* -- %s -- */\n\n",
				eventNameByIndex(pmi, i));

		for (unsigned j = 0; j < pmi->state_list->count; j++)
		{

			fprintf(pcmd->cFile, "\t\t/* -- %s -- */\t",
					stateNameByIndex(pmi, j)
				   );

			if (j) fprintf(pcmd->cFile, ", ");

			if (pmi->actionArray[i][j])
			{

				if (pmi->modFlags & mfActionsReturnStates)
				{
					printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
					if (strlen(pmi->actionArray[i][j]->action->name))
					{
						fprintf(pcmd->cFile, "_%s\n",
								pmi->actionArray[i][j]->action->name);
					}
					else
					{
						fprintf(pcmd->cFile
								, (pmi->actionArray[i][j]->transition->type == STATE)
								? "_transitionTo%s\n"
								: "_%s\n"
								, pmi->actionArray[i][j]->transition->name
							   );
					}
				}
				else //if (pmi->modFlags & mfActionsReturnStates)
				{

					fprintf(pcmd->cFile, "{");

					/* also handle the transition only case */
					printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
					fprintf(pcmd->cFile
							, "_%s%s,"
							, strlen(pmi->actionArray[i][j]->action->name)
							? pmi->actionArray[i][j]->action->name
							: "noAction"
							, compact_action_array ? "_e" : ""
						   );


					printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
					if (pmi->transition_fn_list->count)
					{
						if (pmi->actionArray[i][j]->transition)
						{
							fprintf(pcmd->cFile
									, pmi->actionArray[i][j]->transition->type == STATE
									? "_transitionTo%s%s"
									: "_%s%s"
									, pmi->actionArray[i][j]->transition->name
									, compact_action_array
									? "_e"
									: ""
								   );
						}
						else
						{
							fprintf(pcmd->cFile
									, "_noTransition%s"
									, compact_action_array
									? "_e"
									: pmi->transition_fn_list->count
									? "Fn"
									: ""
								   );
						}
					}
					else // if (pmi->transition_fn_list->count)
					{
						fprintf(pcmd->cFile, "_%s",
								pmi->actionArray[i][j]->transition ?
								pmi->actionArray[i][j]->transition->name :
								stateNameByIndex(pmi, j));
					}

					fprintf(pcmd->cFile, "}\n");

				}

			}
			else // if (pmi->actionArray[i][j])
			{

				/* we need to insert a dummy here */
				if (pmi->modFlags & mfActionsReturnStates)
				{

					printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
					fprintf(pcmd->cFile, "_noTransition%s\n",
							compact_action_array
							? "_e"
							: pmi->transition_fn_list->count
							? "Fn"
							: ""
						   );

				}
				else
				{

					fprintf(pcmd->cFile, "{");

					printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
					fprintf(pcmd->cFile
							, "_noAction%s, "
							, compact_action_array ? "_e" : ""
						   );
					printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
					fprintf(pcmd->cFile
							, "_%s%s"
							, (pmi->transition_fn_list->count == 0)
							? stateNameByIndex(pmi, j)
							: "noTransition"
							, pmi->transition_fn_list->count
							? compact_action_array
							? "_e"
							: "Fn"
							: ""
						   );

					fprintf(pcmd->cFile, "}\n");

				}

			}

		}
		fprintf(pcmd->cFile, "\t},\n");

	}
	fprintf(pcmd->cFile, "};\n");

}

static void defineCMachineFSM(pCMachineData pcmd, pMACHINE_INFO pmi)
{

	if (pmi->machine_list)
	{
		declareSubMachineManagers(pcmd, pmi);
	}

	if (pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		declareStateEntryAndExitManagers(pcmd, pmi);
	}

	if (pmi->data_block_count)
	{
		declareEventDataManager(pcmd);
	}

	if (pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		declareStateEntryAndExitManagers(pcmd, pmi);
	}

	fprintf(pcmd->cFile
			, "#ifndef EVENT_IS_NOT_EXCLUDED_FROM_LOG\n"
		   );

	fprintf(pcmd->cFile
			, "#define EVENT_IS_NOT_EXCLUDED_FROM_LOG(e) ((e) == (e))\n"
		   );

	fprintf(pcmd->cFile
			, "#endif\n"
		   );

	fprintf(pcmd->cFile, "void ");
	printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->cFile, "FSM(p");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile
			, " pfsm, %s"
			, pmi->data_block_count ? "p"  : ""
			);
	streamHungarianToUnderbarCaps(pcmd->cFile, ultimateAncestor(pmi)->name->name);
	fprintf(pcmd->cFile, "_EVENT event)\n{\n");

	if      (pmi->modFlags & mfReentrant)           writeReentrantFSM(pcmd, pmi);
	else if (pmi->modFlags & mfActionsReturnStates) writeActionsReturnStateFSM(pcmd, pmi);
	else                                            writeOriginalFSM(pcmd, pmi);

}

static void defineCSubMachineFSM(pCMachineData pcmd, pMACHINE_INFO pmi)
{
	if (pmi->states_with_entry_fns_count || pmi->states_with_exit_fns_count)
	{
		declareStateEntryAndExitManagers(pcmd, pmi);
	}

	fprintf(pcmd->cFile
			, "#ifndef EVENT_IS_NOT_EXCLUDED_FROM_LOG\n"
		   );

	fprintf(pcmd->cFile
			, "#define EVENT_IS_NOT_EXCLUDED_FROM_LOG(e) (e == e)\n"
		   );

	fprintf(pcmd->cFile
			, "#endif\n"
		   );

	streamHungarianToUnderbarCaps(pcmd->cFile, ultimateAncestor(pmi)->name->name);
	fprintf(pcmd->cFile, "_EVENT%s "
			, ultimateAncestor(pmi)->data_block_count ? "_ENUM" : ""
		   );
	printAncestry(pmi, pcmd->cFile, "_", alc_lower, ai_include_self);
	fprintf(pcmd->cFile, "FSM(p");
	printAncestry(pmi, pcmd->cFile, "_", alc_upper, ai_include_self);
	fprintf(pcmd->cFile, " pfsm, ");
	streamHungarianToUnderbarCaps(pcmd->cFile, ultimateAncestor(pmi)->name->name);
	fprintf(pcmd->cFile, "_EVENT%s "
			, ultimateAncestor(pmi)->data_block_count ? "_ENUM" : ""
		   );
	fprintf(pcmd->cFile, " event)\n{\n");

	if      (pmi->modFlags & mfReentrant)           writeReentrantFSM(pcmd, pmi);
	else if (pmi->modFlags & mfActionsReturnStates) writeActionsReturnStateFSM(pcmd, pmi);
	else                                            writeOriginalSubFSM(pcmd, pmi);

}

pFSMOutputGenerator generateCMachineWriter(pFSMOutputGenerator parent)
{
	pFSMOutputGenerator pfsmog;

	if (parent)
	{
		pFSMCSubMachineOutputGenerator pfsmcsmog;

		pfsmcsmog = (pFSMCSubMachineOutputGenerator)calloc(1, sizeof(FSMCSubMachineOutputGenerator));

		pfsmcsmog->fsmog.writeMachine = writeCSubMachine;
		pfsmcsmog->fsmog.initOutput   = initCSubMachine;
		pfsmcsmog->fsmog.closeOutput  = closeCMachine;

		pfsmcsmog->top_level_fsmcog = &CMachineWriter;
		pfsmcsmog->parent_fsmcog    = (pFSMCOutputGenerator)parent;

		pfsmog =  (pFSMOutputGenerator)pfsmcsmog;
	}
	else
	{
		pfsmog =  (pFSMOutputGenerator)&CMachineWriter;
	}

	if (output_generated_file_names_only)
	{
		pfsmog->initOutput   = initCMachineFN;
		pfsmog->writeMachine = writeCMachineFN;
		pfsmog->closeOutput  = closeCMachineFN;
	}

	return pfsmog;
}

#ifdef FSM_C_TEST

	#include "y.tab.h"

ID_INFO id_info8;
ID_INFO id_info7;
ID_INFO id_info6;
ID_INFO id_info5;
ID_INFO id_info4;
ID_INFO id_info3;
ID_INFO id_info2;
ID_INFO id_info1;

ACTION_SE_INFO a_se_info11 = {
	&id_info6,				NULL
};

ACTION_SE_INFO a_se_info10 = {
	&id_info5,				&a_se_info11
};

ACTION_SE_INFO a_se_info9 = {
	&id_info4,				NULL
};

ACTION_SE_INFO a_se_info8 = {
	&id_info8,				NULL
};

ACTION_SE_INFO a_se_info7 = {
	&id_info7,				&a_se_info8
};

ACTION_SE_INFO a_se_info6 = {
	&id_info4,				NULL
};

ACTION_SE_INFO a_se_info5 = {
	&id_info3,				&a_se_info6
};

ACTION_SE_INFO a_se_info4 = {
	&id_info6,				NULL
};

ACTION_SE_INFO a_se_info3 = {
	&id_info3,				NULL
};

ACTION_SE_INFO a_se_info2 = {
	&id_info2,				&a_se_info3
};

ACTION_SE_INFO a_se_info1 = {
	&id_info5,				NULL
};

ACTION_SE_INFO a_se_info0 = {
	&id_info1,				NULL
};

ACTION_INFO	action_info3 = {
	&a_se_info9,				&a_se_info10,				NULL
};

ACTION_INFO	action_info2 = {
	&a_se_info5,				&a_se_info7,				&id_info1
};

ACTION_INFO	action_info1 = {
	&a_se_info2,				&a_se_info4,				&id_info2
};

ACTION_INFO	action_info0 = {
	&a_se_info0,				&a_se_info1,				NULL
};

ID_INFO id_info12 = {
	"a4",					ACTION,		NULL,				NULL,				NULL,				NULL,					&action_info3,			0
};

ID_INFO id_info11 = {
	"a3",					ACTION,		NULL,				NULL,				NULL,				&id_info12,		&action_info2,			0
};

ID_INFO id_info10 = {
	"a2",					ACTION,		NULL,				NULL,				NULL,				&id_info11,		&action_info1,			0
};

ID_INFO id_info9 = {
	"a1",					ACTION,		NULL,				NULL,				NULL,				&id_info10,		&action_info0,			0
};

ID_INFO id_info8 = {
	"e4",					EVENT,		NULL,				NULL,				NULL,				NULL,					NULL,								3
};

ID_INFO id_info7 = {
	"e3",					EVENT,		&id_info8,	NULL,				&id_info8,	NULL,					NULL,								2
};

ID_INFO id_info6 = {
	"e2",					EVENT,		&id_info7,	NULL,				&id_info7,	NULL,					NULL,								1
};

ID_INFO id_info5 = {
	"e1",					EVENT,		&id_info6,	NULL,				&id_info6,	NULL,					NULL,								0
};

ID_INFO id_info4 = {
	"s4",					STATE,		&id_info5,	NULL,				NULL,				NULL,					NULL,								3
};

ID_INFO id_info3 = {
	"s3",					STATE,		&id_info4,	&id_info4,	NULL,				NULL,					NULL,								2
};

ID_INFO id_info2 = {
	"s2",					STATE,		&id_info3,	&id_info3,	NULL,				NULL,					NULL,								1
};

ID_INFO id_info1 = {
	"s1",					STATE,		&id_info2,	&id_info2,	NULL,				NULL,					NULL,								0
};

ID_INFO id_info0 = {
	"newMachine",	MACHINE,	&id_info1,	NULL,				NULL,				NULL,					NULL,								0
};

MACHINE_INFO machine = {
	&id_info1, 4, &id_info5, 4, &id_info9, &id_info0
};

int main(int argc, char **argv)
{

	pCMachineData	pcmd = newCMachineData("foo");

	if (!populateActionArray(&machine)) writeCMachine(pcmd, &machine);

/*
  id_info0.name = "anotherMachine";

  writeCMachine(pcmd,&machine);
*/
	destroyCMachineData(pcmd, 1);

	return (0);

}
#endif

