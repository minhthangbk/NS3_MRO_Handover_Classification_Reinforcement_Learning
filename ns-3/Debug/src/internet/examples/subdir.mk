################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/internet/examples/codel-vs-droptail-asymmetric.cc \
../src/internet/examples/codel-vs-droptail-basic-test.cc \
../src/internet/examples/main-simple.cc 

CC_DEPS += \
./src/internet/examples/codel-vs-droptail-asymmetric.d \
./src/internet/examples/codel-vs-droptail-basic-test.d \
./src/internet/examples/main-simple.d 

OBJS += \
./src/internet/examples/codel-vs-droptail-asymmetric.o \
./src/internet/examples/codel-vs-droptail-basic-test.o \
./src/internet/examples/main-simple.o 


# Each subdirectory must supply rules for building sources it contributes
src/internet/examples/%.o: ../src/internet/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


