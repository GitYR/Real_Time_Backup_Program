#include "./ssu_backup.h"

void backup_insert(Node * node,List * newList) // 무조건 맨 뒤에 추가 (뒤로 갈수록 최신)
{
	newList -> next = NULL;

	if(backup_empty(node))
		node -> head = newList;
	else
	{
		List * toSearch = node -> head;

		while(toSearch-> next != NULL)
			toSearch = toSearch -> next;

		toSearch -> next = newList;
	}
}

bool backup_delete(Node * node) // 무조건 맨 앞 리스트만 지움
{
	List * delList = node -> head;

	if(backup_empty(node))
	{
		fprintf(stderr,"list is empty\n");
		return 0;
	}

	node -> head = node -> head -> next;

	if(remove(delList->filename) < 0)
	{
		fprintf(stderr,"remove() error\n");
		return 0;
	}

	free(delList);
	return 1;
}

List * backup_search(Node * node,char * filename)
{
	List * toSearch = node -> head;

	if(backup_empty(node))
		return NULL;

	while(toSearch)
	{
		if(!strcmp(toSearch->filename,filename))
			return toSearch;
		toSearch = toSearch -> next;
	}
	return NULL;
}

void backup_delete_random(Node * node,List * list)
{
	List * delList;
	List * prev = NULL;
	List * cur = node -> head;

	if(backup_empty(node))
		return;

	while(cur)
	{
		if(cur == list)
		{
			if(cur == node -> head)
			{
				node -> head = node -> head -> next;
				delList = cur;
				free(delList);
				return;
			}
			else
			{
				delList = cur;
				cur = cur -> next;
				prev -> next = cur;
				free(delList);
				return;
			}
		}
		prev = cur;
		cur = cur -> next;
	}
}

bool backup_empty(Node * node) // 백업 리스트 비었는지 확인
{
	if(node -> head == NULL)
		return TRUE;
	else
		return FALSE;
}

int backup_count(Node * node) // 백업 리스트 개수
{
	int count;
	List * cur = node -> head;

	if(backup_empty(node))
		return 0;
	else
		count = 0;

	while(cur != NULL)
	{
		count++;
		cur = cur -> next;
	}

	return count;
}

bool backup_print(Node * node) // for testing
{
	int count = 1;
	List * print = node -> head;

	if(backup_empty(node))
		return FALSE;

	while(print)
	{
		printf("%d : %s\t%dbytes\n", count, print->timestr, print->size);
		print = print -> next;
		count++;
	}
	return TRUE;
}

void backup_time(Node * node)
{
	List * del, * prev;
	List * ps = node -> head;

	if(backup_empty(node))
		return;
	else
	{
		prev = NULL;

		while(ps)
		{
			del = ps;
			ps->sec += node->period;
			
			if(ps->sec > node->ttime && node->option.t)
			{
				if(ps == node -> head)
				{
					ps = ps -> next;
					backup_delete(node);
					prev = NULL;
				}
				else // ps != node -> head
				{
					prev -> next = ps -> next;
					ps = ps -> next;

					if(remove(del->filename) < 0)
						fprintf(stderr,"remove() error\n");
					free(del);
				}
				continue;
			}
			prev = ps;
			ps = ps -> next;
		}
	}
}

void backup_to_select(Node * node,Select * select,int count)
{
	int i;
	List * temp = node -> head;

	if(backup_empty(node))
		return;

	for(i = 0; i < count; i++)
	{
		if(temp)
		{
			select[i].number = i+1;
			strcpy(select[i].filename,temp->filename);
		}
		temp = temp -> next;
	}
}
