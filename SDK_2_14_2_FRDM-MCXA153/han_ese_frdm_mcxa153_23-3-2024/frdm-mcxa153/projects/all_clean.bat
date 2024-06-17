@echo off

REM @echo Clean demo_examples
REM call:clean_demo_examples .\demo_examples

@echo Clean driver_examples
call:clean_driver_examples .\driver_examples\ctimer\match_interrupt
call:clean_driver_examples .\driver_examples\ctimer\pwm
call:clean_driver_examples .\driver_examples\flexpwm\edge_aligned
call:clean_driver_examples .\driver_examples\gpio\input_interrupt
call:clean_driver_examples .\driver_examples\gpio\input_polling
call:clean_driver_examples .\driver_examples\gpio\output
call:clean_driver_examples .\driver_examples\lpi2c\controller_interrupt_p3t1755
call:clean_driver_examples .\driver_examples\lpi2c\controller_polling_p3t1755
call:clean_driver_examples .\driver_examples\lpspi\master_slave_interrupt
call:clean_driver_examples .\driver_examples\lpspi\master_slave_polling
call:clean_driver_examples .\driver_examples\lptmr\match_interrupt
call:clean_driver_examples .\driver_examples\lpuart\dma
call:clean_driver_examples .\driver_examples\lpuart\interrupt
call:clean_driver_examples .\driver_examples\lpuart\polling
call:clean_driver_examples .\driver_examples\systick\match_interrupt

@echo Clean general_examples
call:clean_general_examples .\general_examples\critical_section
call:clean_general_examples .\general_examples\getting_started
call:clean_general_examples .\general_examples\hardfault

@echo Clean shield_examples
call:clean_shield_examples .\shield_examples\rgb_led
call:clean_shield_examples .\shield_examples\ssd1306\dma
call:clean_shield_examples .\shield_examples\ssd1306\polling

exit /B

:clean_demo_examples <project_path>
    cd %1%\armgcc\
    CALL clean.bat  >nul 2>nul
    cd ..\mcuxpresso
    CALL clean.bat  >nul 2>nul
    cd ..\mdk
    CALL clean.bat  >nul 2>nul
    cd ..\..\..\..
    exit /B

:clean_driver_examples <project_path>
    cd %1%\armgcc\
    CALL clean.bat  >nul 2>nul
    cd ..\mcuxpresso
    CALL clean.bat  >nul 2>nul
    cd ..\mdk
    CALL clean.bat  >nul 2>nul
    cd ..\..\..\..
    exit /B
    
:clean_general_examples <project_path>
    cd %1%\armgcc\
    CALL clean.bat  >nul 2>nul
    cd ..\mcuxpresso
    CALL clean.bat  >nul 2>nul
    cd ..\mdk
    CALL clean.bat  >nul 2>nul
    cd ..\..\..
    exit /B

:clean_shield_examples <project_path>
    cd %1%\armgcc\
    CALL clean.bat  >nul 2>nul
    cd ..\mcuxpresso
    CALL clean.bat  >nul 2>nul
    cd ..\mdk
    CALL clean.bat  >nul 2>nul
    cd ..\..\..
    exit /B
