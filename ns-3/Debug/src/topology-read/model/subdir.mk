################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/topology-read/model/inet-topology-reader.cc \
../src/topology-read/model/orbis-topology-reader.cc \
../src/topology-read/model/rocketfuel-topology-reader.cc \
../src/topology-read/model/topology-reader.cc 

CC_DEPS += \
./src/topology-read/model/inet-topology-reader.d \
./src/topology-read/model/orbis-topology-reader.d \
./src/topology-read/model/rocketfuel-topology-reader.d \
./src/topology-read/model/topology-reader.d 

OBJS += \
./src/topology-read/model/inet-topology-reader.o \
./src/topology-read/model/orbis-topology-reader.o \
./src/topology-read/model/rocketfuel-topology-reader.o \
./src/topology-read/model/topology-reader.o 


# Each subdirectory must supply rules for building sources it contributes
src/topology-read/model/%.o: ../src/topology-read/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


