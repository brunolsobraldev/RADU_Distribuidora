/*
    RADU Distribuidora
    Script de estrutura para os modulos de ESTOQUE e VENDAS.

    IMPORTANTE:
    - Este script NAO altera dbo.PRODUTO nem dbo.FORNECEDOR.
    - Nao usa DROP TABLE, TRUNCATE TABLE ou DELETE.
    - Cria somente o que ainda nao existe.
    - As tabelas LOTE_ESTOQUE, MOVIMENTACAO_ESTOQUE, CLIENTE,
      PEDIDO e ITENS_PEDIDO ja existem no banco e sao apenas
      verificadas, pois o codigo foi adaptado ao schema real.
*/

USE RADU_Distribuidora;
GO

SET NOCOUNT ON;
GO

/* ============================================================
   ESTOQUE (unica tabela realmente ausente)
   ============================================================ */

IF OBJECT_ID(N'dbo.ESTOQUE', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.ESTOQUE
    (
        Cod_Estoque INT IDENTITY(1,1) NOT NULL,
        Cod_Produto INT NOT NULL,
        Quantidade INT NOT NULL
            CONSTRAINT DF_ESTOQUE_QUANTIDADE DEFAULT (0),

        CONSTRAINT PK_ESTOQUE
            PRIMARY KEY CLUSTERED (Cod_Estoque),

        CONSTRAINT UQ_ESTOQUE_PRODUTO
            UNIQUE (Cod_Produto),

        CONSTRAINT FK_ESTOQUE_PRODUTO
            FOREIGN KEY (Cod_Produto)
            REFERENCES dbo.PRODUTO (Cod_Produto),

        CONSTRAINT CK_ESTOQUE_QUANTIDADE
            CHECK (Quantidade >= 0)
    );

    PRINT 'Tabela dbo.ESTOQUE criada.';
END
ELSE
BEGIN
    PRINT 'Tabela dbo.ESTOQUE ja existe. Nada a criar.';
END;
GO

/* ============================================================
   INDICE PARA CONSULTA DE LOTES POR PRODUTO E VALIDADE
   ============================================================ */

IF NOT EXISTS
(
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_LOTE_ESTOQUE_PRODUTO_VALIDADE'
      AND object_id = OBJECT_ID(N'dbo.LOTE_ESTOQUE')
)
BEGIN
    CREATE INDEX IX_LOTE_ESTOQUE_PRODUTO_VALIDADE
        ON dbo.LOTE_ESTOQUE (FK_Cod_Produto, Data_Validade);

    PRINT 'Indice IX_LOTE_ESTOQUE_PRODUTO_VALIDADE criado.';
END
ELSE
BEGIN
    PRINT 'Indice IX_LOTE_ESTOQUE_PRODUTO_VALIDADE ja existe.';
END;
GO

/* ============================================================
   VERIFICACOES DAS TABELAS EXISTENTES
   O codigo depende destas colunas. Se alguma faltar, o script
   para com erro em vez de deixar o sistema falhar em runtime.
   ============================================================ */

IF COL_LENGTH(N'dbo.PRODUTO', N'Cod_Produto') IS NULL
BEGIN
    ;THROW 50001, 'dbo.PRODUTO nao possui a coluna Cod_Produto.', 1;
END;

IF COL_LENGTH(N'dbo.PRODUTO', N'Nome') IS NULL
BEGIN
    ;THROW 50002, 'dbo.PRODUTO nao possui a coluna Nome.', 1;
END;

IF COL_LENGTH(N'dbo.PRODUTO', N'Preco_Venda_Unidade') IS NULL
BEGIN
    ;THROW 50003, 'dbo.PRODUTO nao possui a coluna Preco_Venda_Unidade.', 1;
END;

IF COL_LENGTH(N'dbo.LOTE_ESTOQUE', N'FK_Cod_Produto') IS NULL
BEGIN
    ;THROW 50004, 'dbo.LOTE_ESTOQUE nao possui a coluna FK_Cod_Produto.', 1;
END;

IF COL_LENGTH(N'dbo.LOTE_ESTOQUE', N'Data_Recebimento') IS NULL
BEGIN
    ;THROW 50005, 'dbo.LOTE_ESTOQUE nao possui a coluna Data_Recebimento.', 1;
END;

IF COL_LENGTH(N'dbo.LOTE_ESTOQUE', N'Data_Validade') IS NULL
BEGIN
    ;THROW 50006, 'dbo.LOTE_ESTOQUE nao possui a coluna Data_Validade.', 1;
END;

IF COL_LENGTH(N'dbo.LOTE_ESTOQUE', N'Quantidade_Recebida') IS NULL
BEGIN
    ;THROW 50007, 'dbo.LOTE_ESTOQUE nao possui a coluna Quantidade_Recebida.', 1;
END;

IF COL_LENGTH(N'dbo.LOTE_ESTOQUE', N'Saldo_Disponivel') IS NULL
BEGIN
    ;THROW 50008, 'dbo.LOTE_ESTOQUE nao possui a coluna Saldo_Disponivel.', 1;
END;

IF COL_LENGTH(N'dbo.MOVIMENTACAO_ESTOQUE', N'FK_Cod_Produto') IS NULL
BEGIN
    ;THROW 50009, 'dbo.MOVIMENTACAO_ESTOQUE nao possui a coluna FK_Cod_Produto.', 1;
END;

IF COL_LENGTH(N'dbo.MOVIMENTACAO_ESTOQUE', N'FK_Id_Lote') IS NULL
BEGIN
    ;THROW 50010, 'dbo.MOVIMENTACAO_ESTOQUE nao possui a coluna FK_Id_Lote.', 1;
END;

IF COL_LENGTH(N'dbo.MOVIMENTACAO_ESTOQUE', N'Tipo_Movimento') IS NULL
BEGIN
    ;THROW 50011, 'dbo.MOVIMENTACAO_ESTOQUE nao possui a coluna Tipo_Movimento.', 1;
END;

IF COL_LENGTH(N'dbo.MOVIMENTACAO_ESTOQUE', N'Quantidade') IS NULL
BEGIN
    ;THROW 50012, 'dbo.MOVIMENTACAO_ESTOQUE nao possui a coluna Quantidade.', 1;
END;

IF COL_LENGTH(N'dbo.MOVIMENTACAO_ESTOQUE', N'Data_Movimento') IS NULL
BEGIN
    ;THROW 50013, 'dbo.MOVIMENTACAO_ESTOQUE nao possui a coluna Data_Movimento.', 1;
END;

IF COL_LENGTH(N'dbo.MOVIMENTACAO_ESTOQUE', N'Observacao') IS NULL
BEGIN
    ;THROW 50014, 'dbo.MOVIMENTACAO_ESTOQUE nao possui a coluna Observacao.', 1;
END;

IF COL_LENGTH(N'dbo.CLIENTE', N'Id_Cliente') IS NULL
BEGIN
    ;THROW 50015, 'dbo.CLIENTE nao possui a coluna Id_Cliente.', 1;
END;

IF COL_LENGTH(N'dbo.CLIENTE', N'Nome_Razao_Social') IS NULL
BEGIN
    ;THROW 50016, 'dbo.CLIENTE nao possui a coluna Nome_Razao_Social.', 1;
END;

IF COL_LENGTH(N'dbo.CLIENTE', N'CPF_CNPJ') IS NULL
BEGIN
    ;THROW 50017, 'dbo.CLIENTE nao possui a coluna CPF_CNPJ.', 1;
END;

IF COL_LENGTH(N'dbo.CLIENTE', N'Telefone') IS NULL
BEGIN
    ;THROW 50018, 'dbo.CLIENTE nao possui a coluna Telefone.', 1;
END;

IF COL_LENGTH(N'dbo.CLIENTE', N'Email') IS NULL
BEGIN
    ;THROW 50019, 'dbo.CLIENTE nao possui a coluna Email.', 1;
END;

IF COL_LENGTH(N'dbo.CLIENTE', N'Endereco_Entrega') IS NULL
BEGIN
    ;THROW 50020, 'dbo.CLIENTE nao possui a coluna Endereco_Entrega.', 1;
END;

IF COL_LENGTH(N'dbo.PEDIDO', N'Num_Pedido') IS NULL
BEGIN
    ;THROW 50021, 'dbo.PEDIDO nao possui a coluna Num_Pedido.', 1;
END;

IF COL_LENGTH(N'dbo.PEDIDO', N'Data_Pedido') IS NULL
BEGIN
    ;THROW 50022, 'dbo.PEDIDO nao possui a coluna Data_Pedido.', 1;
END;

IF COL_LENGTH(N'dbo.PEDIDO', N'Status_Pedido') IS NULL
BEGIN
    ;THROW 50023, 'dbo.PEDIDO nao possui a coluna Status_Pedido.', 1;
END;

IF COL_LENGTH(N'dbo.PEDIDO', N'Valor_Total') IS NULL
BEGIN
    ;THROW 50024, 'dbo.PEDIDO nao possui a coluna Valor_Total.', 1;
END;

IF COL_LENGTH(N'dbo.PEDIDO', N'FK_Id_Cliente') IS NULL
BEGIN
    ;THROW 50025, 'dbo.PEDIDO nao possui a coluna FK_Id_Cliente.', 1;
END;

IF COL_LENGTH(N'dbo.ITENS_PEDIDO', N'FK_Num_Pedido') IS NULL
BEGIN
    ;THROW 50026, 'dbo.ITENS_PEDIDO nao possui a coluna FK_Num_Pedido.', 1;
END;

IF COL_LENGTH(N'dbo.ITENS_PEDIDO', N'FK_Cod_Produto') IS NULL
BEGIN
    ;THROW 50027, 'dbo.ITENS_PEDIDO nao possui a coluna FK_Cod_Produto.', 1;
END;

IF COL_LENGTH(N'dbo.ITENS_PEDIDO', N'FK_Id_Lote') IS NULL
BEGIN
    ;THROW 50028, 'dbo.ITENS_PEDIDO nao possui a coluna FK_Id_Lote.', 1;
END;

IF COL_LENGTH(N'dbo.ITENS_PEDIDO', N'Qtd_Vendida') IS NULL
BEGIN
    ;THROW 50029, 'dbo.ITENS_PEDIDO nao possui a coluna Qtd_Vendida.', 1;
END;

IF COL_LENGTH(N'dbo.ITENS_PEDIDO', N'Tipo_Venda') IS NULL
BEGIN
    ;THROW 50030, 'dbo.ITENS_PEDIDO nao possui a coluna Tipo_Venda.', 1;
END;

IF COL_LENGTH(N'dbo.ITENS_PEDIDO', N'Preco_Praticado') IS NULL
BEGIN
    ;THROW 50031, 'dbo.ITENS_PEDIDO nao possui a coluna Preco_Praticado.', 1;
END;

PRINT 'Verificacoes de estrutura concluidas com sucesso.';
GO
