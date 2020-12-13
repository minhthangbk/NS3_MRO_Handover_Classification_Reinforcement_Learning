################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../scratch/RlfModel_Validation1_TL_60kmph.cc \
../scratch/RlfModel_Validation_PP_5kmph.cc \
../scratch/RlfModel_Validation_TE_5kmph.cc \
../scratch/RlfModel_Validation_TL_10kmph.cc \
../scratch/RlfModel_Validation_TL_20kmph.cc \
../scratch/RlfModel_Validation_TL_40kmph.cc \
../scratch/RlfModel_Validation_TL_5kmph.cc \
../scratch/RlfModel_Validation_TL_60kmph.cc 

CC_DEPS += \
./scratch/RlfModel_Validation1_TL_60kmph.d \
./scratch/RlfModel_Validation_PP_5kmph.d \
./scratch/RlfModel_Validation_TE_5kmph.d \
./scratch/RlfModel_Validation_TL_10kmph.d \
./scratch/RlfModel_Validation_TL_20kmph.d \
./scratch/RlfModel_Validation_TL_40kmph.d \
./scratch/RlfModel_Validation_TL_5kmph.d \
./scratch/RlfModel_Validation_TL_60kmph.d 

OBJS += \
./scratch/RlfModel_Validation1_TL_60kmph.o \
./scratch/RlfModel_Validation_PP_5kmph.o \
./scratch/RlfModel_Validation_TE_5kmph.o \
./scratch/RlfModel_Validation_TL_10kmph.o \
./scratch/RlfModel_Validation_TL_20kmph.o \
./scratch/RlfModel_Validation_TL_40kmph.o \
./scratch/RlfModel_Validation_TL_5kmph.o \
./scratch/RlfModel_Validation_TL_60kmph.o 


# Each subdirectory must supply rules for building sources it contributes
scratch/%.o: ../scratch/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


