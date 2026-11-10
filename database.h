#ifndef DATABASE_H
#define DATABASE_H

#include <sql.h>
#include <sqlext.h>

int conectarBanco();

/*
    Funcoes auxiliares usadas pelos modulos novos (estoque, vendas e
    relatorios). Nao alteram o comportamento de conectarBanco() nem do
    CRUD de produtos.
*/

int abrirConexao(SQLHENV *ambiente, SQLHDBC *conexao);
void fecharConexao(SQLHENV ambiente, SQLHDBC conexao);
void imprimirErroODBC(SQLSMALLINT tipoHandle, SQLHANDLE handle);

#endif
