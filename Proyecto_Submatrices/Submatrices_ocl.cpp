#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <CL/cl.h>
#include "FuncionesCL.h"

typedef struct {
	int x,  // Coordenada x de la submatriz
		y, // Coordenada y de la submatriz
		t; // Tama o de la submatriz
} terna_t;

void initializedouble(int t,double *a,double lv,double uv)
{
  int i;

	for(i=0;i<t;i++)
		//Valores generados entre lv y uv con 2 decimales
		a[i]=((int) ((((1.*rand())/RAND_MAX)*(uv-lv)+lv)*100.))/100.;
}

void initialize(int t,double *a,terna_t *ternas,int r)
{
	int i;

	initializedouble(t*t,a,-10.,10.);

	for(i=0;i<r;i++)
	{
		ternas[i].x=(int) (((1.*rand())/RAND_MAX)*t);
		ternas[i].y=(int) (((1.*rand())/RAND_MAX)*t);
		ternas[i].t=(int) (((1.*rand())/RAND_MAX)*(t-2)+2);
	}
}

void escribir(int t,double *a){
  int i, j;

  for (i = 0; i < t; i++)
  {
    for (j = 0; j < t; j++)
    {  
      printf("%.2f ",a[i*t+j]);
    }
    printf("\n");
  }
  printf("\n");
}

void escribirt(terna_t *a,int t)
{
	int i;

	for(i=0;i<t;i++)
		printf("%d ",a[i].x);
	printf("\n");
	for(i=0;i<t;i++)
		printf("%d ",a[i].y);
	printf("\n");
	for(i=0;i<t;i++)
		printf("%d ",a[i].t);
	printf("\n");
}

/*
c
c     mseconds - returns elapsed milliseconds since Jan 1st, 1970.
c
*/
long long mseconds(){
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
}

int ObtenerParametros(int argc, char *argv[], int *debug, int *num_workitems, int *workitems_por_workgroups) {
	int i;
	*debug=0;
	*num_workitems=0;
	*workitems_por_workgroups=0;
	if (argc<2)
		return 0;
	for (i=2; i<argc;) {
		if (strcmp(argv[i], "-d")==0) {
			*debug=1;
			i++;
		}
		else if (strcmp(argv[i], "-wi")==0) {
			i++;
			if (i==argc)
				return 0;
			*num_workitems=atoi(argv[i]);
			i++;
			if (*num_workitems<=0)
				return 0;
		}
		else if (strcmp(argv[i], "-wi_wg")==0) {
			i++;
			if (i==argc)
				return 0;
			*workitems_por_workgroups=atoi(argv[i]);
			i++;
			if (*workitems_por_workgroups<=0)
				return 0;
		}
		else
			return 0;
	}
	return 1;
}

typedef struct {
	cl_platform_id *plataformas;
	cl_device_id *dispositivos;
	cl_context contexto;
	cl_command_queue cola;
	cl_program programa;
	cl_kernel kernel; 
	// Se pueden definir m s campos de tipo cl_kernel si es necesario
	cl_uint num_dispositivos, num_plataformas;
	cl_int error;
	cl_mem bufferTerna, bufferA, bufferA_Aux;
} EntornoOCL_t;

// **************************************************************************
// ***************************** IMPLEMENTACIÓN *****************************
// **************************************************************************
cl_int InicializarEntornoOCL(EntornoOCL_t *entorno) {
    if (ObtenerPlataformas(entorno->plataformas, entorno->num_plataformas) != CL_SUCCESS) {
        if (entorno->plataformas != NULL)
            delete entorno->plataformas;
        return 0;
    }
    
    if (ObtenerDispositivos(entorno->plataformas[0], CL_DEVICE_TYPE_ALL, entorno->dispositivos, entorno->num_dispositivos) != CL_SUCCESS ) {
        delete entorno->plataformas;
        if (entorno->dispositivos != NULL)
            delete entorno->dispositivos;
        return 0;
    }

    if (CrearContexto(entorno->plataformas[0], entorno->dispositivos, entorno->num_dispositivos, entorno->contexto) != CL_SUCCESS) {
        delete entorno->dispositivos;
        delete entorno->plataformas;
        return 0;
    } 

    entorno->cola = clCreateCommandQueue (entorno->contexto, entorno->dispositivos[0], CL_QUEUE_PROFILING_ENABLE, &entorno->error);
    if (entorno->error != CL_SUCCESS ) {
        printf ("Problema al crear la cola de comandos\n");
        CodigoError(entorno->error);
        delete entorno->dispositivos;
        delete entorno->plataformas;
        return 0;
    }
        
    if (CrearPrograma(entorno->programa, entorno->contexto, entorno->num_dispositivos, entorno->dispositivos, "", "submatrices.cl") != CL_SUCCESS) {
        delete entorno->dispositivos;
        delete entorno->plataformas;
        return 0;
    }


    entorno->kernel = clCreateKernel(entorno->programa, "submatrices", &entorno->error);
    if (entorno->error != CL_SUCCESS) {
        printf("Problema al obtener el kernel\n");
        CodigoError(entorno->error);
        delete entorno->dispositivos;
        delete entorno->plataformas;
        return 0;
    }

}

cl_int LiberarEntornoOCL(EntornoOCL_t *entorno) {
	clReleaseContext(entorno->contexto);
	clReleaseCommandQueue(entorno->cola);
	clReleaseProgram(entorno->programa);
	clReleaseKernel(entorno->kernel);
	clReleaseDevice(entorno->dispositivos[0]);
}

/*
N -> Tama o de la matriz (NxN)
A -> Matriz
ternas -> Vector de ternas con los tama os y las coordenadas de las submatrices
num_sb -> N mero de submatrices
num_workitems -> N mero de work items que se usar n para lanzar el kernel. Es opcional, se puede usar o no dentro de la funci n
workitems_por_workgroups -> N mero de work items que se lanzar n en cada work group. Es opcional, se puede usar o no dentro de la funci n
*/
cl_int ocl(int N,double *A,terna_t *ternas, int num_sb, EntornoOCL_t *entorno, int num_workitems, int workitems_por_workgroups) {
	double *A_aux = (double *)malloc(N * N * sizeof(double));
	for (int i = 0; i < N * N; i++)
	{
		A_aux[i] = A[i];
	}
	for (int i=0; i < num_sb; i++){
		entorno->bufferA = clCreateBuffer(entorno->contexto, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, sizeof(double *), A, &entorno->error);
		if (entorno->error !=CL_SUCCESS)
		{
			printf("Error al crear el buffer de la copia de la matriz A ");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}

		entorno->bufferA_Aux = clCreateBuffer(entorno->contexto, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(double *), A_aux, &entorno->error);
		if (entorno->error !=CL_SUCCESS)
		{
			printf("Error al crear el buffer de la matriz A ");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}
		
		entorno->bufferTerna = clCreateBuffer(entorno->contexto, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(terna_t *), &ternas[i], &entorno->error);
		if (entorno->error !=CL_SUCCESS)
		{
			printf("Error al crear el buffer de terna[i] ");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}
		
		entorno->error = clSetKernelArg(entorno->kernel, 0, sizeof(int), &N);
		if (entorno->error != CL_SUCCESS) {
			printf("Problema al asignar el parametro 0\n");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}
		
		entorno->error = clSetKernelArg(entorno->kernel, 1, sizeof(cl_mem), &entorno->bufferA);
		if (entorno->error != CL_SUCCESS) {
			printf("Problema al asignar el parametro 1\n");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}


		entorno->error = clSetKernelArg(entorno->kernel, 2, sizeof(cl_mem), &entorno->bufferTerna);
		if (entorno->error != CL_SUCCESS) {
			printf("Problema al asignar el parametro 2\n");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}

		entorno->error = clSetKernelArg(entorno->kernel, 3, sizeof(cl_mem), &entorno->bufferA_Aux);
		if (entorno->error != CL_SUCCESS) {
			printf("Problema al asignar el parametro 3\n");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}
		
		
		cl_event evento_exe;
		size_t WorkItems[2] = {ternas[i].t,ternas[i].t};
		entorno->error = clEnqueueNDRangeKernel (entorno->cola, entorno->kernel, 2, NULL, WorkItems, NULL, 0, NULL, &evento_exe);
		if (entorno->error != CL_SUCCESS) {
			printf("Problema al dar la orden de ejecucion del kernel\n");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;

		}

		entorno->error = clFinish(entorno->cola);
		 if (entorno->error != CL_SUCCESS) {
			printf("Error al sincronizar con la cola de comandos\n");
			CodigoError(entorno->error);
			delete entorno->dispositivos;
			delete entorno->plataformas;
			return 0;
		}
	}
	free(A_aux);
}
// **************************************************************************
// *************************** FIN IMPLEMENTACI N ***************************
// **************************************************************************

/*
Recibir  los siguientes par metros (los par metros entre corchetes son opcionales): fichEntrada [-d]
fichEntrada -> Obligatorio. Fichero de entrada con los par metros de lanzamiento de los experimentos
-d -> Opcional. Si se indica, se mostrar n por pantalla los valores iniciales, finales y tiempo de cada experimento
-wi work_items -> Opcional. Si se indica, se lanzar n tantos work items como se indique en work_items (para OpenCL)
-wi_wg workitems_por_workgroup -> Opcional. Si se indica, se lanzar n tantos work items en cada work group como se indique en WorkItems_por_WorkGroup (para OpenCL)
*/
int main(int argc,char *argv[]) {
	int i,
		debug=0,				   // Indica si se desean mostrar los tiempos y resultados parciales de los experimentos
		num_workitems=0, 		   // N mero de work items que se utilizar n
		workitems_por_workgroups=0, // N mero de work items por cada work group que se utilizar n
		num_problems, 		 	   // N mero de experimentos
		matrix_size, 		 	   // Tama o de la matriz
		seed, 			 	   // Semilla  
		num_random;		 	   // N mero de submatrices
	double *A; 			 	   // Matriz de datos. Se representa en forma de vector. Para acceder a la fila f y la columna c: A[f*N+c]
	terna_t *ternas;			   // Vector de ternas con los tama os y las coordenadas de las submatrices
	long long ti,			 	   // Tiempo inicial
			tf,			 	   // Tiempo final
			tt=0; 		 	   // Tiempo acumulado de los tiempos parciales de todos los experimentos realizados
	FILE *f;				 	   // Fichero con los datos de entrada
	EntornoOCL_t entorno; 	 	   //Entorno para el control de OpenCL

	if (!ObtenerParametros(argc, argv, &debug, &num_workitems, &workitems_por_workgroups)) {
		printf("Ejecuci n incorrecta\nEl formato correcto es %s fichEntrada [-d] [-wi work_items] [-wi_wg workitems_por_workgroup]\n", argv[0]);
		return 0;
	}
	
	InicializarEntornoOCL(&entorno);

	// Se leen el n mero de experimentos a realizar
	f=fopen(argv[1],"r");
	fscanf(f, "%d",&num_problems);
	
	for(i=0;i<num_problems;i++) {
		//Por cada experimento se leen
		fscanf(f, "%d",&matrix_size);	//Tama o de la matriz (cuadrada)
		fscanf(f, "%d",&seed);		//Semilla para la inicializaci n de n meros aleatorios
		fscanf(f, "%d",&num_random);	//N mero de submatrices a generar
		//Reserva de memoria para la matriz de datos y las ternas de las submatrices
		A = (double *) malloc(sizeof(double)*matrix_size*matrix_size);
		ternas = (terna_t *) malloc(sizeof(terna_t)*num_random);

		srand(seed);
		initialize(matrix_size,A,ternas,num_random);
		
		if (debug) {
			printf("Matriz original del experimento %d:\n", i); escribir(matrix_size, A);
			printf("Submatrices del experimento %d:\n", i); escribirt(ternas, num_random);
		}

		ti=mseconds(); 
		ocl(matrix_size, A, ternas, num_random, &entorno, num_workitems, workitems_por_workgroups);
		tf=mseconds(); 
		tt+=tf-ti;
		
		if (debug) {
			printf("Tiempo del experimento %d: %Ld ms\n", i, tf-ti);
			printf("Matriz resultado del experimento %d:\n", i); escribir(matrix_size, A);
		}
		free(A);
		free(ternas);
	}
  
	LiberarEntornoOCL(&entorno);
	printf("Tiempo total de %d experimentos: %Ld ms\n", num_problems, tt);
	fclose(f);
	return 0;
}
