#include "./ssu_backup.h"

void list_init(void)
{
	head = NULL;
}

void list_insert(Node * newNode)
{
	newNode->next = NULL;

	if(list_empty())
		head = newNode; // 시작점 
	else
	{
		Node * toSearch;
		toSearch = head;

		while(toSearch->next != NULL)
			toSearch = toSearch -> next;

		toSearch -> next = newNode;
	}
}

bool list_delete(char * filename)
{
	Node * delNode;
	Node * prev = NULL;
	Node * toSearch = head;

	if(list_empty())
	{
		fprintf(stderr,"list is empty\n");
		return 0;
	}

	while(toSearch != NULL)
	{
		if(!strcmp(toSearch->filename,filename)) // 발견
		{
			delNode = toSearch;
		
			if(toSearch != head) // 삭제할 Node가 head가 아님
			{
				toSearch = toSearch -> next;
				prev -> next = toSearch;
			}
			else // head
			{
				head = head -> next;
			}

			free(delNode);
			return 1;
		}
		prev = toSearch;
		toSearch = toSearch -> next;
	}
	return 0;
}

Node * list_search(char * filename)
{
	Node * toSearch = head;

	if(list_empty())
	{
		fprintf(stderr,"list is empty\n");
		return NULL;
	}

	while(toSearch != NULL)
	{
		if(!strcmp(toSearch->filename,filename))
			return toSearch;
		toSearch = toSearch -> next;
	}
	return NULL;
}

bool list_compare(char * filename)
{
	Node * toSearch = head;

	if(list_empty())
		return TRUE;
	else
	{
		while(toSearch)
		{
			if(!strcmp(toSearch->filename,filename))
				return FALSE;
			toSearch = toSearch -> next;
		}
	}
	return TRUE;
}

void list_print(void)
{
	int i = 1;
	Node * print;
	print = head;

	if(list_empty())
	{
		fprintf(stderr,"list is empty!\n");
		return;
	}

	while(print)
	{
		printf("filename : %s\tperiod : %d, ", print->filename, print->period);
		if(print->option.m)
			printf(",M option : true ");
		else
			printf(",M option : false ");

		if(print->option.n)
			printf(",N option : true ");
		else
			printf(",N option : false ");

		if(print->option.t)
			printf(",T option : true ");
		else
			printf(",T option : false ");

		if(print->option.d)
			printf(",D option : true\n");
		else
			printf(",D option : false\n");

		print = print -> next;
	}
}

bool list_empty(void)
{
	if(head == NULL)
		return TRUE;
	else
		return FALSE;
}
