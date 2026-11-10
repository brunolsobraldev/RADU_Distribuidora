
#include <stdio.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "database.h"

int conectarBanco() {
    SQLHENV ambiente;
    SQLHDBC conexao;
    SQLRETURN resultado;


    // Aloca o ambiente ODBC
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &ambiente);

    SQLSetEnvAttr(
        ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    // Cria o identificador da conexao
    SQLAllocHandle(SQL_HANDLE_DBC, ambiente, &conexao);

    // String de conexao
    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

    // Tenta conectar
    resultado = SQLDriverConnect(
        conexao,
        NULL,
        stringConexao,
        SQL_NTS,
        NULL,
        0,
        NULL,
        SQL_DRIVER_NOPROMPT
    );

    if (SQL_SUCCEEDED(resultado)) {
        printf("\nConexao com o banco realizada!\n");
        return 1;
    } else {
        printf("\nErro ao conectar com o banco!\n");
        return 0;
    }
}


/* ============================================================
   IMPRIMIR DIAGNOSTICO ODBC
   ============================================================ */

void imprimirErroODBC(SQLSMALLINT tipoHandle, SQLHANDLE handle) {

    SQLCHAR estado[6];
    SQLCHAR mensagem[400];

    SQLINTEGER erroNativo;
    SQLSMALLINT tamanhoMensagem;

    SQLSMALLINT registro = 1;
    SQLRETURN resultado;

    if (handle == SQL_NULL_HANDLE) {
        return;
    }

    while ((resultado = SQLGetDiagRec(
                tipoHandle,
                handle,
                registro,
                estado,
                &erroNativo,
                mensagem,
                (SQLSMALLINT)sizeof(mensagem),
                &tamanhoMensagem
            )) == SQL_SUCCESS) {

        printf(
            "ODBC [%s] (%ld): %s\n",
            estado,
            (long)erroNativo,
            mensagem
        );

        registro++;
    }
}


/* ============================================================
   ABRIR CONEXAO
   ============================================================ */

int abrirConexao(SQLHENV *ambiente, SQLHDBC *conexao) {

    SQLRETURN resultado;

    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

    if (ambiente == NULL || conexao == NULL) {
        return 0;
    }

    *ambiente = SQL_NULL_HENV;
    *conexao = SQL_NULL_HDBC;

    resultado = SQLAllocHandle(
        SQL_HANDLE_ENV,
        SQL_NULL_HANDLE,
        ambiente
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar ambiente ODBC.\n");
        return 0;
    }

    resultado = SQLSetEnvAttr(
        *ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao configurar ODBC.\n");
        imprimirErroODBC(SQL_HANDLE_ENV, *ambiente);

        SQLFreeHandle(SQL_HANDLE_ENV, *ambiente);
        *ambiente = SQL_NULL_HENV;

        return 0;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_DBC,
        *ambiente,
        conexao
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar conexao.\n");
        imprimirErroODBC(SQL_HANDLE_ENV, *ambiente);

        SQLFreeHandle(SQL_HANDLE_ENV, *ambiente);
        *ambiente = SQL_NULL_HENV;

        return 0;
    }

    resultado = SQLDriverConnect(
        *conexao,
        NULL,
        stringConexao,
        SQL_NTS,
        NULL,
        0,
        NULL,
        SQL_DRIVER_NOPROMPT
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao conectar ao banco de dados.\n");
        imprimirErroODBC(SQL_HANDLE_DBC, *conexao);

        SQLFreeHandle(SQL_HANDLE_DBC, *conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, *ambiente);

        *conexao = SQL_NULL_HDBC;
        *ambiente = SQL_NULL_HENV;

        return 0;
    }

    return 1;
}


/* ============================================================
   FECHAR CONEXAO
   ============================================================ */

void fecharConexao(SQLHENV ambiente, SQLHDBC conexao) {

    if (conexao != SQL_NULL_HDBC) {
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
    }

    if (ambiente != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
    }
}
