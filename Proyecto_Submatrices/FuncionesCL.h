#include <CL/cl.h>
#include <stdio.h>

void CodigoError(cl_int err);

char leerFuentes(char *&src, const char *fileName);

cl_int ObtenerPlataformas(cl_platform_id *&platforms, cl_uint &num_platforms);

cl_int ObtenerDispositivos(cl_platform_id platform, cl_device_type device_type, cl_device_id *&devices, cl_uint &num_devices);

cl_int CrearContexto(cl_platform_id platform, cl_device_id *devices, cl_uint num_devices, cl_context &contexto);

cl_int CrearPrograma(cl_program &program, cl_context context,cl_uint num_devices, const cl_device_id *device_list, const char *options, const char *fichero);

cl_int ObtenerTiempoEjecucionEvento(cl_event event, cl_ulong &tiempo);

cl_int ObtenerTiempoEjecucionEvento(cl_event event_ini, cl_event event_fin, cl_ulong &tiempo);