#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "produtos.h"

/* ============================================================
   LISTAR PRODUTOS
   ============================================================ */

void listarProdutos() {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    resultado = SQLAllocHandle(
        SQL_HANDLE_ENV,
        SQL_NULL_HANDLE,
        &ambiente
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar ambiente ODBC.\n");
        return;
    }

    resultado = SQLSetEnvAttr(
        ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao configurar ODBC.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_DBC,
        ambiente,
        &conexao
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar conexao.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

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

        printf("\nErro ao conectar ao banco de dados.\n");

        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    printf("\n==============================================\n");
    printf("             LISTA DE PRODUTOS\n");
    printf("==============================================\n\n");

    resultado = SQLAllocHandle(
        SQL_HANDLE_STMT,
        conexao,
        &comando
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("Erro ao criar comando SQL.\n");

        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLCHAR consulta[] =
        "SELECT "
        "Cod_Produto, "
        "Nome, "
        "Volume, "
        "Tipo_Recipiente, "
        "Categoria, "
        "Preco_Custo_Unidade, "
        "Preco_Custo_Pacote, "
        "Preco_Venda_Unidade, "
        "Preco_Venda_Pacote, "
        "FK_CNPJ_Fornecedor "
        "FROM dbo.PRODUTO "
        "ORDER BY Cod_Produto;";

    resultado = SQLExecDirect(
        comando,
        consulta,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("Erro ao executar consulta.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLINTEGER codProduto;

    char nome[101];
    char volume[21];
    char tipoRecipiente[31];
    char categoria[51];
    char cnpjFornecedor[15];

    double precoCustoUnidade;
    double precoCustoPacote;
    double precoVendaUnidade;
    double precoVendaPacote;

    SQLLEN indicador;

    while (SQLFetch(comando) == SQL_SUCCESS) {

        SQLGetData(
            comando,
            1,
            SQL_C_SLONG,
            &codProduto,
            sizeof(codProduto),
            &indicador
        );

        SQLGetData(
            comando,
            2,
            SQL_C_CHAR,
            nome,
            sizeof(nome),
            &indicador
        );

        SQLGetData(
            comando,
            3,
            SQL_C_CHAR,
            volume,
            sizeof(volume),
            &indicador
        );

        SQLGetData(
            comando,
            4,
            SQL_C_CHAR,
            tipoRecipiente,
            sizeof(tipoRecipiente),
            &indicador
        );

        SQLGetData(
            comando,
            5,
            SQL_C_CHAR,
            categoria,
            sizeof(categoria),
            &indicador
        );

        SQLGetData(
            comando,
            6,
            SQL_C_DOUBLE,
            &precoCustoUnidade,
            sizeof(precoCustoUnidade),
            &indicador
        );

        SQLGetData(
            comando,
            7,
            SQL_C_DOUBLE,
            &precoCustoPacote,
            sizeof(precoCustoPacote),
            &indicador
        );

        SQLGetData(
            comando,
            8,
            SQL_C_DOUBLE,
            &precoVendaUnidade,
            sizeof(precoVendaUnidade),
            &indicador
        );

        SQLGetData(
            comando,
            9,
            SQL_C_DOUBLE,
            &precoVendaPacote,
            sizeof(precoVendaPacote),
            &indicador
        );

        SQLGetData(
            comando,
            10,
            SQL_C_CHAR,
            cnpjFornecedor,
            sizeof(cnpjFornecedor),
            &indicador
        );

        printf("Codigo: %d\n", codProduto);
        printf("Nome: %s\n", nome);
        printf("Volume: %s\n", volume);
        printf("Tipo de recipiente: %s\n", tipoRecipiente);
        printf("Categoria: %s\n", categoria);

        printf(
            "Preco de custo (unidade): R$ %.2f\n",
            precoCustoUnidade
        );

        printf(
            "Preco de custo (pacote): R$ %.2f\n",
            precoCustoPacote
        );

        printf(
            "Preco de venda (unidade): R$ %.2f\n",
            precoVendaUnidade
        );

        printf(
            "Preco de venda (pacote): R$ %.2f\n",
            precoVendaPacote
        );

        printf(
            "CNPJ do fornecedor: %s\n",
            cnpjFornecedor
        );

        printf("----------------------------------------------\n");
    }

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

    printf("\nConsulta finalizada.\n");
}


/* ============================================================
   CADASTRAR PRODUTO
   ============================================================ */

void cadastrarProduto() {

    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    char nome[101];
    char volume[21];
    char tipoRecipiente[31];
    char categoria[51];
    char cnpjFornecedor[15];

    double precoCustoUnidade;
    double precoCustoPacote;
    double precoVendaUnidade;
    double precoVendaPacote;

    printf("\n==============================================\n");
    printf("             CADASTRO DE PRODUTO\n");
    printf("==============================================\n\n");

    printf("Nome: ");
    fflush(stdout);
    scanf(" %100[^\n]", nome);

    printf("Volume: ");
    fflush(stdout);
    scanf(" %20[^\n]", volume);

    printf("Tipo de recipiente: ");
    fflush(stdout);
    scanf(" %30[^\n]", tipoRecipiente);

    printf("Categoria: ");
    fflush(stdout);
    scanf(" %50[^\n]", categoria);

    printf("Preco de custo por unidade: R$ ");
    fflush(stdout);
    scanf("%lf", &precoCustoUnidade);

    printf("Preco de custo por pacote: R$ ");
    fflush(stdout);
    scanf("%lf", &precoCustoPacote);

    printf("Preco de venda por unidade: R$ ");
    fflush(stdout);
    scanf("%lf", &precoVendaUnidade);

    printf("Preco de venda por pacote: R$ ");
    fflush(stdout);
    scanf("%lf", &precoVendaPacote);

    printf("CNPJ do fornecedor: ");
    fflush(stdout);
    scanf(" %14s", cnpjFornecedor);

    if (strlen(cnpjFornecedor) != 14) {
        printf("\nErro: o CNPJ deve possuir exatamente 14 caracteres.\n");
        return;
    }

    if (precoCustoUnidade < 0 ||
        precoCustoPacote < 0 ||
        precoVendaUnidade < 0 ||
        precoVendaPacote < 0) {

        printf("\nErro: os precos nao podem ser negativos.\n");
        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_ENV,
        SQL_NULL_HANDLE,
        &ambiente
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar ambiente ODBC.\n");
        return;
    }

    resultado = SQLSetEnvAttr(
        ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao configurar ODBC.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_DBC,
        ambiente,
        &conexao
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar conexao.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

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

        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_STMT,
        conexao,
        &comando
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao criar comando SQL.\n");

        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLCHAR consultaFornecedor[] =
        "SELECT COUNT(*) "
        "FROM dbo.FORNECEDOR "
        "WHERE CNPJ = ?;";

    resultado = SQLPrepare(
        comando,
        consultaFornecedor,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao preparar consulta do fornecedor.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLLEN tamanhoCnpj = SQL_NTS;

    resultado = SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_CHAR,
        14,
        0,
        cnpjFornecedor,
        sizeof(cnpjFornecedor),
        &tamanhoCnpj
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao associar CNPJ do fornecedor.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLExecute(comando);

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao consultar fornecedor.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLINTEGER quantidadeFornecedor = 0;
    SQLLEN indicador;

    resultado = SQLFetch(comando);

    if (SQL_SUCCEEDED(resultado)) {

        SQLGetData(
            comando,
            1,
            SQL_C_SLONG,
            &quantidadeFornecedor,
            sizeof(quantidadeFornecedor),
            &indicador
        );
    }

    if (quantidadeFornecedor == 0) {

        printf("\nErro: o fornecedor informado nao existe.\n");
        printf("Cadastre o fornecedor antes de cadastrar o produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    printf("\nFornecedor encontrado.\n");

    SQLFreeStmt(
        comando,
        SQL_CLOSE
    );

    SQLCHAR consultaProduto[] =
        "INSERT INTO dbo.PRODUTO "
        "(Nome, Volume, Tipo_Recipiente, Categoria, "
        "Preco_Custo_Unidade, Preco_Custo_Pacote, "
        "Preco_Venda_Unidade, Preco_Venda_Pacote, "
        "FK_CNPJ_Fornecedor) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    resultado = SQLPrepare(
        comando,
        consultaProduto,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao preparar cadastro do produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLLEN tamanhoNome = SQL_NTS;
    SQLLEN tamanhoVolume = SQL_NTS;
    SQLLEN tamanhoTipoRecipiente = SQL_NTS;
    SQLLEN tamanhoCategoria = SQL_NTS;

    SQLLEN tamanhoPrecoCustoUnidade =
        sizeof(precoCustoUnidade);

    SQLLEN tamanhoPrecoCustoPacote =
        sizeof(precoCustoPacote);

    SQLLEN tamanhoPrecoVendaUnidade =
        sizeof(precoVendaUnidade);

    SQLLEN tamanhoPrecoVendaPacote =
        sizeof(precoVendaPacote);

    SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        100,
        0,
        nome,
        sizeof(nome),
        &tamanhoNome
    );

    SQLBindParameter(
        comando,
        2,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        20,
        0,
        volume,
        sizeof(volume),
        &tamanhoVolume
    );

    SQLBindParameter(
        comando,
        3,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        30,
        0,
        tipoRecipiente,
        sizeof(tipoRecipiente),
        &tamanhoTipoRecipiente
    );

    SQLBindParameter(
        comando,
        4,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        50,
        0,
        categoria,
        sizeof(categoria),
        &tamanhoCategoria
    );

    SQLBindParameter(
        comando,
        5,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoCustoUnidade,
        sizeof(precoCustoUnidade),
        &tamanhoPrecoCustoUnidade
    );

    SQLBindParameter(
        comando,
        6,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoCustoPacote,
        sizeof(precoCustoPacote),
        &tamanhoPrecoCustoPacote
    );

    SQLBindParameter(
        comando,
        7,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoVendaUnidade,
        sizeof(precoVendaUnidade),
        &tamanhoPrecoVendaUnidade
    );

    SQLBindParameter(
        comando,
        8,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoVendaPacote,
        sizeof(precoVendaPacote),
        &tamanhoPrecoVendaPacote
    );

    SQLBindParameter(
        comando,
        9,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_CHAR,
        14,
        0,
        cnpjFornecedor,
        sizeof(cnpjFornecedor),
        &tamanhoCnpj
    );

    resultado = SQLExecute(comando);

    if (SQL_SUCCEEDED(resultado)) {

        printf("\n==============================================\n");
        printf("       PRODUTO CADASTRADO COM SUCESSO!\n");
        printf("==============================================\n");

    } else {

        printf("\nErro ao cadastrar produto.\n");
        printf("Verifique os dados informados.\n");
    }

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


/* ============================================================
   BUSCAR PRODUTO POR CODIGO
   ============================================================ */

void buscarProdutoPorCodigo(int codigo)
{
    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    resultado = SQLAllocHandle(
        SQL_HANDLE_ENV,
        SQL_NULL_HANDLE,
        &ambiente
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar ambiente ODBC.\n");
        return;
    }

    resultado = SQLSetEnvAttr(
        ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao configurar ODBC.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_DBC,
        ambiente,
        &conexao
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar conexao.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

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
        printf("\nErro ao conectar ao banco de dados.\n");

        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_STMT,
        conexao,
        &comando
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar comando SQL.\n");

        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLCHAR consulta[] =
        "SELECT "
        "Cod_Produto, "
        "Nome, "
        "Volume, "
        "Tipo_Recipiente, "
        "Categoria, "
        "Preco_Custo_Unidade, "
        "Preco_Custo_Pacote, "
        "Preco_Venda_Unidade, "
        "Preco_Venda_Pacote, "
        "FK_CNPJ_Fornecedor "
        "FROM dbo.PRODUTO "
        "WHERE Cod_Produto = ?;";

    resultado = SQLPrepare(
        comando,
        consulta,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao preparar consulta do produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLLEN tamanhoCodigo = sizeof(codigo);

    resultado = SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_SLONG,
        SQL_INTEGER,
        0,
        0,
        &codigo,
        sizeof(codigo),
        &tamanhoCodigo
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao associar codigo do produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLExecute(comando);

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao pesquisar produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLINTEGER codProduto;

    char nome[101];
    char volume[21];
    char tipoRecipiente[31];
    char categoria[51];
    char cnpjFornecedor[15];

    double precoCustoUnidade;
    double precoCustoPacote;
    double precoVendaUnidade;
    double precoVendaPacote;

    SQLLEN indicador;

    resultado = SQLFetch(comando);

    if (resultado == SQL_SUCCESS ||
        resultado == SQL_SUCCESS_WITH_INFO) {

        SQLGetData(
            comando,
            1,
            SQL_C_SLONG,
            &codProduto,
            sizeof(codProduto),
            &indicador
        );

        SQLGetData(
            comando,
            2,
            SQL_C_CHAR,
            nome,
            sizeof(nome),
            &indicador
        );

        SQLGetData(
            comando,
            3,
            SQL_C_CHAR,
            volume,
            sizeof(volume),
            &indicador
        );

        SQLGetData(
            comando,
            4,
            SQL_C_CHAR,
            tipoRecipiente,
            sizeof(tipoRecipiente),
            &indicador
        );

        SQLGetData(
            comando,
            5,
            SQL_C_CHAR,
            categoria,
            sizeof(categoria),
            &indicador
        );

        SQLGetData(
            comando,
            6,
            SQL_C_DOUBLE,
            &precoCustoUnidade,
            sizeof(precoCustoUnidade),
            &indicador
        );

        SQLGetData(
            comando,
            7,
            SQL_C_DOUBLE,
            &precoCustoPacote,
            sizeof(precoCustoPacote),
            &indicador
        );

        SQLGetData(
            comando,
            8,
            SQL_C_DOUBLE,
            &precoVendaUnidade,
            sizeof(precoVendaUnidade),
            &indicador
        );

        SQLGetData(
            comando,
            9,
            SQL_C_DOUBLE,
            &precoVendaPacote,
            sizeof(precoVendaPacote),
            &indicador
        );

        SQLGetData(
            comando,
            10,
            SQL_C_CHAR,
            cnpjFornecedor,
            sizeof(cnpjFornecedor),
            &indicador
        );

        printf("\n==============================================\n");
        printf("             PRODUTO ENCONTRADO\n");
        printf("==============================================\n");

        printf("Codigo: %d\n", codProduto);
        printf("Nome: %s\n", nome);
        printf("Volume: %s\n", volume);
        printf("Tipo de recipiente: %s\n", tipoRecipiente);
        printf("Categoria: %s\n", categoria);

        printf(
            "Preco de custo (unidade): R$ %.2f\n",
            precoCustoUnidade
        );

        printf(
            "Preco de custo (pacote): R$ %.2f\n",
            precoCustoPacote
        );

        printf(
            "Preco de venda (unidade): R$ %.2f\n",
            precoVendaUnidade
        );

        printf(
            "Preco de venda (pacote): R$ %.2f\n",
            precoVendaPacote
        );

        printf(
            "CNPJ do fornecedor: %s\n",
            cnpjFornecedor
        );

        printf("==============================================\n");
    }
    else {
        printf(
            "\nProduto com codigo %d nao encontrado.\n",
            codigo
        );
    }

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


/* ============================================================
   ALTERAR PRODUTO
   ============================================================ */

void alterarProduto(int codigo)
{
    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    char nome[101];
    char volume[21];
    char tipoRecipiente[31];
    char categoria[51];
    char cnpjFornecedor[15];

    double precoCustoUnidade;
    double precoCustoPacote;
    double precoVendaUnidade;
    double precoVendaPacote;

    printf("\n==============================================\n");
    printf("             ALTERAR PRODUTO\n");
    printf("==============================================\n");

    /* =====================================================
       1. CONEXAO
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

    resultado = SQLSetEnvAttr(
        ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao configurar ODBC.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_DBC,
        ambiente,
        &conexao
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar conexao.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

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

        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       2. CRIAR STATEMENT
       ===================================================== */

    resultado = SQLAllocHandle(
        SQL_HANDLE_STMT,
        conexao,
        &comando
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar comando SQL.\n");

        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       3. VERIFICAR SE O PRODUTO EXISTE
       ===================================================== */

    SQLCHAR consultaExiste[] =
        "SELECT COUNT(*) "
        "FROM dbo.PRODUTO "
        "WHERE Cod_Produto = ?;";

    resultado = SQLPrepare(
        comando,
        consultaExiste,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao preparar verificacao do produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLLEN tamanhoCodigo = sizeof(codigo);

    resultado = SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_SLONG,
        SQL_INTEGER,
        0,
        0,
        &codigo,
        sizeof(codigo),
        &tamanhoCodigo
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao associar codigo.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLExecute(comando);

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao verificar produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLINTEGER quantidadeProduto = 0;
    SQLLEN indicador;

    if (SQLFetch(comando) == SQL_SUCCESS) {

        SQLGetData(
            comando,
            1,
            SQL_C_SLONG,
            &quantidadeProduto,
            sizeof(quantidadeProduto),
            &indicador
        );
    }

    if (quantidadeProduto == 0) {

        printf(
            "\nProduto com codigo %d nao encontrado.\n",
            codigo
        );

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       4. LIMPAR COMANDO
       ===================================================== */

    SQLFreeStmt(
        comando,
        SQL_CLOSE
    );

    /* =====================================================
       5. DIGITAR NOVOS DADOS
       ===================================================== */

    printf("\nDigite os novos dados do produto.\n\n");

    printf("Novo nome: ");
    fflush(stdout);
    scanf(" %100[^\n]", nome);

    printf("Novo volume: ");
    fflush(stdout);
    scanf(" %20[^\n]", volume);

    printf("Novo tipo de recipiente: ");
    fflush(stdout);
    scanf(" %30[^\n]", tipoRecipiente);

    printf("Nova categoria: ");
    fflush(stdout);
    scanf(" %50[^\n]", categoria);

    printf("Novo preco de custo por unidade: R$ ");
    fflush(stdout);
    scanf("%lf", &precoCustoUnidade);

    printf("Novo preco de custo por pacote: R$ ");
    fflush(stdout);
    scanf("%lf", &precoCustoPacote);

    printf("Novo preco de venda por unidade: R$ ");
    fflush(stdout);
    scanf("%lf", &precoVendaUnidade);

    printf("Novo preco de venda por pacote: R$ ");
    fflush(stdout);
    scanf("%lf", &precoVendaPacote);

    printf("Novo CNPJ do fornecedor: ");
    fflush(stdout);
    scanf(" %14s", cnpjFornecedor);

    /* =====================================================
       6. VALIDACOES
       ===================================================== */

    if (strlen(cnpjFornecedor) != 14) {

        printf(
            "\nErro: o CNPJ deve possuir exatamente 14 caracteres.\n"
        );

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    if (precoCustoUnidade < 0 ||
        precoCustoPacote < 0 ||
        precoVendaUnidade < 0 ||
        precoVendaPacote < 0) {

        printf("\nErro: os precos nao podem ser negativos.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       7. VERIFICAR FORNECEDOR
       ===================================================== */

    SQLCHAR consultaFornecedor[] =
        "SELECT COUNT(*) "
        "FROM dbo.FORNECEDOR "
        "WHERE CNPJ = ?;";

    resultado = SQLPrepare(
        comando,
        consultaFornecedor,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao preparar consulta do fornecedor.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLLEN tamanhoCnpj = SQL_NTS;

    resultado = SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_CHAR,
        14,
        0,
        cnpjFornecedor,
        sizeof(cnpjFornecedor),
        &tamanhoCnpj
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao associar CNPJ do fornecedor.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLExecute(comando);

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao consultar fornecedor.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    quantidadeProduto = 0;

    if (SQLFetch(comando) == SQL_SUCCESS) {

        SQLGetData(
            comando,
            1,
            SQL_C_SLONG,
            &quantidadeProduto,
            sizeof(quantidadeProduto),
            &indicador
        );
    }

    if (quantidadeProduto == 0) {

        printf("\nErro: o fornecedor informado nao existe.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       8. LIMPAR COMANDO
       ===================================================== */

    SQLFreeStmt(
        comando,
        SQL_CLOSE
    );

    /* =====================================================
       9. UPDATE
       ===================================================== */

    SQLCHAR consultaUpdate[] =
        "UPDATE dbo.PRODUTO SET "
        "Nome = ?, "
        "Volume = ?, "
        "Tipo_Recipiente = ?, "
        "Categoria = ?, "
        "Preco_Custo_Unidade = ?, "
        "Preco_Custo_Pacote = ?, "
        "Preco_Venda_Unidade = ?, "
        "Preco_Venda_Pacote = ?, "
        "FK_CNPJ_Fornecedor = ? "
        "WHERE Cod_Produto = ?;";

    resultado = SQLPrepare(
        comando,
        consultaUpdate,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao preparar alteracao do produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLLEN tamanhoNome = SQL_NTS;
    SQLLEN tamanhoVolume = SQL_NTS;
    SQLLEN tamanhoTipo = SQL_NTS;
    SQLLEN tamanhoCategoria = SQL_NTS;

    SQLLEN tamanhoPreco1 = sizeof(precoCustoUnidade);
    SQLLEN tamanhoPreco2 = sizeof(precoCustoPacote);
    SQLLEN tamanhoPreco3 = sizeof(precoVendaUnidade);
    SQLLEN tamanhoPreco4 = sizeof(precoVendaPacote);

    /* Nome */
    SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        100,
        0,
        nome,
        sizeof(nome),
        &tamanhoNome
    );

    /* Volume */
    SQLBindParameter(
        comando,
        2,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        20,
        0,
        volume,
        sizeof(volume),
        &tamanhoVolume
    );

    /* Tipo */
    SQLBindParameter(
        comando,
        3,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        30,
        0,
        tipoRecipiente,
        sizeof(tipoRecipiente),
        &tamanhoTipo
    );

    /* Categoria */
    SQLBindParameter(
        comando,
        4,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_VARCHAR,
        50,
        0,
        categoria,
        sizeof(categoria),
        &tamanhoCategoria
    );

    /* Custo unidade */
    SQLBindParameter(
        comando,
        5,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoCustoUnidade,
        sizeof(precoCustoUnidade),
        &tamanhoPreco1
    );

    /* Custo pacote */
    SQLBindParameter(
        comando,
        6,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoCustoPacote,
        sizeof(precoCustoPacote),
        &tamanhoPreco2
    );

    /* Venda unidade */
    SQLBindParameter(
        comando,
        7,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoVendaUnidade,
        sizeof(precoVendaUnidade),
        &tamanhoPreco3
    );

    /* Venda pacote */
    SQLBindParameter(
        comando,
        8,
        SQL_PARAM_INPUT,
        SQL_C_DOUBLE,
        SQL_DECIMAL,
        10,
        2,
        &precoVendaPacote,
        sizeof(precoVendaPacote),
        &tamanhoPreco4
    );

    /* CNPJ */
    SQLBindParameter(
        comando,
        9,
        SQL_PARAM_INPUT,
        SQL_C_CHAR,
        SQL_CHAR,
        14,
        0,
        cnpjFornecedor,
        sizeof(cnpjFornecedor),
        &tamanhoCnpj
    );

    /* Codigo */
    SQLBindParameter(
        comando,
        10,
        SQL_PARAM_INPUT,
        SQL_C_SLONG,
        SQL_INTEGER,
        0,
        0,
        &codigo,
        sizeof(codigo),
        &tamanhoCodigo
    );

    /* =====================================================
       10. EXECUTAR UPDATE
       ===================================================== */

    resultado = SQLExecute(comando);

    if (SQL_SUCCEEDED(resultado)) {

        printf("\n==============================================\n");
        printf("       PRODUTO ALTERADO COM SUCESSO!\n");
        printf("==============================================\n");

    } else {

        printf("\nErro ao alterar produto.\n");
    }

    /* =====================================================
       11. LIBERAR
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


/* ============================================================
   EXCLUIR PRODUTO
   ============================================================ */

void excluirProduto(int codigo)
{
    SQLHENV ambiente = SQL_NULL_HENV;
    SQLHDBC conexao = SQL_NULL_HDBC;
    SQLHSTMT comando = SQL_NULL_HSTMT;

    SQLRETURN resultado;

    char confirmacao;

    printf("\n==============================================\n");
    printf("             EXCLUIR PRODUTO\n");
    printf("==============================================\n");

    /* =====================================================
       1. CONEXAO
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

    resultado = SQLSetEnvAttr(
        ambiente,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao configurar ODBC.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    resultado = SQLAllocHandle(
        SQL_HANDLE_DBC,
        ambiente,
        &conexao
    );

    if (!SQL_SUCCEEDED(resultado)) {
        printf("\nErro ao criar conexao.\n");
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);
        return;
    }

    SQLCHAR stringConexao[] =
        "DRIVER={ODBC Driver 18 for SQL Server};"
        "SERVER=localhost;"
        "DATABASE=RADU_Distribuidora;"
        "Trusted_Connection=yes;"
        "TrustServerCertificate=yes;";

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

        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       2. CRIAR STATEMENT
       ===================================================== */

    resultado = SQLAllocHandle(
        SQL_HANDLE_STMT,
        conexao,
        &comando
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao criar comando SQL.\n");

        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       3. VERIFICAR SE EXISTE
       ===================================================== */

    SQLCHAR consultaExiste[] =
        "SELECT COUNT(*) "
        "FROM dbo.PRODUTO "
        "WHERE Cod_Produto = ?;";

    resultado = SQLPrepare(
        comando,
        consultaExiste,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao preparar verificacao.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLLEN tamanhoCodigo = sizeof(codigo);

    resultado = SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_SLONG,
        SQL_INTEGER,
        0,
        0,
        &codigo,
        sizeof(codigo),
        &tamanhoCodigo
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao associar codigo.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLExecute(comando);

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao verificar produto.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    SQLINTEGER quantidadeProduto = 0;
    SQLLEN indicador;

    if (SQLFetch(comando) == SQL_SUCCESS) {

        SQLGetData(
            comando,
            1,
            SQL_C_SLONG,
            &quantidadeProduto,
            sizeof(quantidadeProduto),
            &indicador
        );
    }

    if (quantidadeProduto == 0) {

        printf(
            "\nProduto com codigo %d nao encontrado.\n",
            codigo
        );

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       4. CONFIRMACAO
       ===================================================== */

    printf(
        "\nTem certeza que deseja excluir o produto %d? (S/N): ",
        codigo
    );

    fflush(stdout);
    scanf(" %c", &confirmacao);

    if (confirmacao != 'S' &&
        confirmacao != 's') {

        printf("\nExclusao cancelada.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       5. LIMPAR COMANDO
       ===================================================== */

    SQLFreeStmt(
        comando,
        SQL_CLOSE
    );

    /* =====================================================
       6. DELETE
       ===================================================== */

    SQLCHAR consultaDelete[] =
        "DELETE FROM dbo.PRODUTO "
        "WHERE Cod_Produto = ?;";

    resultado = SQLPrepare(
        comando,
        consultaDelete,
        SQL_NTS
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao preparar exclusao.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    resultado = SQLBindParameter(
        comando,
        1,
        SQL_PARAM_INPUT,
        SQL_C_SLONG,
        SQL_INTEGER,
        0,
        0,
        &codigo,
        sizeof(codigo),
        &tamanhoCodigo
    );

    if (!SQL_SUCCEEDED(resultado)) {

        printf("\nErro ao associar codigo para exclusao.\n");

        SQLFreeHandle(SQL_HANDLE_STMT, comando);
        SQLDisconnect(conexao);
        SQLFreeHandle(SQL_HANDLE_DBC, conexao);
        SQLFreeHandle(SQL_HANDLE_ENV, ambiente);

        return;
    }

    /* =====================================================
       7. EXECUTAR DELETE
       ===================================================== */

    resultado = SQLExecute(comando);

    if (SQL_SUCCEEDED(resultado)) {

        printf("\n==============================================\n");
        printf("       PRODUTO EXCLUIDO COM SUCESSO!\n");
        printf("==============================================\n");

    } else {

        printf("\nErro ao excluir produto.\n");
        printf("Verifique se o produto possui registros relacionados.\n");
    }

    /* =====================================================
       8. LIBERAR
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