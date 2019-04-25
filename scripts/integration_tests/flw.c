#include "mc_builtins.c"
#include <stdbool.h>
typedef const char *string;
void print_2_signals(float signal1[64], float signal2[64])
{
	int i;
	float j;
	j = 1;
	float dec;
	dec = 1.0 / 16.0;
	while (j > -0.001) {
		i = 0;
		while (i < 64) {
			if (signal2[i] < j + 0.5 * dec && signal2[i] > j - 0.5 * dec) {
				print("o");
			} else if (signal1[i] < j + 0.5 * dec && signal1[i] > j - 0.5 * dec) {
				print("#");
			} else if (j < 0.5 * dec && j > -0.5 * dec) {
				print("-");
			} else {
				print(" ");
			}
			i = i + 1;
		}
		print_nl();
		j = j - dec;
	}
}

void make_gate(float signal[64])
{
	int i;
	i = 0;
	while (i < 64) {
		if (i < 32) {
			signal[i] = 1.0;
		} else {
			signal[i] = 0.0;
		}
		i = i + 1;
	}
}

void follower(float in[64], float out[64], float a, float r)
{
	float p;
	p = 0.0;
	int i;
	i = 0;
	while (i < 64) {
		if (in[i] > p) {
			p = a * p + (1.0 - a) * in[i];
		} else {
			p = r * p + (1.0 - r) * in[i];
		}
		out[i] = p;
		i = i + 1;
	}
}

int main()
{
	print("Enter Envelope Attack value between 0 and 1 (infinity): ");
	float a;
	a = read_float();
	print_nl();
	print("Enter Envelope Release value between 0 and 1 (infinity): ");
	float r;
	r = read_float();
	print_nl();
	print("Resulting Envelope Follower:");
	print_nl();
	print_nl();

	float gate[64];
	make_gate(gate);

	float envelope[64];
	follower(gate, envelope, a, r);

	print_2_signals(gate, envelope);

	return 0;
}
