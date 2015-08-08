@REM Set a temporary variable for the registry query
@SET RegQry=HKLM\Hardware\Description\System\CentralProcessor\0
 
@REM Query the registry and pipe to the CheckOS.txt file
@REG.exe Query %RegQry% > CheckOS.txt

@Find /i "x86" < CheckOS.txt > StringCheck.txt

@If %ERRORLEVEL% == 0 (
    @Echo "32-Bit Architecture"
    @"x86/liw.exe"
) ELSE (
    @Echo "64-Bit Architecture"
    @"x64/liw64.exe"
)

@DEL CheckOS.txt
@DEL StringCheck.txt
