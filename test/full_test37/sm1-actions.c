#include <string.h>

#include "sub_machine1_priv.h"

ACTION_RETURN_TYPE UFMN(a3)(FSM_TYPE_PTR pfsm)
{
	(void) pfsm;
	DBG_PRINTF(__func__);

   return PARENT(e3);
}

ACTION_RETURN_TYPE UFMN(a2)(FSM_TYPE_PTR pfsm)
{
	(void) pfsm;
	DBG_PRINTF(__func__);

   return THIS(e3);
}

ACTION_RETURN_TYPE UFMN(a1)(FSM_TYPE_PTR pfsm)
{
	(void) pfsm;
	DBG_PRINTF(__func__);

   return THIS(e2);
}

ACTION_RETURN_TYPE UFMN(noAction)(FSM_TYPE_PTR pfsm)
{
	(void) pfsm;
	DBG_PRINTF(__func__);

   return THIS(noEvent);
}

SUB_MACHINE1_STATE UFMN(checkTransition)(FSM_TYPE_PTR pfsm, ACTION_RETURN_TYPE e)
{
	DBG_PRINTF(__func__);

   (void) pfsm;
   (void) e;

   return sub_machine1_s3;
}

void UFMN(translate_e7_data)(pTOP_LEVEL_DATA pfsm_data)
{
	DBG_PRINTF(__func__);

	psub_machine1->data.field1 = pfsm_data->field1;
	memcpy(psub_machine1->data.field2,pfsm_data->field2, sizeof(psub_machine1->data.field2));

	DBG_PRINTF("The int: %d\n", psub_machine1->data.field1);
	DBG_PRINTF("The string: %s\n", psub_machine1->data.field2);
}

ACTION_RETURN_TYPE UFMN(handle_e7)(FSM_TYPE_PTR pfsm)
{
	(void) pfsm;
	DBG_PRINTF(__func__);
	return THIS(noEvent);
}

