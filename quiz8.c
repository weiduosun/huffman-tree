#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "quiz8.h"

#define DEBUG 0

FreqTable *createFreqTable(char *string) {
  FreqTable* table = (FreqTable*)malloc(sizeof(FreqTable));
  for (int i = 0; i < NUM_ASCII_CHARS; i++) {
    table->charCount[i] = 0;
  }
  for (int i = 0; i < strlen(string); i++) {
    table->charCount[string[i]]++;
  }
  return table;
}

void destroyFreqTable(FreqTable *table) {
  free(table);
}

void printFreqTable(FreqTable *table) {
  for (int i = 0; i < NUM_ASCII_CHARS; i++) {
    if (table->charCount[i] != 0) {
      printf("%c: %d\n", i, table->charCount[i]);
    }
  }
}

Deck* createDeck() {
  Deck* newStack = (Deck*)malloc(sizeof(Deck));
  newStack->topTree = -1;
  return newStack;
}

Deck* pushTreeToDeck(HTree* tree, Deck* deck) {
  if (deck->topTree == MAX_MESSAGE_LENGTH - 1) return deck;
  deck->topTree++;
  deck->trees[deck->topTree] = tree;
  return deck;
}

HTree* peekAtTopTree(Deck* deck) {
  if (deck->topTree == -1) return NULL;
  return deck->trees[deck->topTree];
}

HTree* popTreeFromDeck(Deck* deck) {
  if (deck->topTree == -1) return NULL;
  return deck->trees[deck->topTree--];
}

int isDeckEmpty(Deck* deck) {
  if (deck->topTree == -1) return 1;
  return 0;
}

HTree* createHtree(FreqTable *table, int i) {
  HTree* tree = (HTree*)malloc(sizeof(HTree));
      tree->c = i;
      tree->freq = table->charCount[i];
      tree->p0 = NULL;
      tree->p1 = NULL;
  return tree;
}

HTree* createEncodingTree(FreqTable *table) {
  Deck* deck = createDeck();
  for (int i = 0; i < NUM_ASCII_CHARS; i++) {
    if (table->charCount[i] != 0) {
      pushTreeToDeck(createHtree(table, i), deck);
    }
  }
  while (deck->topTree != 0) {
    for (int i = 0; i < deck->topTree; i++) {
  	  for (int j = i+1; j <= deck->topTree; j++) {
  		if (deck->trees[i]->freq < deck->trees[j]->freq) {
  			HTree* temp = deck->trees[i];
  			deck->trees[i] = deck->trees[j];
  			deck->trees[j] = temp;
  		}
  	  }
    }
    HTree* tree = (HTree*)malloc(sizeof(HTree));
    tree->c = 0;
    tree->p0 = popTreeFromDeck(deck);
    tree->p1 = popTreeFromDeck(deck);
    tree->freq = tree->p0->freq + tree->p1->freq;
    pushTreeToDeck(tree, deck);
  }   
  return deck->trees[0];
}

void destroyDeck(Deck* deck) {
  free(deck); 
}

void destroyHTree(HTree *tree) {
  if (tree == NULL) free(tree);
  if (tree->p0 == NULL && tree->p1 == NULL) {
    free(tree);
  }
  if (tree->p0 != NULL) {
    destroyHTree(tree->p0);
  }
  if (tree->p1 != NULL) {
    destroyHTree(tree->p1);
  } 
}

void printHTree(HTree *tree) {
  if (tree->p0 != NULL) {
    printHTree(tree->p0);
  }
  if (tree->p1 != NULL) {
    printHTree(tree->p1);
  }
  if (tree != NULL) {
    printf("%c\t%d\n", tree->c, tree->freq);
  }
}

BitSeq* createBitSeq() {
  BitSeq* bitSeq = (BitSeq*)malloc(sizeof(BitSeq));
  bitSeq->numBitsInSeq = 0;
  for (int i = 0; i < MAX_BIT_SEQ_LENGTH; i++) {
    bitSeq->bits[i] = 0;
  }
  return bitSeq;
}

void printBitSeq(BitSeq *bitSeq) {
  for (int i = 0; i <= bitSeq->numBitsInSeq/16; i++) {
    displayBits(bitSeq->bits[i], 16);
  }
}

void destroyBitSeq(BitSeq* bitSeq) {
  free(bitSeq);
}

void packBits(BitSeq *bitSeq, char* newBitsAsChars) {
  unsigned short mask;
  for (int i = 0; i < strlen(newBitsAsChars); i++) {
    int offset = bitSeq->numBitsInSeq % 16;
    int current = bitSeq->numBitsInSeq / 16;
    mask = 0b1000000000000000;
    if (newBitsAsChars[i] == '1') {
      bitSeq->bits[current] ^= (mask>>offset);
    }
    bitSeq->numBitsInSeq++;
  }
}

void displayBits(unsigned short value, int numBits) {
  int c;
  unsigned displayMask = 1 << (numBits - 1) % 16;
  for (c = 1; c <= numBits; c++) {
    putchar(value & displayMask ? '1' : '0');
    value <<= 1;
    if (c % 16 == 0) {
      putchar(' ');
    }
  }
  putchar('\n');
}

char* code[NUM_ASCII_CHARS];

char s[NUM_ASCII_CHARS];

void printCode() {
  for (int i = 0; i < NUM_ASCII_CHARS; i++) {
    if (code[i] != NULL) printf("%c\t%s\n", i, code[i]);
  }
}

void createCode(HTree *tree, int len){
  if (tree != NULL){
    if(tree->p0 == NULL && tree->p1 == NULL){
      char *temp = (char*)malloc(sizeof(char));
      for(int i = 0; i < len; i++){
	temp[i] = s[i];
      }
      code[tree->c] = temp;
    }
    else {      
      s[len] = '0';
      createCode(tree->p0, len + 1);
      s[len] = '1';
      createCode(tree->p1, len + 1);
    }
  }
}

HTree *encode(char *s, BitSeq *bitSeq) {
  FreqTable *table = createFreqTable(s);
  HTree *tree = createEncodingTree(table);
  createCode(tree, 0);
  for (int i = 0; s[i] != 0 ; i++) {
      packBits(bitSeq, code[s[i]]);
  }
  printCode();
  return tree;
}

char *decode(BitSeq *bitSeq, HTree *tree) {
  unsigned displayMask = 1 << 15;
  HTree *temp = tree;
  char* s = (char*)malloc(tree->freq * sizeof(char));
  int len = 0;
  int count = 0;
  for (int i = 0; i <= bitSeq->numBitsInSeq; i++) {
    if (count % 16 == 0) displayMask = 1 << 15;
    if (temp->p0 == NULL && temp->p1 == NULL) {
      s[len++] = temp->c;
      temp = tree;
    }
    if (bitSeq->bits[i/16] & displayMask) {
      temp = temp->p1;
    }
    else {
      temp = temp->p0;
    }
    displayMask = displayMask >> 1;
    count++;
  }
  return s;
}



