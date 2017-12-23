################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Client.cpp \
../src/ClientReadData.cpp \
../src/Config.cpp \
../src/Server.cpp \
../src/ServerReadData.cpp \
../src/main.cpp 

OBJS += \
./src/Client.o \
./src/ClientReadData.o \
./src/Config.o \
./src/Server.o \
./src/ServerReadData.o \
./src/main.o 

CPP_DEPS += \
./src/Client.d \
./src/ClientReadData.d \
./src/Config.d \
./src/Server.d \
./src/ServerReadData.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<" -std=c++11
	@echo 'Finished building: $<'
	@echo ' '


