#include <math.h>
#include <stdio.h>
#include <stdlib.h>


// Define a default TABLE_SIZE
#define DEFAULT_TABLE_SIZE 360

// Function to compute the cosine and sine tables
void computeTables(int table_size) {
  // Arrays to hold the pre-computed values
  int *cos_table = malloc(table_size * sizeof(int));
  int *sin_table = malloc(table_size * sizeof(int));

  if (cos_table == NULL || sin_table == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  // Constants
  const int SCALE = 1000; // Scale to convert floating point to integer

  // Compute the cosine and sine values
  for (int i = 0; i < table_size; ++i) {
    double radians = (i * M_PI * 2) / table_size; // Convert degrees to radians
    cos_table[i] = (int)(cos(radians) * SCALE);
    sin_table[i] = (int)(sin(radians) * SCALE);
  }

  // Print the cosine table
  printf("#define TABLE_SIZE %d\n", table_size);
  printf("int cos_table[TABLE_SIZE] = {");
  for (int i = 0; i < table_size; ++i) {
    printf("%d", cos_table[i]);
    if (i < table_size - 1)
      printf(", ");
  }
  printf("};\n");

  // Print the sine table
  printf("int sin_table[TABLE_SIZE] = {");
  for (int i = 0; i < table_size; ++i) {
    printf("%d", sin_table[i]);
    if (i < table_size - 1)
      printf(", ");
  }
  printf("};\n");

  // Free allocated memory
  free(cos_table);
  free(sin_table);
}

int main(int argc, char *argv[]) {
  int table_size = DEFAULT_TABLE_SIZE;

  // Check if a custom table size is provided
  if (argc > 1) {
    table_size = atoi(argv[1]);
    if (table_size <= 0) {
      fprintf(stderr, "Invalid table size: %d\n", table_size);
      return 1;
    }
  }

  computeTables(table_size);
  return 0;
}
