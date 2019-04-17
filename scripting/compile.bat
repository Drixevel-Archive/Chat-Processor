@ECHO OFF
IF EXIST "compile.dat" ( del /A compile.dat )
spcomp %~n1.sp -o../plugins/%~n1.smx
PAUSE