/**
 * revision.c 
 *  
 * Revision information. 
 */

/* 
  Revision 1.1 separates the action/transition array from the machine structure.  Two
  benefits proceed from this: first, the array can be made constant; and second, the machine
  structure can now be instantiated multiple times.  A macro is provided by which machines may
  be easily declared.
*/ 

/*
   Revision 1.2 adds a new option: "-i0" will inhibit the creation of an instance of the machine.
   Any other argument to -i, or not having -i0 will cause the creation of an instance, which is
   the historical behavior.
*/

/*
   Revision 1.3 quiets a warning about  the shadowing of a global variable by a local variable; the warning
   was correct, but the shadowing was not consequential.  In the spirit of "no warnings are the only good warnings"....
*/

/*
   Revision 1.4 adds two new functions:
      * transitions may now be done through functions, rather than designated states.  As with actions, these
        are not previously declared; they are simply stated.  The function arguments are first a pointer to the
        machine, followed by the event causing the transition.
      * both actions and transitions may be given return declarations.  These are in the form of "name returns ...;"
        where "name" is a previously seen action or transition function name.  Presently, this information is used
        only to enhance the HTML page.
*/

/*
   Revision 1.5 adds a new output type modifier, 's'.  Specifying -ts will create a machine in which each state is
      handled by a function, which switches on the event to execute the appropriate action function and select the
      next state.  This type of machine uses no memory, as there is no event/state table; the machine is
      purely code.
 
      Also, the "actions return states" machine is fixed.
*/

/*
   Revision 1.6 adds a new command line option, -c, which will create a more compact event/state table.  The option is
   currently only meaningful when -tc is used with machines having actions which return events.
 
   Compaction is achieved by creating a table of pointers to the action functions along with an enum to index it.  This enum
   and the state enum are used to create the event/state table basic type; and, the enums are declared with the packed
   attribute.  Thus, for machines having less than 256 actions and states, the size of the basic event/state table type struct
   is only 2 bytes.
 
   When transition functions are used, a similar table/enum pair is constructed.
*/

/*
   Revision 1.7:
      * introduces the concept of transition functions to machines having actios which return states.
      * extends the -c option to machines having actions which return void.  This required no code changes.
      * introduces a new machine qualifier: 'on transition ID;' where ID names a function which will be called
        whenever the state changes.  The function takes a pointer to the machine, and the value of the new state.
*/

/*
   Revision 1.8:
      * introduces Hierarchical state machines.
*/

/*
   Revision 1.9:
 
   Removed the concept of an "active" sub state machine; all sub-machines can
   run based on the event given to the top level machine.
*/

/*
   Revision 1.10:
 
   Create weakly linked stubs for all user functions.  Cygwin
   seems to have trouble with this.  It works for Mingwsa. Have
   not tested Linux.
*/

/*
   Revision 1.11:
 
   Remove line-endings from generated DBG_PRINTF statements.
*/

/*
   Revision 1.12:
 
   "Action returns" statements can now reference sub-machine events 
   and "parent" events.
 
*/

/*
   Revision 1.13:
 
   RUN_STATE_MACHINE and DECLARE_%s_MACHINE now enclose parameters in parentheses. 
*/

/*
   Revision 1.14:
 
   returns statements with multiple entries can now end with a namespace event reference.
*/

/*
   Revision 1.15:
 
   Compact table generation correctly references states and state transition functions.
*/

/*
   Revision 1.16:
 
   C Switch Machines now generate any sub-machines.
*/

/*
   Revision 1.17:
 
   Data initialization is mediated through a macro.
*/

/*
   Revision 1.18:
 
   findAndRunSubMachine not declared when no sub-machines are present.
*/

/*
   Revision 1.19:
 
   --generate-weak-fns=false suppresses the generation of weak function stubs.
*/

/*
   Revision 1.20:
 
   --core-logging-only=true suppresses the generation of debug logging outside of the core fsm function.
*/

/*
   Revision 1.21:
 
   Sub-machines no longer have their own event enumerations; rather, all machines use one enumeration created at
   the top level.  This resolves warnings generated by armcc.  Convenience macros are provided for referencing
   the right enumeration.
 
   Sub-machines no longer generate unnecessary state transition function declarations.
*/

/*
   Revision 1.22:
 
   Revision string now available as a macro in generated header file.  String itself shortened to just the numerics.
*/

/*
   Revision 1.23:
 
   enum typedefs now C++ compliant.
   Sub-machine typedefs precede data structure declaration.
*/

/*
   Revision 1.24:
 
   support PlantUML with -tp
   include html image tag with --include-svg-img=true to add
   <img src="[filename].svg" -alt "PlantUML diagram separately generated."/> before the event-state table.
*/

/*
   Revision 1.25:
 
   The event-state table now scrolls when needed.
 
   --css-content-internal=true with -th option copies css content into the html file.  The default is to reference the styleshet externally.

   --css-content-filename=&lt;filename&gt; with -th option uses the indicated file for css content.

*/

/*
   Revision 1.26:
 
   RUN_STATE_MACHINE and DECLARE_xxx_MACHINE comply with MISRA C:2012:20.7
*/

/*
   Revision 1.27:
 
   sub-machines can designate events as being shared from the parent machine.  Code is generated to call the sub-machine whenever the pareent experiences the event.
 
   a <i>native impl(ementation)?</i> block can be defined in the machine qualifier section, analogous to the native block preceding the machine, the <i>native
     implementation</i> block is copied into the source file, rather than the header.
*/

/*
   Revision 1.28:
 
   EOLs which crept back into DBG_PRINTFs removed.
*/

/*
   Revision 1.29:
 
   Declare data translation functions in switch machines
   Allow states to declare that sub-machines are to be inhibited therein
   Put the current event into the FSM struct; this should not be used to make
      decisions (that's what the machine itself is for!), but for refined
      error reporting (or the like) in actions.
*/

/*
   Revision 1.28:
 
   EOLs which crept back into DBG_PRINTFs removed.
*/

/*
   Revision 1.30:
 
   The DECLARE_%s_MACHINE macro is now properly formed
*/

/*
   Revision 1.31:
 
   Input filenames can now contain path information.
*/

/*
   Revision 1.32:
 
   The macro EVENT_IS_NOT_EXCLUDED_FROM_LOG(e) can be used to exclude
   events from the main loop debug log.
 
*/

/*
   Revision 1.33:
 
   Fixed bug in PlantUML generation which resulted in malformed
      transitions when shared events are involved.
 
   Added "numAllEvents" macro to the top level event enumeration.
 
*/

/*
   Revision 1.34:
 
   Fixed bug in PlantUML generation which resulted in malformed
      transitions when shared events are involved.
 
   Added "numAllEvents" macro to the top level event enumeration.
 
*/

/*
   Revision 1.35:
 
   added new option --short-debug-names to generate machine debug info without
   name prefix.
 
   Contributed by maintainer Daniel Jasinsky
*/

/*
   Revision 1.36:
 
   braces can now appear inside of native and data blocks
   
   matrix transition state is now legal as a transition only statement.
   
   transition statements appearing immediately after a sub-machine no longer cause an error.
 
   new command line option --force-generation-of-event-passing-actions forces the generation
   of event passing actions when weak function generation is inhibited.
 
*/

/*
   Revision 1.37:
 
   --add-machine-name can be used to have the machine name included when --short-debug-names is used.
 
   Two macros are introduced: ACTION_RETURN_TYPE, which is defined as the return type for action functions;
     and, FSM_TYPE which is defined as the type of the machine.  These macros can be used to facilitate movement
     of action functions between sub-machines.
 
   Doc comments are now picked up when used with parent::
 
   A new keyword, "all" is added, which must appear in parentheses, to designate a state or event vector
     subsuming all events or states.
 
   States may now have entry and exit functions: <i>on (entry|exit) <function_name></i> where <function_name> is
     optional
 
*/

/*
   Revision 1.38:
 
   Named entry and exit functions are now handled correctly (rather than being swapped). 
*/

const char rev_string[] = "1.38";

