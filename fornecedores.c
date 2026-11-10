#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "fornecedores.h"

void cadastrarFornecedor() {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    /* =====================================================
       VARIÁVEIS DOS DADOS DO FORNECEDOR
       ===================================================== */

    char cnpj[15];
    char razaoSocial[121];
    char nomeFantasia[101];
    char telefone[21];
    char email[121];
    char contatoNome[101];
    char endereco[201];

    /* =====================================================
       1. Coletar dados
       ===================================================== */

    printf("\n==============================================\n");
    printf("          CADASTRO DE FORNECEDOR\n");
    printf("==============================================\n\n");

    printf("CNPJ (14 digitos): ");
    scanf(" %14s", cnpj);

    if (strlen(cnpj) != 14) {
        printf("\nErro: o CNPJ deve possuir exatamente 14 caracteres.\n");
        return;
    }

    printf("Razao Social: ");
    scanf(" %120[^\n]", razaoSocial);

    printf("Nome Fantasia: ");
    scanf(" %100[^\n]", nomeFantasia);

    printf("Telefone: ");
    scanf(" %20[^\n]", telefone);

    printf("Email: ");
    scanf(" %120[^\n]", email);

    printf("Nome do contato: ");
    scanf(" %100[^\n]", contatoNome);

    printf("Endereco: ");
    scanf(" %200[^\n]", endereco);

    /* =====================================================
       2. Criar ambiente ODBC
       ===================================================== */

    resultado = SQLAllocHandle(
        SQL_HANDLE_ENV,
        SQL_NULL_HANDLE,
        &ambiente
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar ambiente ODBC.\n");
        return;
    }

    /* =====================================================
       3. Configurar ODBC
       ===================================================== */

    resultado = SQLSetEnvAttr(
        ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao configurar ODBC.\n");

        SQLFreeHandle(
            SQL_HANDLE_ENV,
            ambiente
        );

        return;
    }

    /* =====================================================
       4. Criar conexao
       ===================================================== */

    resultado = SQLAllocHandle(
        SQL_HANDLE_DBC,
        ambiente,
        &conexao
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar conexao.\n");

        SQLFreeHandle(
            SQL_HANDLE_ENV,
            ambiente
        );

        return;
    }

    /* =====================================================
       5. String de conexao
       ===================================================== */

    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

    /* =====================================================
       6. Conectar ao SQL Server
       ===================================================== */

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

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao conectar ao banco.\n");

        SQLFreeHandle(
            SQL_HANDLE_DBC,
            conexao
        );

        SQLFreeHandle(
            SQL_HANDLE_ENV,
            ambiente
        );

        return;
    }

    /* =====================================================
       7. Criar statement
       ===================================================== */

    resultado = SQLAllocHandle(
        SQL_HANDLE_STMT,
        conexao,
        &comando
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao criar comando SQL.\n");

        SQLDisconnect(conexao);

        SQLFreeHandle(
            SQL_HANDLE_DBC,
            conexao
        );

        SQLFreeHandle(
            SQL_HANDLE_ENV,
            ambiente
        );

        return;
    }

    /* =====================================================
       8. SQL PARAMETRIZADO
       ===================================================== */

    SQLCHAR consulta[] =
        "INSERT INTO dbo.FORNECEDOR "
        "(CNPJ, Razao_Social, Nome_Fantasia, Telefone, "
        "Email, Contato_Nome, Endereco) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    resultado = SQLPrepare(
        comando,
        consulta,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao preparar consulta SQL.\n");

        SQLFreeHandle(
            SQL_HANDLE_STMT,
            comando
        );

        SQLDisconnect(conexao);

        SQLFreeHandle(
            SQL_HANDLE_DBC,
            conexao
        );

        SQLFreeHandle(
            SQL_HANDLE_ENV,
            ambiente
        );

        return;
    }

    /* =====================================================
       9. Associar os parametros
       ===================================================== */

    SQLLEN tamanhoCnpj = SQL_NTS;
    SQLLEN tamanhoRazaoSocial = SQL_NTS;
    SQLLEN tamanhoNomeFantasia = SQL_NTS;
    SQLLEN tamanhoTelefone = SQL_NTS;
    SQLLEN tamanhoEmail = SQL_NTS;
    SQLLEN tamanhoContatoNome = SQL_NTS;
    SQLLEN tamanhoEndereco = SQL_NTS;

    /* CNPJ */
    SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_CHAR,
        14,
        0,
        cnpj,
        sizeof(cnpj),
        &tamanhoCnpj
    );

    /* Razao Social */
    SQLBindParameter(
        comando,
        2,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        120,
        0,
        razaoSocial,
        sizeof(razaoSocial),
        &tamanhoRazaoSocial
    );

    /* Nome Fantasia */
    SQLBindParameter(
        comando,
        3,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        100,
        0,
        nomeFantasia,
        sizeof(nomeFantasia),
        &tamanhoNomeFantasia
    );

    /* Telefone */
    SQLBindParameter(
        comando,
        4,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        20,
        0,
        telefone,
        sizeof(telefone),
        &tamanhoTelefone
    );

    /* Email */
    SQLBindParameter(
        comando,
        5,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        120,
        0,
        email,
        sizeof(email),
        &tamanhoEmail
    );

    /* Contato */
    SQLBindParameter(
        comando,
        6,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        100,
        0,
        contatoNome,
        sizeof(contatoNome),
        &tamanhoContatoNome
    );

    /* Endereco */
    SQLBindParameter(
        comando,
        7,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        200,
        0,
        endereco,
        sizeof(endereco),
        &tamanhoEndereco
    );

    /* =====================================================
       10. Executar INSERT
       ===================================================== */

    resultado = SQLExecute(comando);

    if (SQL_SUCCEEDED(resultado)) {

        printf("\nFornecedor cadastrado com sucesso!\n");

    } else {

        printf("\nErro ao cadastrar fornecedor.\n");
        printf("Verifique se o CNPJ ou Email ja estao cadastrados.\n");
    }

    /* =====================================================
       11. Liberar recursos
       ===================================================== */

    SQLFreeHandle(
        SQL_HANDLE_STMT,
        comando
    );

    SQLDisconnect(conexao);

    SQLFreeHandle(
        SQL_HANDLE_DBC,
        conexao
    );

    SQLFreeHandle(
        SQL_HANDLE_ENV,
        ambiente
    );
}