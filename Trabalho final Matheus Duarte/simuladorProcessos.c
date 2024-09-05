//FEITO POR MATHEUS DUARTE E EDUARDO MULLER.
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define MAX_PROCESSOS 100
typedef struct  //CRIO O TIPO PROCESSO.
{
    int pid;
    int tempoDeChegada;
    int burst;
    int prioridade;
    int tempoRestante;
    int tempoDeEspera;
} processo;

int lerArquivo(const char *nomeDoArquivo, processo processos[], int *numeroDeProcessos);

processo* obterProcessoDeMaiorPrioridade(processo processos[], int numeroDeProcessos, int tempoAtual);

void escalonarProcessos(processo processos[], int numeroDeProcessos);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: %s <nome_do_arquivo>\n", argv[0]);
        return 1;
    }

    setlocale(LC_ALL, "Portuguese");

    processo processos[MAX_PROCESSOS];
    int numeroDeProcessos;

    if (lerArquivo(argv[1], processos, &numeroDeProcessos) != 0)
    {
        return 1;
    }

    printf("Número de processos lidos: %d\n", numeroDeProcessos);
    for (int i = 0; i < numeroDeProcessos; i++)
    {
        printf("PID: %d, Tempo de chegada: %d, Burst: %d, Prioridade: %d\n",
               processos[i].pid, processos[i].tempoDeChegada, processos[i].burst, processos[i].prioridade);
    }

    escalonarProcessos(processos, numeroDeProcessos);

    return 0;
}

//FUNÇÃO QUE LE O NOME DO ARQUIVO E GUARDA SUAS INFORMAÇÕES EM UM VETOR DO TIPO PROCESSO.
int lerArquivo(const char *nomeDoArquivo, processo processos[], int *numeroDeProcessos)
{
    FILE *arquivo = fopen(nomeDoArquivo, "r");
    if (arquivo == NULL)  //TESTO SE O ARQUIVO FOI LIDO CORRETAMENTE.
    {
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    *numeroDeProcessos = 0;//VOU USAR ESSA VARIAVEL DURANTE TODO O PROGRAMA PARA SABER QUANTOS PROCESSOS FORAM LIDOS DO ARQUIVO.
    while (fscanf(arquivo, "%d, %d, %d, %d",
                  &processos[*numeroDeProcessos].pid,
                  &processos[*numeroDeProcessos].tempoDeChegada,
                  &processos[*numeroDeProcessos].burst,
                  &processos[*numeroDeProcessos].prioridade) != EOF)//FAÇO ESSE WHILE COLOCANDO EOF (END OF THE FILE) COMO PARAMETRO.
    {
        processos[*numeroDeProcessos].tempoRestante = processos[*numeroDeProcessos].burst;//VOU USAR ESSE DADO PARA SABER QUANTO TEMPO AINDA UM PROCESSO PRECISA
        processos[*numeroDeProcessos].tempoDeEspera = 0;// VOU USAR ESSA PARA IR SOMANDO A CADA PASSAR DE TEMPO
        (*numeroDeProcessos)++;// A CADA LOOP DO WHILE SOMO UM AO NUMERO DE PROCESSOS LIDOS.
    }

    fclose(arquivo);
    return 0;
}
//FUNÇÃO QUE RECEBE AS INFORMAÇÕES DA FUNÇÃO ESCALONADOR, USADA PARA OBTER QUAL PROGRAMA TEM A PRIORIDADE DE EXECUÇÃO NO TEMPO DE ESCOLHA.
processo* obterProcessoDeMaiorPrioridade(processo processos[], int numeroDeProcessos, int tempoAtual)
{
    processo *escolhido = NULL;
    for (int i = 0; i < numeroDeProcessos; i++)
    {
        if (processos[i].tempoDeChegada <= tempoAtual && processos[i].tempoRestante > 0)
        {
            if (escolhido == NULL || processos[i].prioridade < escolhido->prioridade ||
                    (processos[i].prioridade == escolhido->prioridade && processos[i].pid < escolhido->pid))
            {
                escolhido = &processos[i];
            }
        }
    }
    return escolhido;
}

void escalonarProcessos(processo processos[], int numeroDeProcessos)
{
    int tempoAtual = 0;
    int finalizados = 0;
    processo *processoAtual = NULL;
    processo *processoAnterior = NULL;

    while (finalizados < numeroDeProcessos)  //LOOP ATÉ TODOS OS PROCESSOS TEREM SIDO EXECUTADOS
    {
        processoAnterior = processoAtual;//DEFINO O ANTERIOR SEMPRE COMO PROCESSO ATUAL PARA PODER FAZER A TROCA DEPOIS.
        processoAtual = obterProcessoDeMaiorPrioridade(processos, numeroDeProcessos, tempoAtual);// ATUAL RECEBE O PROCESSO RETORNADO POR OBTER PRIORIDADE.

        if (processoAtual != NULL)  //SE NÃO FOR NULO
        {
            if (processoAtual != processoAnterior)  //SE A ESCOLHA DO PROCESSO NESSE TEMPO É DIFERENTE DA ESCOLHA DO TEMPO ANTERIOR
            {
                if (processoAnterior != NULL)  //SE ANTERIOR NÃO FOR NULO.
                {
                    //TROCAMOS O PROCESSO QUE ESTÁ EM EXECUÇÃO E O PAUSAMOS
                    printf("Tempo %d: Processo PID %d foi pausado - Burst: %d, Tempo restante: %d\n",
                           tempoAtual, processoAnterior->pid, processoAnterior->burst, processoAnterior->tempoRestante);
                }
                //MOSTRAMOS QUAL PROCESSO ESTÁ ENTRANDO NO LUGAR DELE.
                printf("Tempo %d: Processo PID %d entrou em execução - Tempo de chegada: %d, Burst: %d, Prioridade: %d, Tempo restante: %d\n",
                       tempoAtual, processoAtual->pid, processoAtual->tempoDeChegada, processoAtual->burst, processoAtual->prioridade, processoAtual->tempoRestante);
            }

            processoAtual->tempoRestante--;//DIMINUIO 1 NO TEMPO RESTANTE DO PROCESSO ATUAL A CADA LOOP

            if (processoAtual->tempoRestante == 0)  //SE O TEMPO RESTANTE FOR IGUAL A 0 SOMAMOS UM A VARIAVEL FINALIZADOS, E MOSTRAMOS QUE ELE ACABOU NA TELA.
            {
                finalizados++;
                printf("Tempo %d: Processo PID %d terminou a execução.\n", tempoAtual + 1, processoAtual->pid);
                processoAtual = NULL;
            }
        }


        for (int i = 0; i < numeroDeProcessos; i++)  //INCREMENTO 1 AO TEMPO DE ESPERA DE TODOS OS PROCESSOS QUE NÃO FOREM O ATUAL EM EXECUÇÃO.
        {
            if (&processos[i] != processoAtual && processos[i].tempoDeChegada <= tempoAtual && processos[i].tempoRestante > 0)
            {
                processos[i].tempoDeEspera++;
            }
        }

        tempoAtual++;
    }

    //USO ESSE LOOP PARA CALCULAR O TEMPO DE ESPERA TOTAL DOS PROCESSOS LIDOS.
    int tempoDeEsperaTotal = 0;
    for (int i = 0; i < numeroDeProcessos; i++)
    {
        tempoDeEsperaTotal += processos[i].tempoDeEspera;
    }
    double tempoDeEsperaMedio = (double)tempoDeEsperaTotal / numeroDeProcessos;//FAÇO A MEDIA E IMPRIMO ELA
    printf("Tempo de espera médio: %.2f\n", tempoDeEsperaMedio);
}

