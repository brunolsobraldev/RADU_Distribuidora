@echo off
cd /d "%~dp0"

echo.
echo ====================================
echo   COMPILANDO RADU DISTRIBUIDORA
echo ====================================
echo.

gcc main.c database.c produtos.c fornecedores.c estoque.c vendas.c relatorios.c -o radu.exe -lodbc32

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ====================================
    echo   FALHA NA COMPILACAO
    echo ====================================
    exit /b %ERRORLEVEL%
)

echo.
echo ====================================
echo   COMPILACAO CONCLUIDA: radu.exe
echo ====================================
