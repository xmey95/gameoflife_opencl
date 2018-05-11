#include "ocl_boiler.h"
#include "main.h"
#include "preferred_wg.h"
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

	size_t memsize = sizeof(int)*rows*cols;

	/* Hic sunt leones */

	cl_platform_id p = select_platform(0);
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
	cl_kernel where_expand_k = clCreateKernel(prog, "where_expand", &err);
	ocl_check(err, "create kernel where_expand");

	err = clGetKernelWorkGroupInfo(init_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_init), &preferred_wg_init, NULL);
	err = clGetKernelWorkGroupInfo(generation_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_generation), &preferred_wg_generation, NULL);
	err = clGetKernelWorkGroupInfo(where_expand_k, d,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(preferred_wg_where_expand), &preferred_wg_where_expand, NULL);

	cl_mem mat = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
			memsize, NULL, &err);
	ocl_check(err, "create buffer");

	cl_event init_evt = init(que, init_k, mat, rows, cols, init_c);

	int *dst = clEnqueueMapBuffer(que, mat, CL_TRUE,
			CL_MAP_READ, 0, memsize,
			1, &init_evt, NULL, &err);
	ocl_check(err, "read buffer");

	print(dst, rows, cols);

	printf("KERNEL INIT:\t%gms\t%gGB/s\n\n", runtime_ms(init_evt),
	(2.0*memsize)/runtime_ns(init_evt));

	cl_event initorexpand_evt = init_evt;

	for(int i = 1; i <= gener; i++){
		cl_mem d_dst = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
			memsize, NULL, &err);
			ocl_check(err, "create buffer dst");

		cl_event expand_evt;

		if(i > 1){
			initorexpand_evt = expand_evt;
		}
		system("clear");
		printf("generazione %d\n\n", i);

		cl_event generation_evt = generation(que, generation_k,
			d_dst, mat, rows, cols, initorexpand_evt);

		err = clWaitForEvents(1u, &generation_evt);
		ocl_check(err, "clWaitForEvents");


		dst = clEnqueueMapBuffer(que, mat, CL_TRUE,
					CL_MAP_READ, 0, memsize,
					1, &init_evt, NULL, &err);
		ocl_check(err, "read buffer");

		print(dst, rows, cols);
		swap(&mat, &d_dst);

		err = clReleaseMemObject(d_dst);
		ocl_check(err, "free buffer d_dst");

		cl_int sides_init[4] = {0}; //sx,dx,up,dw
		cl_mem sides = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
			sizeof(cl_int)*4, sides_init, &err);
			ocl_check(err, "create buffer dst");

		cl_event where_expand_evt = where_expand(que, where_expand_k,
			mat, sides, rows, cols, generation_evt);

		int *sides_after = clEnqueueMapBuffer(que, sides, CL_TRUE,
		    CL_MAP_READ, 0, sizeof(cl_int)*4,
		    1, &where_expand_evt, NULL, &err);
		ocl_check(err, "read buffer");

		err = clWaitForEvents(1u, &where_expand_evt);
		ocl_check(err, "clWaitForEvents");

		int cols_src = cols;

		size_t src_origin[3] = {0, 0, 0};
    size_t dst_origin[3] = {sizeof(int)*sides_after[0], sides_after[2], 0};
    size_t region[3] = {cols*sizeof(int), rows, 1};

		if(sides_after[0] == 1) cols++;
		if(sides_after[1] == 1) cols++;
		if(sides_after[2] == 1) rows++;
		if(sides_after[3] == 1) rows++;

		memsize = sizeof(int)*rows*cols;

		int *n_mat = (int *) malloc(sizeof(int) * rows * cols);

		initialize_newmat(n_mat, rows, cols);

		cl_mem new_mat = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
					memsize, n_mat, &err);
		ocl_check(err, "create buffer new_mat");

		err = clEnqueueCopyBufferRect(que,
									mat,
									new_mat,
									src_origin,
									dst_origin,
									region,
									cols_src * sizeof(int),
									0,
									cols * sizeof(int),
									0,
									1,
									&where_expand_evt,
									&expand_evt);

		swap(&new_mat, &mat);

		err = clWaitForEvents(1u, &expand_evt);
		ocl_check(err, "clWaitForEvents");

		err = clReleaseMemObject(new_mat);
		ocl_check(err, "free buffer new_mat");
		err = clReleaseMemObject(sides);
		ocl_check(err, "free buffer sides");


		printf("\nKERNEL GENERATION:\t%gms\t%gGB/s\n", runtime_ms(generation_evt),
		(10.0*memsize)/runtime_ns(generation_evt));
		printf("KERNEL WHERE_EXPAND:\t%gms\t%gGB/s\n", runtime_ms(where_expand_evt),
		(2.0*memsize)/runtime_ns(where_expand_evt));
		printf("KERNEL EXPAND:\t\t%gms\t%gGB/s\n", runtime_ms(expand_evt),
		(2.0*memsize)/runtime_ns(expand_evt));

		usleep(100000);
	}

	return 0;
}
