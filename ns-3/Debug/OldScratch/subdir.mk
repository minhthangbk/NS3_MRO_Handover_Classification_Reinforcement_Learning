################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../OldScratch/Etri14_5to15kmph_ProAlg2.cc \
../OldScratch/RlfModel_Validation_1.cc \
../OldScratch/scratch-simulator.cc 

CC_DEPS += \
./OldScratch/Etri14_5to15kmph_ProAlg2.d \
./OldScratch/RlfModel_Validation_1.d \
./OldScratch/scratch-simulator.d 

OBJS += \
./OldScratch/Etri14_5to15kmph_ProAlg2.o \
./OldScratch/RlfModel_Validation_1.o \
./OldScratch/scratch-simulator.o 


# Each subdirectory must supply rules for building sources it contributes
OldScratch/%.o: ../OldScratch/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


