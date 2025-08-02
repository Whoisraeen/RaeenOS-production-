@echo off
setlocal enableDelayedExpansion

set "output_file=hashes_and_paths.txt"
del "%output_file%" 2>NUL

for /R . %%f in (*) do (
    if exist "%%f" (
        echo %%f >> "%output_file%"
        certutil -hashfile "%%f" MD5 | findstr /R "^[0-9a-fA-F]*$" >> "%output_file%"
    )
)

endlocal
