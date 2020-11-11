SET PATH=C:\Keil_v5\ARM\ARMCC\Bin;C:\MentorGraphics\9.5PADS\SDD_HOME\common\win32\bin;C:\MentorGraphics\9.5PADS\SDD_HOME\common\win32\lib;C:\MentorGraphics\9.5PADS\MGC_HOME.ixn\bin;C:\MentorGraphics\9.5PADS\MGC_HOME.ixn\lib;C:\Program Files (x86)\Intel\Intel(R) Management Engine Components\iCLS\;C:\Program Files\Intel\Intel(R) Management Engine Components\iCLS\;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files (x86)\Intel\Intel(R) Management Engine Components\DAL;C:\Program Files\Intel\Intel(R) Management Engine Components\DAL;C:\Program Files (x86)\NVIDIA Corporation\PhysX\Common;C:\Program Files\Intel\WiFi\bin\;C:\Program Files\Common Files\Intel\WirelessCommon\;C:\Program Files (x86)\Microchip\xc8\v1.42\bin;C:\Program Files (x86)\Microchip\xc8\v1.45\bin;C:\Program Files\TortoiseSVN\bin;C:\Program Files\VisualSVN Server\bin;;C:\Program Files\NVIDIA Corporation\NVIDIA NvDLISR;C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\;%SYSTEMROOT%\System32\OpenSSH\;C:\Users\AIAQUA\AppData\Local\Microsoft\WindowsApps;;C:\MentorGraphics\9.5PADS\SDD_HOME\CAMCAD;C:\MentorGraphics\9.5PADS\SDD_HOME\common\win32\lib
SET CPU_TYPE=HC32L136J8TA
SET CPU_VENDOR=HDSC
SET UV2_TARGET=Hy-Clor
SET CPU_CLOCK=0x00B71B00
"C:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via ".\output\startup_hc32l136._ia"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\system_hc32l136.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\ctrl.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\keypad.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\main.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\mcu.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\ddl.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\flash.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\gpio.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\interrupts_hc32l136.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\sysctrl.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\bt.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\adc.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\output\timer3.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\output\FW_Compact Line_Vxx_2020.lnp"
"C:\Keil_v5\ARM\ARMCC\Bin\fromelf.exe" ".\output\FW_Compact Line_Vxx_2020.axf" --i32combined --output ".\output\FW_Compact Line_Vxx_2020.hex"
