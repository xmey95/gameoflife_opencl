#include "ocl_boiler.h"
#include "main.h"

#define ROWS 50
#define COLS 100

int main(int argc, char *argv[])
{
	int rows = ROWS;
	int cols = COLS;

	if (argc == 3){
		rows = atoi(argv[1]);
		cols = atoi(argv[2]);
		if (rows <= 0 || cols <= 0)
			error("il numero di righe/colonne deve essere positivo");
	}

	const size_t memsize = sizeof(int)*rows*cols;

	/* Hic sunt leones */

	cl_platform_id p = select_platform();
	cl_device_id d = select_device(p);
	cl_context ctx = create_context(p, d);
	cl_command_queue que = create_queue(ctx, d);
	cl_program prog = create_program("kernels.ocl", ctx, d);

	cl_int err;

	/* Extract kernels */

	 cl_kernel init_k = clCreateKernel(prog, "init", &err);
 	ocl_check(err, "create kernel init");
	cl_kernel generation_k = clCreateKernel(prog, "generation", &err);
	ocl_check(err, "create kernel generation");

	err = clGetKernelWorkGroupInfo(init_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_init), &preferred_wg_init, NULL);
	err = clGetKernelWorkGroupInfo(generation_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_generation), &preferred_wg_generation, NULL);

	cl_mem mat = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
			memsize, NULL, &err);
	ocl_check(err, "create buffer");
	cl_mem d_dst = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
		memsize, NULL, &err);
	ocl_check(err, "create buffer dst");

	cl_event init_evt = init(que, init_k, mat, rows, cols);

	print(rows, cols, mat, init_evt, que, memsize, err);

	for(int i = 1; i <= 130; i++){
		printf("generazione %d\n\n", i);
		cl_event generation_evt = generation(que, generation_k,
			d_dst, mat, rows, cols, init_evt);
		print(rows, cols, d_dst, generation_evt, que, memsize, err);
		mat = d_dst;
		printf("generation time:\t%gms\t%gGB/s\n\n", runtime_ms(generation_evt),
			(2.0*memsize)/runtime_ns(generation_evt));
	}


	printf("init time:\t%gms\t%gGB/s\n", runtime_ms(init_evt),
		(2.0*memsize)/runtime_ns(init_evt));

	return 0;
}
