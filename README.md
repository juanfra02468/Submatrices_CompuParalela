# Submatrices NxN - Suma de Cuadrados

## Definición del Problema

Dada una matriz \( A \) de \( N \times N \) números reales y un conjunto de ternas del tipo \( (x, y, t) \), se deben obtener submatrices, calcular su cuadrado y sumar el resultado a \( A \). La acumulación de una submatriz no afecta a las siguientes.

### Ejemplo de Matriz \( A \):

|Columna 1|Columna 2|Columna 3|Columna 4|Columna 5|Columna 6|
|----|-----|-----|-----|-----|-----|
| -9.24 | -7.69 | -2.98 |  2.61 |  4.98 | -9.98 |
| -4.00 |  1.33 |  2.65 |  2.15 |  8.70 |  9.63 |
|  7.99 | -5.10 |  2.12 |  6.86 |  9.64 | -1.78 |
|  3.31 |  9.19 |  6.56 |  9.68 | -7.36 |  5.82 |
| -2.71 |  6.49 |  6.42 |  7.53 |  7.23 |  8.43 |
|  5.13 |  7.89 | -9.25 | -7.85 |  0.60 |  5.72 |


### Lista de Ternas:

|| \( x \) | \( y \) | \( t \) |
|---------|---------|---------|---------|
|Terna 1| 0       | 2       | 3       |
|Terna 2| 2       | 1       | 3       |

## Ficheros Suministrados

- `Submatrices_sec.cpp`: Implementación secuencial.
- `Submatrices_ocl.cpp`: Implementación OpenCL.
- `Submatrices_mpi_ocl.cpp`: Implementación híbrida.
- `fichEntrada`: Ejemplo de fichero de entrada.
- `Submatrices`: Ejecutable compilado.

## Compilación

Ejemplo de comandos de compilación:

- **Secuencial**: 
  ```bash
  g++ Submatrices_sec.cpp -o ejecutable

- **OpenCL**: 
  ```bash
  g++ Submatrices_ocl.cpp -o ejecutable -lOpenCL

- **Hibrida**: 
  ```bash
  mpic++ Submatrices_mpi_ocl.cpp -o ejecutable -lOpenCL
