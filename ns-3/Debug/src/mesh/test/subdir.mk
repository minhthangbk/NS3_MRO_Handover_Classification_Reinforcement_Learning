################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/mesh/test/mesh-information-element-vector-test-suite.cc 

CC_DEPS += \
./src/mesh/test/mesh-information-element-vector-test-suite.d 

OBJS += \
./src/mesh/test/mesh-information-element-vector-test-suite.o 


# Each subdirectory must supply rules for building sources it contributes
src/mesh/test/%.o: ../src/mesh/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


