#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "Search.h"
#include "Engine.h"
#include "TTable.h"

int MATERIAL_VALUES[6] = {100,300,300,500,1000,10000};
int CAPTURE_VALUES[6] = {1,3,3,6,9,1};

TTable * TABLE;

time_t START_TIME;
time_t END_TIME;


int START_DEPTH = 2;
int MAX_DEPTH = 10;
int DELTA_DEPTH = 2;
int END_DEPTH = 10;

int TOTAL_BOARDS_SEARCHED = 0;
int TOTAL_MOVES_FOUND = 0;

int getBestMoveIndex(Board * board, int turn){

	START_TIME = time(NULL);
	END_TIME = START_TIME + 9;
	
	TABLE = createTTable();
	
	int size = 0;
	int * moves = getAllMoves(board,turn,&size);
	int * moves_p = moves;
	
	size = 0;
	int * unsorted = getAllMoves(board,turn,&size);
	
	if (size == 0)
		return -1;
		
	int values[size];
	
	int depth, i;
	
	for(depth = START_DEPTH; depth < MAX_DEPTH && depth != END_DEPTH; depth += DELTA_DEPTH){
		printf("==========================================\n");
		printf("Searching Depth Level : %d\n",depth);
		
		int alpha = -MATE;
		int beta = MATE;
		
		for(i = 0; i < size; i++, moves += 5){
			int preSearched = TOTAL_BOARDS_SEARCHED;
			values[i] = -alphaBetaPrune(board,!turn,moves,depth,-beta,-alpha,turn);
			printf("#%d\tValue: %d\tSearched: %d\n",i,values[i],TOTAL_BOARDS_SEARCHED-preSearched);
			
			if (values[i] > alpha)
				alpha = values[i];
				
			if (alpha == MATE)
				return endSearch(i+1,size,values,moves_p,unsorted);
		}
		
		valueSort(values,moves_p,size);
		moves = moves_p;
	}
	
	return endSearch(i+1,size,values,moves_p,unsorted);
}

int endSearch(int index, int size, int * values, int * sorted, int * unsorted){
	printf("==========================================\n");
	printf("Total Boards Searched \t: %d\n",TOTAL_BOARDS_SEARCHED);
	printf("Total Moves Found \t: %d\n",TOTAL_MOVES_FOUND);
	return 0;
}

void valueSort(int * values, int * moves, int size){
	int i,j;
	int a[5], b[5];
	for(i = 0; i < size; i++){
		for(j = i + 1; j < size; j++){
			if (values[j] > values[i]){
				memcpy(a,moves+i*5,sizeof(int) * 5);
				memcpy(b,moves+j*5,sizeof(int) * 5);
				memcpy(moves+j*5,a,sizeof(int) * 5);
				memcpy(moves+i*5,b,sizeof(int) * 5);
				int x = values[j];
				values[i] = values[j];
				values[j] = x;
			}
		}
	}
}

int * hueristicSort(Board * board, int * moves, int size){

}

int alphaBetaPrune(Board * board, int turn, int * move, int depth, int alpha, int beta, int eval){
	
	if (END_TIME < time(NULL))
		return eval == turn ? -MATE : MATE;
	
	ApplyMove(board,move);
	
	if (depth == 0){
		int value = evaluateBoard(board, turn);
		RevertMove(board,move);
		return value;
	}
	
	int best = 0, size = 0;
	int * moves = getAllMoves(board,turn,&size);
	int * moves_p = moves;
	
	TOTAL_MOVES_FOUND += size;
	
	if (size == 0){
		best = validateMove(board,turn) == 1 ? 0 : -MATE;
	} else {
		best = -MATE - 1;
		int i;
		for(i = 0; i < size; i++, moves += 5){
			int value = -alphaBetaPrune(board,!turn,moves,depth-1,-beta,-alpha,eval);
			if (value > best)
				best = value;	
			if (best > alpha)
				alpha = best;	
			if (best >= beta)
				break;
		}
	}

	RevertMove(board,move);
	free(moves_p);
	return best;
}

int evaluateBoard(Board * board, int turn){
	TOTAL_BOARDS_SEARCHED += 1;
	return 	evaluateMaterial(board, turn) + 
			evaluatePosition(board, turn) +
			evaluateMoves(board,turn)	-
			evaluateMoves(board,!turn);
}

int evaluateMaterial(Board * board, int turn){
	int i, value = 0;
	for(i = 0; i < 64; i++){
		if (board->TYPES[i] != EMPTY){
			if (board->COLORS[i] == turn)
				value += MATERIAL_VALUES[board->TYPES[i]];
			else
				value -= MATERIAL_VALUES[board->TYPES[i]];
		}
	}
	return value;
}

int evaluatePosition(Board * board, int turn){
	int x,y, value = 0;
	for(x = 2; x < 6; x++)
		for(y = 2; y < 6; y++)
			if (board->Types[x][y] == KNIGHT)
				value += board->Colors[x][y] == turn ? VALUE_CENTRAL_KNIGHT : -VALUE_CENTRAL_KNIGHT;
	
	for(x = 3; x < 5; x++)
		for(y = 3; y < 5; y++)
			if (board->Types[x][y] != EMPTY)
				value += board->Colors[x][y] == turn ? VALUE_CENTER_ATTACKED : -VALUE_CENTER_ATTACKED;

	return value;
}

int evaluateMoves(Board * board, int turn){
	int size = 0;
	int * moves = getAllMoves(board,turn,&size);
	int * moves_p = moves;
	
	TOTAL_MOVES_FOUND += size;
	
	float nv = 0;
	int i, value=0;
	for(i = 0; i < size; i++, moves += 5){
		if (board->TYPES[moves[1]] == BISHOP)
			value += VALUE_BISHOP_RANGE;
		else if (board->TYPES[moves[1]] == KNIGHT)
			value += VALUE_KNIGHT_RANGE;
		if (moves[2] / 8 > 2 && moves[2] / 8 < 5 && moves[2] % 8 > 2 && moves[2] % 8 < 5)
			value += VALUE_CENTER_ATTACKED;
		if (moves[0] == 0 && moves[3] != 9)
			nv += CAPTURE_VALUES[board->TYPES[moves[2]]] / CAPTURE_VALUES[board->TYPES[moves[1]]];	
	}
	
	free(moves_p);
	return value + (int)(nv/2);
}
