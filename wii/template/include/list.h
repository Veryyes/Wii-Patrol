#ifndef LIST_H
#define LIST_H
typedef struct node
{
	float x;
	float y;
	struct node* next;
	struct node* prev;
}Node;

void append(Node**, Node**, float, float);
void clear(Node**, Node**);
void printlist(Node*);
#endif
