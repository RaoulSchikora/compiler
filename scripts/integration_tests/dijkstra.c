#include <stdbool.h>
#include "mc_builtins.c"
typedef const char* string;

void dijkstra(int path_cost[15])
{
	int calc_cost[15];
	int i;
	int j;
	i = 0;

	while (i < 4)
	{
		j = 0;
		while (j < 4)
		{
			if (path_cost[i * 4 + j] == 0)
				calc_cost[i * 4 + j] = 9999;
			else
				calc_cost[i * 4 + j] = path_cost[i * 4 + j];

			j = j + 1;
		}
		i = i + 1;
	}

	int distance[4];
	bool visited[4];

	int node_count;
	int next_node;
	node_count = 0;

	while (node_count < 4)
	{
		distance[node_count] = calc_cost[node_count];

		if (node_count == 0)
			visited[node_count] = true;
		else
			visited[node_count] = false;

		node_count = node_count + 1;
	}

	node_count = 1;

	while (node_count < 4)
	{
		int min_distance;
		int traversel_count;

		min_distance = 9999;
		traversel_count = 0;

		while (traversel_count < 4)
		{
			if (!visited[traversel_count] &&
				distance[traversel_count] < min_distance)
			{
				min_distance = distance[traversel_count];
				next_node = traversel_count;
			}
			traversel_count = traversel_count + 1;
		}

		visited[next_node] = true;
		traversel_count = 0;

		while (traversel_count < 4)
		{
			if (!visited[traversel_count])
			{
				if (distance[traversel_count] >
					calc_cost[next_node * 4 + traversel_count] + min_distance)
				{
					distance[traversel_count] =
						calc_cost[next_node * 4 + traversel_count] + min_distance;
				}
			}
			traversel_count = traversel_count + 1;
		}
		node_count = node_count + 1;
	}

	int print_counter;
	print_counter = 1;
	while (print_counter < 4)
	{
		print("Distance to node ");
		print_int(print_counter);
		print(" = ");
		print_int(distance[print_counter]);
		print_nl();

		print_counter = print_counter + 1;
	}
}

int main()
{
	int path_cost[15];
	int input_counter;
	int node_counter;
	input_counter = 1;
	node_counter = 1;
	path_cost[0] = 0;

	print("Enter the path costs ('0' = no path)");
	print_nl();

	while (input_counter < 15)
	{
		if (input_counter < 4)
		{
			print("Enter path cost from 0 -> ");
			print_int(node_counter);
			print(":");
			print_nl();
			path_cost[input_counter] = read_int();
		}

		if (input_counter >= 4 && input_counter < 8)
		{
			if (input_counter != 5)
			{
				print("Enter path cost from 1 -> ");
				print_int(node_counter);
				print(":");
				print_nl();
				path_cost[input_counter] = read_int();
			}
			else
			{
				path_cost[input_counter] = 0;
			}
		}

		if (input_counter >= 8 && input_counter < 12)
		{
			if (input_counter != 10)
			{
				print("Enter path cost from 2 -> ");
				print_int(node_counter);
				print(":");
				print_nl();
				path_cost[input_counter] = read_int();
			}
			else
			{
				path_cost[input_counter] = 0;
			}
		}

		if (input_counter >= 12)
		{
			if (input_counter != 15)
			{
				print("Enter path cost from 3 -> ");
				print_int(node_counter);
				print(":");
				print_nl();
				path_cost[input_counter] = read_int();
			}
			else
			{
				path_cost[input_counter] = 0;
			}
		}

		if (node_counter >= 3)
			node_counter = 0;
		else
			node_counter = node_counter + 1;

		input_counter = input_counter + 1;
	}

	dijkstra(path_cost);

	return 0;
}