/**
	fsm_html.c

		Creates HTML page and table to describe Finite State Machines


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

	Creation: 	sstanton		Mar-13-2002
*/

#include "fsm_html.h"

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#if defined (LINUX) || defined (CYGWIN)
#include <string.h>
#include <unistd.h>
#endif

/*
	Our interface to the outside world
*/
int initHTMLWriter(pMACHINE_INFO);
void writeHTMLWriter(pMACHINE_INFO);
void closeHTMLWriter(int);

FSMOutputGenerator HTMLMachineWriter = {
	initHTMLWriter,
	writeHTMLWriter,
	closeHTMLWriter
};

pFSMOutputGenerator pHTMLMachineWriter = &HTMLMachineWriter;


/*
	Our internal data
*/
typedef struct _html_machine_data_ HTMLMachineData, *pHTMLMachineData;

struct _html_machine_data_ {

	FILE	*htmlFile
				;

	char	*htmlName
				;

};

HTMLMachineData htmlMachineData = {
		NULL
	,	NULL
};

int initHTMLWriter (pMACHINE_INFO pmi)
{

	time_t		now;

    if (!pmi->outFileBase)
    {

		htmlMachineData.htmlFile = stdout;

	}
	else {

		htmlMachineData.htmlName = createFileName(pmi->outFileBase,".html");

		if (!(htmlMachineData.htmlFile = openFile(htmlMachineData.htmlName,"w"))) {

			CHECK_AND_FREE(htmlMachineData.htmlName);

		}
		else {

			/* we're good to go; write the preamble */

			time(&now);

			fprintf(htmlMachineData.htmlFile,"<!DOCTYPE html>\n<html>\n");

			fprintf(htmlMachineData.htmlFile,"<!--\n\t%s\n\n",htmlMachineData.htmlName);
			fprintf(htmlMachineData.htmlFile,"\tThis file automatically generated by FSMLang\n\n");
			fprintf(htmlMachineData.htmlFile,"\tOn %s\n\n-->\n",ctime(&now));

			fprintf(htmlMachineData.htmlFile,"<head>\n");

			fprintf(htmlMachineData.htmlFile,"<title>FSM Lang : %s</title>\n",pmi->outFileBase);

            if (!pmi->html_info.css_content_internal)
            {
                fprintf(htmlMachineData.htmlFile
                        , "<link REL=stylesheet type=\"text/css\" href=\"%s\">\n"
                        , pmi->html_info.css_content_filename ? pmi->html_info.css_content_filename : "fsmlang.css"
                        );
            }
            else
            {
                fprintf(htmlMachineData.htmlFile, "<style>\n");
                if (copyFileContents(htmlMachineData.htmlFile, pmi->html_info.css_content_filename))
                {
                    fprintf(stderr,"%s: Could not copy css file contents\n",me);
                    return (1);
                }
                fprintf(htmlMachineData.htmlFile, "</style>\n");
            }

			fprintf(htmlMachineData.htmlFile,"</head><body>\n");

		}

	}

	/* this may look funny, but it does the trick */
	return ((int) !htmlMachineData.htmlFile);

}

void writeHTMLWriter(pMACHINE_INFO pmi)
{

	pID_INFO			  pid;
	pACTION_INFO	  pai;
  pACTION_SE_INFO pasei;
	int						  e,s;

	if (!pmi)

		return;

	fprintf(htmlMachineData.htmlFile,"<h2>%s</h2>\n"
		, pmi->name->name
		);

	if (pmi->name->docCmnt)
		fprintf(htmlMachineData.htmlFile,"<p>%s<p>\n",pmi->name->docCmnt);

	fprintf(htmlMachineData.htmlFile,"<table class=machine>\n");

	/* first row */
	fprintf(htmlMachineData.htmlFile,"\t<tr>\n");
	fprintf(htmlMachineData.htmlFile,"\t\t<th class=blankCorner rowspan=2 colspan=2>&nbsp;</th>\n");
	fprintf(htmlMachineData.htmlFile,"\t\t<th class=eventLabel colspan=%d>%s</th>\n"
		,pmi->event_count	
		,"Events"
		);
	fprintf(htmlMachineData.htmlFile,"\t</tr>\n");

	/* event names row */
	fprintf(htmlMachineData.htmlFile,"\t<tr>\n");
	for (e = 0;e < pmi->event_count;e++)
		fprintf(htmlMachineData.htmlFile,"\t\t<th class=eventName>%s</th>\n"
			, eventNameByIndex(pmi,e)
			);
	fprintf(htmlMachineData.htmlFile,"\t</tr>\n");

	/* State Label column */
	fprintf(htmlMachineData.htmlFile,"\t<tr>\n");
	fprintf(htmlMachineData.htmlFile,"\t\t<th class=stateLabel rowspan=%d>%s</th>\n"
		, pmi->state_count
		, "S<br/>t<br/>a<br/>t<br/>e<br/>s"
		);

	/* now, it gets a bit tricky with the row breaks */
	for (s = 0; s < pmi->state_count; s++) {

		if (s)

			fprintf(htmlMachineData.htmlFile,"\t<tr>\n");

		fprintf(htmlMachineData.htmlFile,"\t\t<th class=stateName>%s</th>\n"
			,	stateNameByIndex(pmi,s)
			);

			for (e = 0; e < pmi->event_count; e++) {

				if (pmi->modFlags & mfActionsReturnStates) {

					fprintf(htmlMachineData.htmlFile
                  ,"\t\t<td class=%s>%s"
                  ,	pmi->actionArray[e][s] ?
								     (strlen(pmi->actionArray[e][s]->action->name) 
                        ? "action" : "noAction") 
                        : "nullAction"
                  ,	pmi->actionArray[e][s] 
                        ? (strlen(pmi->actionArray[e][s]->action->name) 
                           ? pmi->actionArray[e][s]->action->name 
                           : "transition") 
                        : "Not Defined"
                  );

          if (
              pmi->actionArray[e][s]
             && pmi->actionArray[e][s]->action->action_returns_decl
              )
          {
             fprintf(htmlMachineData.htmlFile
                     ,"<br/>returns:\n\t<ul class=\"return_decl\">\n"
                     );
             for (pasei = pmi->actionArray[e][s]->action->action_returns_decl;
                  pasei;
                  pasei = pasei->next
                  )
             {
                fprintf(htmlMachineData.htmlFile
                        , "\t\t<li>%s</li>\n"
                        , pasei->se->name
                        );
             }
             fprintf(htmlMachineData.htmlFile
                     , "\t</ul>\n"
                     );
          }

          if (
              pmi->actionArray[e][s]
             && pmi->actionArray[e][s]->transition
              )
          {
             fprintf(htmlMachineData.htmlFile
                     , "<br/>%s"
                     , pmi->actionArray[e][s]->transition->name
                     );

             if (pmi->actionArray[e][s]->transition->transition_fn_returns_decl)
             {
                fprintf(htmlMachineData.htmlFile
                        ,"<br/>returns:\n\t<ul class=\"return_decl\">\n"
                        );
                for (pasei = pmi->actionArray[e][s]->transition->transition_fn_returns_decl;
                     pasei;
                     pasei = pasei->next
                     )
                {
                   fprintf(htmlMachineData.htmlFile
                           , "\t\t<li>%s</li>\n"
                           , pasei->se->name
                           );
                }
                fprintf(htmlMachineData.htmlFile
                        , "\t</ul>\n"
                        );
             }
             fprintf(htmlMachineData.htmlFile
                     , "</td>\n"
                     );
             }

          fprintf(htmlMachineData.htmlFile
                  , "</td>\n"
                  );

				}
				else {

					fprintf(htmlMachineData.htmlFile
                  ,"\t\t<td class=%s>%s"
                  ,	pmi->actionArray[e][s] 
                     ? (strlen(pmi->actionArray[e][s]->action->name) 
                        ? "action" 
                        : "noAction") 
                     : "nullAction"
                  ,	pmi->actionArray[e][s] 
                     ? (strlen(pmi->actionArray[e][s]->action->name) 
                        ? pmi->actionArray[e][s]->action->name 
                        : "noAction") 
                     : "&nbsp;"
                  );

          if (
              pmi->actionArray[e][s]
             && pmi->actionArray[e][s]->action->action_returns_decl
              )
          {
             fprintf(htmlMachineData.htmlFile
                     ,"<br/>returns:\n\t<ul class=\"return_decl\">\n"
                     );
             for (pasei = pmi->actionArray[e][s]->action->action_returns_decl;
                  pasei;
                  pasei = pasei->next
                  )
             {
                fprintf(htmlMachineData.htmlFile
                        , "\t\t<li>%s</li>\n"
                        , pasei->se->name
                        );
             }
             fprintf(htmlMachineData.htmlFile
                     , "\t</ul>\n"
                     );
          }
          
          fprintf(htmlMachineData.htmlFile
                  , "<br/><b>transition</b> : %s"
                  , pmi->actionArray[e][s] ? 
                    (pmi->actionArray[e][s]->transition ? 
                      pmi->actionArray[e][s]->transition->name : "none")
                    : "none"
                  );

          if (
              pmi->actionArray[e][s]
              && pmi->actionArray[e][s]->transition 
             && pmi->actionArray[e][s]->transition->transition_fn_returns_decl
              )
          {
             fprintf(htmlMachineData.htmlFile
                     ,"<br/>returns:\n\t<ul class=\"return_decl\">\n"
                     );
             for (pasei = pmi->actionArray[e][s]->transition->transition_fn_returns_decl;
                  pasei;
                  pasei = pasei->next
                  )
             {
                fprintf(htmlMachineData.htmlFile
                        , "\t\t<li>%s</li>\n"
                        , pasei->se->name
                        );
             }
             fprintf(htmlMachineData.htmlFile
                     , "\t</ul>\n"
                     );
          }
          fprintf(htmlMachineData.htmlFile
                  , "</td>\n"
                  );

				}

			}

			fprintf(htmlMachineData.htmlFile,"\t</tr>\n");

	}
	
	fprintf(htmlMachineData.htmlFile,"</table>\n<p>\n");

	/*
    Now, list the events, states, actions, and any transition functions
    with their Document Comments
	*/
	fprintf(htmlMachineData.htmlFile,"<table class=\"elements\">\n");

	fprintf(htmlMachineData.htmlFile,"<tr>\n");
	fprintf(htmlMachineData.htmlFile,"<th colspan=2 align=left>Events</th>\n");
	fprintf(htmlMachineData.htmlFile,"</tr>\n");
	
	for (e = 0; e < pmi->event_count; e++) {

		pid = eventPidByIndex(pmi,e);
	
		fprintf(htmlMachineData.htmlFile,"<tr>\n");
		fprintf(htmlMachineData.htmlFile,"<td class=\"label\">%s</td>\n"
			, pid->name);
		fprintf(htmlMachineData.htmlFile,"<td>%s</td>\n"
			, pid->docCmnt ? pid->docCmnt : "&nbsp;");
		fprintf(htmlMachineData.htmlFile,"</tr>\n");

	}

	fprintf(htmlMachineData.htmlFile,"<tr>\n");
	fprintf(htmlMachineData.htmlFile,"<th colspan=2 align=left>States</th>\n");
	fprintf(htmlMachineData.htmlFile,"</tr>\n");
	
	for (s = 0; s < pmi->state_count; s++) {
	
		pid = statePidByIndex(pmi,s);

		fprintf(htmlMachineData.htmlFile,"<tr>\n");
		fprintf(htmlMachineData.htmlFile,"<td class=\"label\">%s</td>\n"
			, pid->name);
		fprintf(htmlMachineData.htmlFile,"<td>%s</td>\n"
			, pid->docCmnt ? pid->docCmnt : "&nbsp;");
		fprintf(htmlMachineData.htmlFile,"</tr>\n");

	}

	fprintf(htmlMachineData.htmlFile,"<tr>\n");
	fprintf(htmlMachineData.htmlFile,"<th colspan=2 align=left>Actions</th>\n");
	fprintf(htmlMachineData.htmlFile,"</tr>\n");
	
	for (pid = pmi->action_list; pid; pid = pid->nextAction) {
	
		fprintf(htmlMachineData.htmlFile,"<tr>\n");
		fprintf(htmlMachineData.htmlFile,"<td class=\"label\">%s</td>\n"
			, strlen(pid->name) ? pid->name : "transition");
		fprintf(htmlMachineData.htmlFile,"<td>\n");
		fprintf(htmlMachineData.htmlFile,"%s"
            , pid->docCmnt ? pid->docCmnt : "&nbsp;"
            );
    if (pid->action_returns_decl)
    {
       fprintf(htmlMachineData.htmlFile,"\n<br/><br/>Returns:<ul class=\"return_decl\">");
       for (pasei = pid->action_returns_decl; pasei; pasei = pasei->next)
       {
          fprintf(htmlMachineData.htmlFile,"<li>%s</li>\n"
                  ,pasei->se->name
                  );
       }
       fprintf(htmlMachineData.htmlFile,"</ul>\n"
               );
    }
    fprintf(htmlMachineData.htmlFile, "</td>\n</tr>\n");

	}

  if (pmi->transition_fn_count)
  {
     fprintf(htmlMachineData.htmlFile, "<tr>\n");
     fprintf(htmlMachineData.htmlFile,"<th colspan=2 align=left>Transition Functions</th>\n");
     fprintf(htmlMachineData.htmlFile,"</tr>\n");

     for (pid = pmi->transition_fn_list; pid; pid = pid->nextTransitionFn)
     {
        fprintf(htmlMachineData.htmlFile,"<tr>\n");
        fprintf(htmlMachineData.htmlFile,"<td class=\"label\">%s</td>\n"
          , pid->name);
        fprintf(htmlMachineData.htmlFile,"<td>\n");
        fprintf(htmlMachineData.htmlFile,"%s"
                , pid->docCmnt ? pid->docCmnt : "&nbsp;"
                );
        if (pid->transition_fn_returns_decl)
        {
           fprintf(htmlMachineData.htmlFile,"\n<br/><br/>Returns:<ul class=\"return_decl\">");
           for (pasei = pid->transition_fn_returns_decl; pasei; pasei = pasei->next)
           {
              fprintf(htmlMachineData.htmlFile,"<li>%s</li>\n"
                      ,pasei->se->name
                      );
           }
           fprintf(htmlMachineData.htmlFile,"</ul>\n"
                   );
        }
        fprintf(htmlMachineData.htmlFile, "</td>\n</tr>\n");
     }
  }
	
  fprintf(htmlMachineData.htmlFile, "</table>\n");
}

void closeHTMLWriter(int good)
{

	if (good) {

		fprintf(htmlMachineData.htmlFile,"</body>\n</html>\n");

	}

	fclose(htmlMachineData.htmlFile);

	if (!good) {

		unlink(htmlMachineData.htmlName);

	}

	CHECK_AND_FREE(htmlMachineData.htmlName);

}

