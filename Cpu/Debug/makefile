################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
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
all: Cpu

dependents:
	-cd /home/utnso/git/tp-2017-1c-el-grupo-numero/shared/Debug && $(MAKE) all

# Tool invocations
Cpu: $(OBJS) $(USER_OBJS) /home/utnso/git/tp-2017-1c-el-grupo-numero/shared/Debug/libshared.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/git/tp-2017-1c-el-grupo-numero/shared/Debug" -o "Cpu" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Cpu
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/tp-2017-1c-el-grupo-numero/shared/Debug/libshared.so:

-include ../makefile.targets
