################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../build/.conf_check_d8a2c40aecfb7c8eac7dd03fcbfb1110/test.c 

OBJS += \
./build/.conf_check_d8a2c40aecfb7c8eac7dd03fcbfb1110/test.o 

C_DEPS += \
./build/.conf_check_d8a2c40aecfb7c8eac7dd03fcbfb1110/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_d8a2c40aecfb7c8eac7dd03fcbfb1110/%.o: ../build/.conf_check_d8a2c40aecfb7c8eac7dd03fcbfb1110/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


