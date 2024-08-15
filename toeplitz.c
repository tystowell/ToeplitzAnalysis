#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define MAT_M 16
#define MAT_N 24

void print_binary (unsigned char val) {
	for (int i = 0; i < 8; i++)
		printf("%d", !!((val << i) & 0x80));
}

void print_mat (unsigned char *matrix, int m, int n){
	int bytes_per_row = (n-1) / 8 + 1;

	for (int i = 0; i < m; i++){
		for (int j = 0; j < bytes_per_row; j++)
			print_binary(matrix[bytes_per_row * i + j]);
		printf("\n");
	}
}

void print_sample (unsigned char *sample, int length) {
	int bytes_per_sample = (length-1) / 8 + 1;

	for (int i = 0; i < bytes_per_sample; i++)
		print_binary(sample[i]);

	printf("\n");
}

void get_random_bits(unsigned char *result, int bits){
	int bytes = (bits - 1) / 8 + 1;
	for(int i = 0; i < bytes; i++){
		if (i != 0)
			result[i] = rand() & 0xff;
		else
			result[i] = rand() & (0xff >> (bytes*8-bits));
	}
}

void create_mat (unsigned char *seed, unsigned char *matrix, int m, int n){
	int bytes_per_row = (n-1) / 8 + 1;
	int overflow_bits = bytes_per_row * 8 - n;

	int seed_bytes = (m + n - 1) / 8 + 1;

	for (int i = 0; i < m; i++){
		for (int j = 0; j < bytes_per_row; j++){
			for (int k = 0; k < 8; k++){
				if (j > 0 || k >= overflow_bits) {
					int seed_value = (j * 8 + k - overflow_bits) - i + m - 1;
					int seed_bit = (seed[seed_bytes - 1 - (seed_value / 8)] & (1 << (seed_value % 8))) >> (seed_value % 8);
					matrix[bytes_per_row * i + j] = matrix[bytes_per_row * i + j] ^ (seed_bit << (7-k));
				}
			}
		}
	}
}

void multiply_mat (unsigned char *matrix, unsigned char *sample, unsigned char *result, int m, int n) {
	int sample_bytes = (n-1) / 8 + 1;
	int result_bytes = (m-1) / 8 + 1;
	int overflow_bits = result_bytes * 8 - m;

	for (int i = 0; i < m; i++){
		int bit_result = 0;
		for (int j = 0; j < sample_bytes; j++){
			int mult = matrix[i * sample_bytes + j] & sample[j];
			for(int i = 0; i < 8; i++)
				bit_result ^= (mult >> i) & 1;
		}
		result[(i + overflow_bits) / 8] ^= bit_result << (7 - ((i + overflow_bits) % 8));
	}
}

uint64_t sample_to_int(unsigned char *sample, int length) {
	int sample_bytes = (length-1) / 8 + 1;
	uint64_t result = 0;
	for (int i = 0; i < sample_bytes; i++)
		result += sample[sample_bytes-i-1] << (i * 8);
	return result;
}

void int_to_sample(uint64_t value, unsigned char *sample, int length) {
	int sample_bytes = (length-1) / 8 + 1;
	for (int i = 0; i < sample_bytes; i++)
		sample[sample_bytes-i-1] = (value & (0xff << (8*i))) >> (8*i);
}

int main() {
	srand(time(NULL));
	unsigned char seed[(MAT_N+MAT_M-2)/8 + 1] = {0};
	unsigned char matrix[((MAT_N-1)/8 + 1) * MAT_M] = {0};

	unsigned char sample[(MAT_N-1)/8+1] = {0};
	unsigned char result[(MAT_M-1)/8+1] = {0};

	get_random_bits(seed, MAT_M + MAT_N - 1);
	create_mat(seed, matrix, MAT_M, MAT_N);

	print_mat(matrix, MAT_M, MAT_N);
	printf("\n");

	get_random_bits(sample, MAT_N);
	multiply_mat(matrix, sample, result, MAT_M, MAT_N);

	print_sample(sample, MAT_N);
	printf("\n");
	print_sample(result, MAT_M);

	return 0;
}
