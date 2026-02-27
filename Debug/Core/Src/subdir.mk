################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app_main.c \
../Core/Src/audio_stream.c \
../Core/Src/cs43l22.c \
../Core/Src/filter.c \
../Core/Src/main.c \
../Core/Src/stm32f4_discovery.c \
../Core/Src/stm32f4_discovery_audio.c \
../Core/Src/stm32f4xx_hal_adc.c \
../Core/Src/stm32f4xx_hal_adc_ex.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/app_main.o \
./Core/Src/audio_stream.o \
./Core/Src/cs43l22.o \
./Core/Src/filter.o \
./Core/Src/main.o \
./Core/Src/stm32f4_discovery.o \
./Core/Src/stm32f4_discovery_audio.o \
./Core/Src/stm32f4xx_hal_adc.o \
./Core/Src/stm32f4xx_hal_adc_ex.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/app_main.d \
./Core/Src/audio_stream.d \
./Core/Src/cs43l22.d \
./Core/Src/filter.d \
./Core/Src/main.d \
./Core/Src/stm32f4_discovery.d \
./Core/Src/stm32f4_discovery_audio.d \
./Core/Src/stm32f4xx_hal_adc.d \
./Core/Src/stm32f4xx_hal_adc_ex.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_DEVICE/App -I"C:/Users/eren.bülbül/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/BSP/Components/cs43l22" -I"C:/Users/eren.bülbül/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/BSP/Components/Common" -I"C:/Users/eren.bülbül/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/BSP/STM32F4-Discovery" -I"C:/Users/Gökçe/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3/Drivers/BSP/Components/Common" -I"C:/Users/Gökçe/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3/Drivers/BSP/STM32F4-Discovery" -I"C:/Users/Gökçe/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3/Drivers/BSP/Components/cs43l22" -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app_main.cyclo ./Core/Src/app_main.d ./Core/Src/app_main.o ./Core/Src/app_main.su ./Core/Src/audio_stream.cyclo ./Core/Src/audio_stream.d ./Core/Src/audio_stream.o ./Core/Src/audio_stream.su ./Core/Src/cs43l22.cyclo ./Core/Src/cs43l22.d ./Core/Src/cs43l22.o ./Core/Src/cs43l22.su ./Core/Src/filter.cyclo ./Core/Src/filter.d ./Core/Src/filter.o ./Core/Src/filter.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f4_discovery.cyclo ./Core/Src/stm32f4_discovery.d ./Core/Src/stm32f4_discovery.o ./Core/Src/stm32f4_discovery.su ./Core/Src/stm32f4_discovery_audio.cyclo ./Core/Src/stm32f4_discovery_audio.d ./Core/Src/stm32f4_discovery_audio.o ./Core/Src/stm32f4_discovery_audio.su ./Core/Src/stm32f4xx_hal_adc.cyclo ./Core/Src/stm32f4xx_hal_adc.d ./Core/Src/stm32f4xx_hal_adc.o ./Core/Src/stm32f4xx_hal_adc.su ./Core/Src/stm32f4xx_hal_adc_ex.cyclo ./Core/Src/stm32f4xx_hal_adc_ex.d ./Core/Src/stm32f4xx_hal_adc_ex.o ./Core/Src/stm32f4xx_hal_adc_ex.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src

