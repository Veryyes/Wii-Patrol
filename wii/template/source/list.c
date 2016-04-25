#include <stdlib.h>
#include <stdio.h>
#include "list.h"

void append(Node** head, Node** tail, float x, float y)
{
	if(*tail==NULL)//if *tail is null, then *head should also be null...
	{
		*tail = malloc(sizeof(Node));
		if(*tail==NULL)
		{
			printf("malloc failed\n");
			exit(1);
		}
		*head = *tail;

		(*tail)->x = x;
		(*tail)->y = y;
		(*tail)->next = NULL;
		(*tail)->prev = NULL;
	}else
	{
		Node* temp = malloc(sizeof(Node));
		if(temp==NULL)
		{
			printf("malloc failed\n");
			exit(1);
		}
		temp->x = x;
		temp->y = y;
		
		(*tail)->next = temp;
		temp->prev = (*tail);
		temp->next = NULL;
		(*tail) = temp;
	}
}

void clear(Node** head, Node** tail)
{
	Node* curr = *head;
	while(curr!=NULL)
	{
		Node* temp = curr;
		curr = curr->next;
		free(temp);
	}
	*head = NULL;
	*tail = NULL;
}

void printlist(Node* head)
{
	Node* temp = head;
	while(temp!=NULL)
	{
		printf("(%f, %f)",temp->x, temp->y);
		temp = temp->next;
	}
}

