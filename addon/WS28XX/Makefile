#
# Makefile
#

CIRCLEHOME = ../..

OBJS	= ws28xxstripe.o

libws28xx.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk
