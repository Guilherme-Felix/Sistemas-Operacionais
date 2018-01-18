/*
*  lottery.c - Implementacao do algoritmo Lottery Scheduling e sua API
*
*  Autores: Guilherme Felix - 201365504B / Leandro Dornela - 201365309AC
*  Projeto: Trabalho Pratico I - Sistemas Operacionais - 2017.1
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*
*  O algoritmo de escalonamento por loteria esta' implementado da seguinte
*  forma:
*
*  Sorteia-se um número no intervalo [0,total_tickets_validos]
*  Percorre-se a lista de processos, buscando os que estão prontos:
*     Diminui-se do numero sorteado, o numero de tickets do processo
*     Se a diferenca for menor ou igual a zero, este deve ser o processo escolhido
*  
*/

#include "lottery.h"
#include <stdio.h>
#include <string.h>
#define MESSAGE 1     // flag para mensagem de controle

//Nome unico do algoritmo. Deve ter 4 caracteres.
const char lottName[]="LOTT";
//Slot que este escalonador ficara associado apos ser registrado
int slotLocal = -1;

//=====Funcoes Auxiliares=====

void successMsgSchedInfo()
{
	printf("=============================\n");
	printf("SchedInfo criado com sucesso!\n");
	printf("=============================\n");
}

void successMsgSchedRegister(int slot)
{
	printf("------------------------------------------------------");
	printf("\n Registro do escalonador bem sucedido! Slot n: %d \n", slot);
	printf("------------------------------------------------------\n");
}

void successMsgInitSchedParams(Process *p)
{
	printf("\nParametros do processo:%d associados com sucesso!\n", processGetPid(p));
}


void successMsgReleaseParams(Process *p)
{
	printf("\n¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");
	printf("ReleaseParams do processo:%d.\n", processGetPid(p));
	printf("¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");
}

//=====Funcoes da API=====

//Funcao chamada pela inicializacao do S.O. para a incializacao do escalonador
//conforme o algoritmo Lottery Scheduling
//Deve envolver a inicializacao de possiveis parametros gerais
//Deve envolver o registro do algoritmo junto ao escalonador
void lottInitSchedInfo() {
	//Cria um ponteiro pra estrutura SchedInfo
	SchedInfo *si = malloc(sizeof(SchedInfo));

	if(MESSAGE)
	{
		successMsgSchedInfo();
	}
	// Inicializa os parametros
	strcpy(si->name, lottName);
	si->initParamsFn = lottInitSchedParams;
	si->scheduleFn = lottSchedule;
	si->releaseParamsFn = lottReleaseParams;
	// Registra o escalonador
	slotLocal = schedRegisterScheduler(si);
	
	if(MESSAGE)
	{
		if (slotLocal == 0)
		{
			successMsgSchedRegister(slotLocal);
		}
	}
}

//Inicializa os parametros de escalonamento de um processo p, chamada 
//normalmente quando o processo e' associado ao slot de Lottery
void lottInitSchedParams(Process *p, void *params) {
	int ret_val = schedSetScheduler(p, params, slotLocal);
    if(ret_val && MESSAGE)
		successMsgInitSchedParams(p);
	//...

}

//Retorna o proximo processo a obter a CPU, conforme o algortimo Lottery 
Process* lottSchedule(Process *plist) {
	Process *p = NULL;
	Process *next = NULL;
	int total_tkt_validos = 0;
	LotterySchedParams *lsp = NULL;
	int r = 0;
	
	// Percorre a lista de processos buscando os prontos para contar o total de tickets validos para sorteio.
	for (p = plist; p != NULL; p = next)
	{
		next = processGetNext(p);
		lsp = processGetSchedParams(p);
	
		if(processGetStatus(p) == PROC_READY && processGetSchedSlot(p) == slotLocal)
		{
			total_tkt_validos = total_tkt_validos + lsp->num_tickets;
		}
	}
	// Sorteia um ticket (no universo do total de tickets de processos prontos)
	r = rand() % total_tkt_validos;

	// Percorre novamente a lista em busca dos processos prontos	
	for (p = plist; p != NULL; p = next)
	{
		next = processGetNext(p);
		lsp = processGetSchedParams(p);

		if(processGetStatus(p) == PROC_READY && processGetSchedSlot(p) == slotLocal)
		{
			r = r - lsp->num_tickets;
			if(r <= 0)
			{
				return p;
			}
		}
	}
}

//Libera os parametros de escalonamento de um processo p, chamada 
//normalmente quando o processo e' desassociado do slot de Lottery
//Retorna o numero do slot ao qual o processo estava associado
int lottReleaseParams(Process *p) {
	
	if (MESSAGE)
	{
		successMsgReleaseParams(p);	
	}

	LotterySchedParams *lsp_old = processGetSchedParams(p);
	free(lsp_old);

	return slotLocal;
}

//Transfere certo numero de tickets do processo src para o processo dst.
//Retorna o numero de tickets efetivamente transfeirdos (pode ser menos)
int lottTransferTickets(Process *src, Process *dst, int tickets) {

	LotterySchedParams *lsp_src, *lsp_dst;
	int old_tkt;

	// acessa os parametros de escalonamento de ambos os processos
	lsp_src = processGetSchedParams(src);
	lsp_dst = processGetSchedParams(dst);

	// pega os tickets do processo origem	
	old_tkt = lsp_src->num_tickets;
	
	// se o processo de origem tiver menos tickets do que requisitado
	// transferem-se 90% do que foi pedido.
	if ( old_tkt < tickets )
	{
		tickets = (int)(0.9*old_tkt);
	}

	lsp_src->num_tickets = old_tkt - tickets;

	//acrescenta no numero de tickets do processo destino
	lsp_dst->num_tickets = lsp_dst->num_tickets + tickets;

	return tickets;
	//...
	//...
}
