################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/buildings/test/building-position-allocator-test.cc \
../src/buildings/test/buildings-helper-test.cc \
../src/buildings/test/buildings-pathloss-test.cc \
../src/buildings/test/buildings-shadowing-test.cc 

CC_DEPS += \
./src/buildings/test/building-position-allocator-test.d \
./src/buildings/test/buildings-helper-test.d \
./src/buildings/test/buildings-pathloss-test.d \
./src/buildings/test/buildings-shadowing-test.d 

OBJS += \
./src/buildings/test/building-position-allocator-test.o \
./src/buildings/test/buildings-helper-test.o \
./src/buildings/test/buildings-pathloss-test.o \
./src/buildings/test/buildings-shadowing-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/buildings/test/%.o: ../src/buildings/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


