#include "ocl_boiler.h"
#include "main.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int rows = ROWS_GOSPER;
	int cols = COLS_GOSPER;
	int gener = GENER;
	char init_c = 'g';

	if (argc == 5){
		if(*argv[4] == 'd'){
			rows = ROWS_DIEHARD;
			cols = COLS_DIEHARD;
			init_c = 'd';
		}
		rows = atoi(argv[1]);
		cols = atoi(argv[2]);
		gener = atoi(argv[3]);
		if(init_c == 'g'){
			if (rows < ROWS_GOSPER || cols < COLS_GOSPER)
				error("il numero di righe/colonne non e' sufficente(gosper)");
		}
		else{
			if (rows < ROWS_DIEHARD || cols < COLS_DIEHARD)
				error("il numero di righe/colonne non e' sufficente(diehard)");
		}
	}
	else {
		error("USAGE: main <rows> <cols> <generations> <configuration: g | d>");
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

	cl_event init_evt = init(que, init_k, mat, rows, cols, init_c);

	print(rows, cols, mat, init_evt, que, memsize, err);

	printf("init time:\t%gms\t%gGB/s\n\n", runtime_ms(init_evt),
	(2.0*memsize)/runtime_ns(init_evt));

	for(int i = 1; i <= gener; i++){
		system("clear");
		printf("generazione %d\n\n", i);
		cl_event generation_evt = generation(que, generation_k,
			d_dst, mat, rows, cols, init_evt);
		print(rows, cols, d_dst, generation_evt, que, memsize, err);
		mat = d_dst;
		printf("generation time:\t%gms\t%gGB/s\n\n", runtime_ms(generation_evt),
			(2.0*memsize)/runtime_ns(generation_evt));
		usleep(100000);
		/*
		cl_event generation_evt = generation(que, generation_k,
			d_dst, mat, rows, cols, init_evt);
		*/
	}

	return 0;
}
