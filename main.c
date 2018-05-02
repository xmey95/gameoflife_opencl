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

	if(argc < 2){
		error("USAGE: main [configuration] [rows] [cols] [generations]\n\nConfigurations:\ng - Gosper Cannon\nd - Diehard\na - Acorn\n");
	}

	if(argc >= 2){
		if(*argv[1] == 'd'){
			rows = ROWS_DIEHARD;
			cols = COLS_DIEHARD;
			init_c = 'd';
		}
		else if(*argv[1] == 'a'){
			rows = ROWS_ACORN;
			cols = COLS_ACORN;
			init_c = 'a';
		}
	}
	if (argc >= 3){
		if(atoi(argv[2]) > rows){
			rows = atoi(argv[2]);
		}
	}
	if (argc >= 4){
		if(atoi(argv[3]) > cols){
			cols = atoi(argv[3]);
		}
	}
	if (argc >= 5){
		gener = atoi(argv[4]);
	}

	size_t memsize = sizeof(cl_int)*rows*cols;

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
	cl_kernel expand_k = clCreateKernel(prog, "expand", &err);
	ocl_check(err, "create kernel expand");

	err = clGetKernelWorkGroupInfo(init_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_init), &preferred_wg_init, NULL);
	err = clGetKernelWorkGroupInfo(generation_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_generation), &preferred_wg_generation, NULL);
	err = clGetKernelWorkGroupInfo(expand_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_expand), &preferred_wg_expand, NULL);

	cl_mem mat = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
			memsize, NULL, &err);
	ocl_check(err, "create buffer");
	cl_mem d_dst = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
		memsize, NULL, &err);
	ocl_check(err, "create buffer dst");
	cl_mem tmp = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
		memsize, NULL, &err);
	ocl_check(err, "create buffer tmp");

	cl_event init_evt = init(que, init_k, mat, rows, cols, init_c);

	print(rows, cols, mat, init_evt, que, memsize, err);

	printf("init time:\t%gms\t%gGB/s\n\n", runtime_ms(init_evt),
	(2.0*memsize)/runtime_ns(init_evt));

	for(int i = 1; i <= gener; i++){
		cl_event initorexpand_evt = init_evt;
		cl_event expand_evt;
		if(i > 1){
			initorexpand_evt = expand_evt;
		}
		system("clear");
		printf("generazione %d\n\n", i);
		cl_event generation_evt = generation(que, generation_k,
			d_dst, mat, rows, cols, initorexpand_evt);
		print(rows, cols, d_dst, generation_evt, que, memsize, err);
		tmp = mat;
		mat = d_dst;
		d_dst = tmp;
		printf("generation time:\t%gms\t%gGB/s\n\n", runtime_ms(generation_evt),
			(2.0*memsize)/runtime_ns(generation_evt));

		rows++;
		cols++;
		memsize = sizeof(cl_int)*rows*cols;

		cl_mem new_mat = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
					memsize, NULL, &err);
		ocl_check(err, "create buffer new_mat");


		expand_evt = expand(que, expand_k,
			new_mat, mat, rows, cols, generation_evt);
		print(rows, cols, new_mat, expand_evt, que, memsize, err);

		tmp = new_mat;
		new_mat = mat;
		mat = tmp;

		printf("expand time:\t%gms\t%gGB/s\n\n", runtime_ms(expand_evt),
		(2.0*memsize)/runtime_ns(expand_evt));
		//creazione matrice più grande
		usleep(100000);
	}

	return 0;
}
