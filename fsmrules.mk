####################################################
#
#  .fsm rules
#

.SUFFIXES: .fsm 

ifdef OUTPUT_DIR
FSM=$(OUTPUT_DIR)/fsm $(FSM_FLAGS)
endif

FSM ?= fsm $(FSM_FLAGS)

.fsm.o:
	@echo "FSM:" $(FSM)
	$(FSM) $<
	$(CC) -c $(CFLAGS) $*.c
	rm $*.c

.fsm.c:
	@echo "FSM:" $(FSM)
	$(FSM) $<

.fsm.h:
	@echo "FSM:" $(FSM)
	$(FSM) $<

$(FSM_SRC:.fsm=.h): $(FSM_SRC)

$(FSM_SRC:.fsm=.c): $(FSM_SRC)

$(SRC): $(FSM_SRC:.fsm=.h)

