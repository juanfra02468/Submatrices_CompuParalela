typedef struct {
	int x,  // Coordenada x de la submatriz
		y, // Coordenada y de la submatriz
		t; // Tama o de la submatriz
} terna_t;

__kernel void submatrices (int N, __global double *matrizA, __global terna_t *terna, __global double *matrizA_aux) 
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    
    int fila_i = ((i + terna->y) % N) * N;
    int columna_j = (j + terna->x) % N;
 
	for (int k = 0; k < terna->t; k++) {
		matrizA[fila_i+columna_j] += matrizA_aux[fila_i + (terna->x + k) % N] * matrizA_aux[((terna->y + k) % N) * N + columna_j];
	}
}