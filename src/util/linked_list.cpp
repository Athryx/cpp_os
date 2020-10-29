#include <util/linked_list.hpp>
#include <types.hpp>
#include <util/io.hpp>


using namespace util;


linked_list::linked_list ()
: len (0),
start (NULL),
end (NULL),
current (NULL),
current_i (0)
{
}

bool linked_list::prepend (list_node *zone)
{
	len ++;
	current_i ++;

	if (len == 1)
	{
		start = zone;
		end = zone;
		return true;
	}

	start->prev = zone;
	zone->next = start;
	start = zone;
	return true;
}

bool linked_list::append (list_node *zone)
{
	len ++;

	if (len == 1)
	{
		start = zone;
		end = zone;
		return true;
	}

	end->next = zone;
	zone->prev = end;
	end = zone;
	return true;
}

bool linked_list::insert (usize index, list_node *zone)
{
	if (index > len)
		return false;

	if (index == len)
		return append (zone);

	if (index == 0)
		return prepend (zone);

	if (index <= current_i)
		current_i ++;

	len ++;
	list_node *temp = (*this)[index];

	temp->prev->next = zone;
	zone->prev = temp->prev;
	temp->prev = zone;
	zone->next = temp;

	return true;
}

list_node *linked_list::pop_start ()
{
	if (len == 0)
		return NULL;

	len --;

	if (current == start)
		current = NULL;

	if (len == 0)
		return start;

	list_node *out = start;
	start = start->next;
	return out;
}

list_node *linked_list::pop ()
{
	if (len == 0)
		return NULL;

	len --;

	if (current == end)
		current = NULL;

	if (len == 0)
		return end;

	list_node *out = end;
	end = end->prev;
	return out;
}

list_node *linked_list::remove (usize index)
{
	if (index > len)
		return NULL;

	if (index == len - 1)
		return pop ();

	if (index == 0)
		return pop_start ();


	len --;
	list_node *temp = (*this)[index];

	// ths node is not on the ends, so all pointers are valid
	if (index <= current_i)
		current = temp->next;


	temp->prev->next = temp->next;
	temp->next->prev = temp->prev;

	return temp;
}

void linked_list::remove_p (list_node *zone)
{
	if (len == 0)
		return;

	len --;

	if (current == zone)
		current = NULL;

	if (len == 0)
		return;

	if (zone == start)
	{
		start = zone->next;
		return;
	}

	if (zone == end)
	{
		end = zone->prev;
		return;
	}

	zone->prev->next = zone->next;
	zone->next->prev = zone->prev;
}

list_node *linked_list::operator[] (usize index)
{
	if (index >= len)
		return NULL;

	if (current == NULL)
	{
		current = start;
		current_i = 0;
	}

	isize diff_start = index;
	isize diff_end = len - index - 1;
	isize diff_middle = index - current_i;

	list_node *zone;
	usize i;
	i8 dir;

	if (diff_middle < 0 && diff_start < abs (diff_middle))
	{
		zone = start;
		i = 0;
		dir = 1;
	}
	else if (diff_end < diff_middle)
	{
		zone = end;
		i = len - 1;
		dir = -1;
	}
	else
	{
		zone = current;
		i = current_i;
		dir = sign (diff_middle);
	}

	for (; i != index; i += dir)
	{
		if (dir == 1)
			zone = zone->next;
		else
			zone = zone->prev;

		if (zone == NULL)
		{
			error("zone should not be null")
			return NULL;
		}
	}

	current = zone;
	current_i = index;

	return zone;
}
