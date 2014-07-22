################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include icomet/deps/libevent-2.0.21-stable/test/subdir.mk
-include icomet/deps/libevent-2.0.21-stable/sample/subdir.mk
-include icomet/deps/libevent-2.0.21-stable/subdir.mk
-include icomet/deps/jemalloc-3.4.0/test/subdir.mk
-include icomet/deps/jemalloc-3.4.0/src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: icomet

# Tool invocations
icomet: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "icomet" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) icomet
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
