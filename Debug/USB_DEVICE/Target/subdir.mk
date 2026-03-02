################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE/Target/usbd_conf.c 

OBJS += \
./USB_DEVICE/Target/usbd_conf.o 

C_DEPS += \
./USB_DEVICE/Target/usbd_conf.d 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE/Target/%.o USB_DEVICE/Target/%.su USB_DEVICE/Target/%.cyclo: ../USB_DEVICE/Target/%.c USB_DEVICE/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_DEVICE/App -I"C:/Users/eren.bülbül/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/BSP/Components/cs43l22" -I"C:/Users/eren.bülbül/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/BSP/Components/Common" -I"C:/Users/eren.bülbül/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/BSP/STM32F4-Discovery" -I"C:/Users/Gökçe/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3/Drivers/BSP/Components/Common" -I"C:/Users/Gökçe/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3/Drivers/BSP/STM32F4-Discovery" -I"C:/Users/Gökçe/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3/Drivers/BSP/Components/cs43l22" -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_DEVICE-2f-Target

clean-USB_DEVICE-2f-Target:
	-$(RM) ./USB_DEVICE/Target/usbd_conf.cyclo ./USB_DEVICE/Target/usbd_conf.d ./USB_DEVICE/Target/usbd_conf.o ./USB_DEVICE/Target/usbd_conf.su

.PHONY: clean-USB_DEVICE-2f-Target

